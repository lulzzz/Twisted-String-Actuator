import serial
import time
from ipdb import set_trace as trace
import numpy as np
import csv
import copy
import sys


class dopeStruct():
	pass


def getFeedback(serialObject, dataStored):
	# create return variable struct
	fbk = dopeStruct()

	# read serial
	dataIn = str(serialObject.readline())	

	# add data to log for generating csv file at the end
	dataStored.append(dataIn[0:-2])

	# parse the data into individual variables (need to know what is being sent from the arduino to assign variables correctly)
	parsed = dataIn.split(',')
	if len(parsed) >= 5:
		fbk.m1Revs = parsed[0]
		fbk.m2Revs = parsed[1]
		fbk.outputRev = parsed[2]
		fbk.fsr1 = parsed[3]
		fbk.fsr2 = parsed[4]

	return fbk, dataStored


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


def actuatorFK():
	pass



def main():

	# set parameters for twisted string actuator
	params = dopeStruct()
	params.r0 = 0.55				#[mm] string radius
	params.Lc = 190					#[mm] nominal string length (check this value)
	params.pulley_radius = 38.1		#[mm] pulley radius	


	# set up serial communications
	serialObject = serial.Serial('/dev/cu.usbmodem1411', 115200, timeout=5)
	time.sleep(2)

	# initialize timers
	timeNow = 0
	startTime = time.time()

	if len(sys.argv) <= 1:
		runTime = 10
	else:
		runTime = float(sys.argv[1]) 

	# initialize other variables
	dataStored = list()
	cmd = dopeStruct()
	X1 = params.Lc
	X2 = params.Lc


	# control loop that read Serial line
	while timeNow < runTime:

		# read serial string and parse data
		fbk, dataStored = getFeedback(serialObject, dataStored)


		try: 
			print fbk.m1Revs
		except:
			pass

		# do math for twisted string kinematics
		# X1 = twistedStringFK(fbk.m1Revs, params, X1)
		# X2 = twistedStringFK(fbk.m2Revs, params, X2)


		# update commands every time step
		cmd.m1Revs = 0
		cmd.m2Revs = 0
		cmd.outputRev = 0
		cmd.motorKp = 60
		cmd.motorKd = -.1


		# send commands to arduino for motor control
		sendCommands(serialObject, cmd)

		# update timer 
		timeLast = timeNow
		timeNow = time.time() - startTime
		dt = timeNow - timeLast
		# print 1/dt


	serialObject.close()



	# Write data to CSV file
	with open("output.csv", "wb") as f:
	    writer = csv.writer(f, delimiter = ',')
	    writer.writerows([x.split(',') for x in dataStored])



if __name__ == '__main__':
	main()

