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

	### reset arm position and recalibrate zero
	def reset(self):
		return self.ard_comm.request("R")

	### absolute positioning of arm
	def set_y(self,y_coord):
		return self.ard_comm.request("sY|"+str(y_coord))

	def set_x(self,x_coord):
		return self.ard_comm.request("sX|"+str(x_coord))

	def set_coords(self,coords):
		return sefl.ard_comm.request("sC|{0}:{1}".format(str(coords[0]),str(coords[1])))

	### relative positioning of arm
	def move_y(self,y_diff):
		return self.ard_comm.request("mY|"+str(y_diff))

	def move_x(self,x_diff):
		return self.ard_comm.request("mX|"+str(x_diff))

	def move_coords(self,coords):
		return sefl.ard_comm.request("mC|{0}:{1}".format(str(coords[0]),str(coords[1])))

	### commands for 'accessories'
	def lift(self):
		pass

	def drop(self):
		pass
