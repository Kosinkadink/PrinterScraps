import os, sys, time
import serial

# add relevant dir to path
__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
main_dir = os.path.realpath(os.path.join(__location__,'..'))
sys.path.insert(0, main_dir)

from comm.serialcomm import SerialComm

# interface for sending messages to the arduino

class ArduinoInterface(object):

	def __init__(self, port, baud):
		self.ard_comm = SerialComm(serial.Serial(port,int(baud)))
		self.ard_comm.daemon = True
		self.ard_comm.start()

	### sends commands with a possible list of values
	def doCommand(self,command,valList=None):
		requestCommand = command
		if valList == None or len(valList) == 0:
			return self.ard_comm.request(requestCommand)
		else:
			requestCommand += '|'
			for val in valList:
				requestCommand += str(val)
				requestCommand += '|'
			# remove last separator
			requestCommand = requestCommand[:-1]
			print requestCommand
			return self.ard_comm.request(requestCommand)
	
	def stop(self):
		self.ard_comm.stopThread()
