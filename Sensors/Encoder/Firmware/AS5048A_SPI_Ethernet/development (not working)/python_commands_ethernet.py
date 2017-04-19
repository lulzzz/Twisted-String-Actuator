import serial
import time
import numpy as np 
import ipdb
import matplotlib.pyplot as plt
import matplotlib.animation as ani

import socket
# TCP_IP = '169.254.173.240'
TCP_IP = '169.254.26.112'
TCP_PORT = 30000
BUFFER_SIZE = 512
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

# to get IP address of computer: >> ipconfig getifaddr en0



HIP_ANGLE_OFFSET = 18.0		# assumes link starts all the way up against the motor bracket

################# SETUP #################
time.sleep(2)		# pause for 2 seconds

runTime = 10.0
num_motors = 1
angle = dict()

# # initialize
# for i in range(num_motors):
# 	# set the chip select pin: '0' for motor 1, '1' for motor 2, '2' for motor 3
# 	s.write(str(i))

# 	# zero out the encoder once at the beginning
# 	s.send(MESSAGE)

# 	# initialize
# 	angle[i] = list([0.0])






################# MAIN #################
startTime = time.time()
timeNow = list([0.0])

# draw and show it
# plt.ion()
# fig = plt.figure()
# plt.axis([0,runTime,0,360])
# i = list([0.0])


# main run loop
while (timeNow[-1] < runTime):

	# update all the motor angles
	for i in range(num_motors):
		# set the chip select pin: '0' for motor 1, '1' for motor 2, '2' for motor 3
		# s.write(str(i))

		ipdb.set_trace()
		data1 = s.recv(BUFFER_SIZE)

		# sometimes the string from serial is garbage so this creates an error when converting the garbage to a float
		try:
			angle[i].append(float(s.recv(BUFFER_SIZE)) + HIP_ANGLE_OFFSET)
		except ValueError:
			angle[i].append(angle[i][-1])

	
	print 'angle 1: ', angle[0][-1]  


	timeLast = timeNow[-1]
	timeNow.append(time.time() - startTime)
	dt = timeNow[-1] - timeLast
	freq = 1/dt
	print 'frequency: ', freq

	# plt.plot(timeNow, angle)
	# plt.show()
	# plt.pause(.0001)

s.close()
