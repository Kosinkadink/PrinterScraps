import os, sys, time

from util import ScrapException
from arduinointerface import ArduinoInterface, ArduinoMockInterface

class ScrapInterface(object):


	def __init__(self, conf):
		self.conf = conf
		if not self.conf["DEBUG"]:
			self.movement = ArduinoInterface(conf["move_ard_port"],conf["move_ard_baud"])
		else:
			self.movement = ArduinoMockInterface()
		self.X_MAX = int(conf["X_MAX"])
		self.Y_MAX = int(conf["Y_MAX"])


	### reset arm position and recalibrate zero
	def reset(self):
		return self.movement.doCommand("r")


	### pen controls
	def pen_super_up(self):
		return self.movement.doCommand("U")

	def pen_up(self):
		return self.movement.doCommand("u")

	def pen_down(self):
		return self.movement.doCommand("d")


	### absolute positioning
	def set_x(self,x_inp):
		if int(x_inp) <= self.X_MAX and int(x_inp) >= 0:
			return self.movement.doCommand("sX",[x_inp])
		else:
			raise ScrapException("x_inp is out of bounds")


	def set_y(self,y_inp):
		if int(y_inp) <= self.Y_MAX and int(y_inp) >= 0:
			return self.movement.doCommand("sY",[y_inp])
		else:
			raise ScrapException("y_inp is out of bounds")


	def set_coords(self,coords,passive=False):
		return self.set_coords_indiv(coords[0],coords[1],passive)

	
	def set_coords_indiv(self,x_inp,y_inp,passive=False):
		if int(x_inp) <= self.X_MAX and int(x_inp) >= 0 and int(y_inp) <= self.Y_MAX and int(y_inp) >= 0:
			x_inp = '%4d' % int(x_inp)
			y_inp = '%4d' % int(y_inp)
			if not passive:
				return self.movement.doCommand("s",[x_inp,y_inp])
			else:
				return self.movement.doCommand("sp",[x_inp,y_inp])
		else:
			raise ScrapException("x_inp or y_inp is out of bounds")


	### relative positioning
	def move_x(self, x_inp):
		if abs(int(x_inp)) <= self.X_MAX:
			return self.movement.doCommand("mX",[x_inp])
		else:
			raise ScrapException("x_inp is too big a move")

	def move_y(self,y_inp):
		if abs(int(y_inp)) <= self.Y_MAX:
			return self.movement.doCommand("mY",[y_inp])
		else:
			raise ScrapException("y_inp is too big a move")


	def move_coords(self,x_inp,y_inp):
		if abs(int(x_inp)) <= self.X_MAX and abs(int(y_inp)) <= self.Y_MAX:
			return self.movement.doCommand("m",[x_inp,y_inp])
		else:
			raise ScrapException("x_inp or y_inp is too big to move")


	def move_coords(self,coords):
		return self.move_coords(coords[0],coords[1])
	
	def stop(self):
		self.movement.stop()
