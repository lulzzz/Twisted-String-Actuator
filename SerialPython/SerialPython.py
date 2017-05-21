import serial
import time
from ipdb import set_trace as trace
import numpy as np
import csv
import copy
import sys
import re
import os
import glob
from xlsxwriter.workbook import Workbook




def csv2xlsx():
	# convert csv files to xlsx
	for csvfile in glob.glob(os.path.join('.', '*.csv')):
		workbook = Workbook(csvfile[:-4] + '.xlsx', {'strings_to_numbers': True})
		worksheet = workbook.add_worksheet()
		with open(csvfile, 'rb') as f:
			reader = csv.reader(f)
			for r, row in enumerate(reader):
				for c, col in enumerate(row):
					worksheet.write(r, c, col)
		workbook.close()
		os.remove(csvfile)


class dopeStruct():
	pass


def getFeedback(serialObject, dataStored):
	# update boolean
	refresh = False

	# create return variable struct
	fbk = dopeStruct()

	# read serial
	dataIn = str(serialObject.readline())	

	# don't store the data if its a command on the serial line
	if dataIn[0] != 'c':
		refresh = True

		# remove non-numeric characters (except commas and periods) from string
		dataIn = re.sub("[^0-9,.-]","",dataIn)

		# add time to data abd append data to log for generating csv file at the end. 
		# If npt using re.sub() above then dataIn should be [0:-2] to get rid of the '\n' 
		dataStored.append(dataIn[0:])
		print dataStored[-1]

		# parse the data into individual variables (need to know what is being sent from the arduino to assign variables correctly)
		parsed = dataIn.split(',')
		if len(parsed) >= 5:
			fbk.m1Revs = parsed[0]
			fbk.m2Revs = parsed[1]
			fbk.outputRev = parsed[2]
			fbk.fsr1 = parsed[3]
			fbk.fsr2 = parsed[4]


	return fbk, dataStored, refresh


def sendCommands(serialObject, cmd):

	# format string to send over serial (add c to beginning of string to conform to arduino incoming data parser)
	sendString = 'c' + str(cmd.m1Revs) + ',' + str(cmd.m2Revs) + ',' + str(cmd.outputRev) + ',' + str(cmd.motorKp) + ',' + str(cmd.motorKd)

	# send command over serial
	serialObject.write(sendString)


def twistedStringFK(posFBK, params, X):
	''' This function references the following paper: 
		I. Gaponov, D. Popov, J.H. Ryu. "Twisted String Actuation Systems: A Study of the
		Mathematical Model and a Comparison of Twisted Strings." IEEE/ASME Transaction on 
		Mechatronics, August 2014.	'''

	# first find the variable radius of the string which changes following poisson's raito
	r_var = params.r0*np.sqrt(params.Lc/X)

	# find contraction length of each string based on number of twists in the string (with constant string radius)
	dX = params.Lc - np.sqrt(np.square(params.Lc) - np.square(posFBK)*np.square(r_var))

	X = params.Lc - dX

	# calculate transmission ratio
	transmission = dX/posFBK

	return X


def setPreload(serialObject, preload_num_twists):
	# set the number of twists at the beginning and end of each test so it doesn't need to be done by hand
	string_2_send = 'r' + str(preload_num_twists)
	serialObject.write(string_2_send)



def actuatorFK():
	pass





def main():

	# set parameters for twisted string actuator
	params = dopeStruct()
	params.r0 = 0.55				#[mm] string radius
	params.Lc = 190					#[mm] nominal string length (check this value)
	params.pulley_radius = 38.1		#[mm] pulley radius	

	# Operating parameters
	PAUSE_TIME = 3
	preload_twists = 20
	test = 'step'	# 'step', 'bandwidth', 'nf'
	step_time = 5


	# set up serial communications
	serialObject = serial.Serial('/dev/cu.usbmodem1411', 115200, timeout=5)
	time.sleep(2)

	if len(sys.argv) <= 1:
		runTime = 10
	else:
		runTime = float(sys.argv[1]) 

	# zero encoders
	serialObject.write('e')	# motor encoders
	time.sleep(.1)

	# initialize other variables
	dataStored = list()
	cmd = dopeStruct()
	X1 = params.Lc
	X2 = params.Lc

	# set the preload number of twists
	setPreload(serialObject, preload_twists)
	time.sleep(PAUSE_TIME) 		# [s] wait a few seconds

	# zero output encoder after applying preload twists
	serialObject.write('z')	# output encoder
	time.sleep(.1)

	# initialize timers
	timeNow = 0
	startTime = time.time()

	# control loop that read Serial line
	while timeNow < runTime:

		# read serial string and parse data
		fbk, dataStored, refresh = getFeedback(serialObject, dataStored)

		try: 
			print fbk.outputRev
		except:
			pass

		# do math for twisted string kinematics
		# X1 = twistedStringFK(fbk.m1Revs, params, X1)
		# X2 = twistedStringFK(fbk.m2Revs, params, X2)


		amp = 20		# unit is twists
		phase_shift = np.pi
		freq = 1*(2*np.pi)
		m1Revs_cmd = amp*np.sin(freq*timeNow)
		m2Revs_cmd = amp*np.sin(freq*timeNow + phase_shift) # 180 deg phase shift

		# add preload twists
		m1Revs_cmd += preload_twists
		m2Revs_cmd += preload_twists


		# Run different tests
		if test == 'step':
			# step response test 
			# (use kinematic model eventually instead of just open-loop number of twists to acheive desired output angle step response)
			if timeNow < step_time:
				amp = 0

			m1Revs_cmd = amp + preload_twists
			m2Revs_cmd = -amp + preload_twists

		elif test == 'nf':
			m1Revs_cmd = preload_twists
			m2Revs_cmd = preload_twists

		elif test == 'bandwidth':
			pass


		# update commands every time step
		cmd.m1Revs = np.round(m1Revs_cmd,2)
		cmd.m2Revs = np.round(m2Revs_cmd,2)
		cmd.outputRev = 0
		cmd.motorKp = 60
		cmd.motorKd = -.1


		# send commands to arduino for motor control
		sendCommands(serialObject, cmd)

		# update dataStored to include commands and time stamp
		if refresh:
			dataStored[-1] = str(timeNow) + ',' + str(cmd.m1Revs) + ',' + str(cmd.m2Revs) + ',' + dataStored[-1]

		# update timer 
		timeLast = timeNow
		timeNow = time.time() - startTime
		dt = timeNow - timeLast
		# print 1/dt


	# Write data to CSV file
	save_string = test + '_p' + str(preload_twists) + '_a' + str(amp) + '.csv'
	with open(save_string, "wb") as f:
	    writer = csv.writer(f, delimiter = ',')
	    writer.writerows([x.split(',') for x in dataStored])

	# convert csv to xlsx file
	csv2xlsx()

	# reset the preload number of twists back to zero for next test
	try:
		print "Done: "
		print fbk.m1Revs
		print fbk.m2Revs
	except:
		pass

	for i in range(10):
		setPreload(serialObject, 0)

	time.sleep(PAUSE_TIME) 		# [s] wait a few seconds
	
	try:
		print "Zero: "
		print fbk.m1Revs
		print fbk.m2Revs
	except:
		pass

	serialObject.close()




if __name__ == '__main__':
	main()

