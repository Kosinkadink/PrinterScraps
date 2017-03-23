import os, sys, time

# add relevant dir to path
__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
main_dir = os.path.realpath(os.path.join(__location__,'../..'))
sys.path.insert(0, main_dir)

from util import ScrapException
from comm.serialcomm import SerialComm
from arduinointerface import ArduinoInterface


class ScrapInterface(object):

	# set max
	Y_MAX = 3600
	X_MAX = 3600


	def __init__(self, move_ard, x_max=self.X_MAX, y_max=self.Y_MAX):
		self.movement = ArduinoInterface(move_ard)
		self.X_MAX = x_max
		self.Y_MAX = y_max


	def set_x(self,x_inp):
		if int(x_inp) <= self.X_MAX and int(x_inp) >= 0:
			return movement.set_x(x_inp)
		else:
			raise ScrapException("x_inp is out of bounds")


	def set_y(self,y_inp):
		if int(y_inp) <= self.Y_MAX and int(y_inp) >= 0:
			return movement.set_y(y_inp)
		else:
			raise ScrapException("y_inp is out of bounds")