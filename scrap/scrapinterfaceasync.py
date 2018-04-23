import os, sys
from time import sleep
from collections import deque
# add relevant dir to path
#__location__ = os.path.realpath(
#	os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
#main_dir = os.path.realpath(os.path.join(__location__,'../..'))
#sys.path.insert(0, main_dir)

from util import ScrapException
from arduinointerface import ArduinoInterface, ArduinoMockInterface
from AsyncDriver import ThreadDriver, ProcessDriver


# class used to transport command requests
class PrinterCommand(object):

	def __init__(self,command,values=None):
		self.command = command
		self.values = values

	def __str__(self):
		return "{}:{}".format(self.command,str(self.values))



class ScrapInterfaceAsync(ProcessDriver):

	def __init__(self, conf):
		self.conf = conf
		self.X_OFFSET = int(conf["X_OFFSET"])
		self.Y_OFFSET = int(conf["Y_OFFSET"])
		self.X_MAX = int(conf["X_MAX"])
		self.Y_MAX = int(conf["Y_MAX"])
		ProcessDriver.__init__(self, arduino_process, (conf,))
		self.daemon = True

	def processInput(self, parse_inp):
		arguments = []
		if isinstance(parse_inp,list):
			arguments = parse_inp
		else:
			arguments = [parse_inp]
		for arg in arguments:
			try:
				inp = arg.strip()

				if inp.lower() == 'r':
					self.reset()
				elif inp == 'u':
					self.pen_up()
				elif inp == 'U':
					self.pen_super_up()
				elif inp.lower() == 'd':
					self.pen_down()
				else:
					comm,coords = inp.split()
					coords = coords.strip().split(",")
					if len(coords) < 2:
						raise ValueError("use a comma to seperate coords")
					# do this if NOT from twitch
					#if not isFromUI:
					if comm == 's':
						self.set_coords(coords)
					elif comm == 'sp':
						self.set_coords(coords,passive=True)
					elif comm == 'r':
						self.reset()
			except ValueError as e:
				print str(e)
			except ScrapException as e:
				print str(e)			

	### cancel print
	def cancel(self):
		self.send_through_pipe("CANCEL")

	### reset arm position and recalibrate zero
	def reset(self):
		return self.doCommand("r")


	### pen controls
	def pen_super_up(self):
		return self.doCommand("U")

	def pen_up(self):
		return self.doCommand("u")

	def pen_down(self):
		return self.doCommand("d")


	### absolute positioning
	def set_x(self,x_inp):
		if int(x_inp) <= self.X_MAX and int(x_inp) >= 0:
			return self.doCommand("sX",[x_inp])
		else:
			raise ScrapException("x_inp is out of bounds")


	def set_y(self,y_inp):
		if int(y_inp) <= self.Y_MAX and int(y_inp) >= 0:
			return self.doCommand("sY",[y_inp])
		else:
			raise ScrapException("y_inp is out of bounds")


	def set_coords(self,coords,passive=False):
		return self.set_coords_indiv(coords[0],coords[1],passive)

	
	def set_coords_indiv(self,x_inp,y_inp,passive=False):
		if int(x_inp) <= self.X_MAX and int(x_inp) >= 0 and int(y_inp) <= self.Y_MAX and int(y_inp) >= 0:
			x_inp = '%4d' % (int(x_inp)+self.X_OFFSET)
			y_inp = '%4d' % (int(y_inp)+self.Y_OFFSET)
			if not passive:
				return self.doCommand("s",[x_inp,y_inp])
			else:
				return self.doCommand("sp",[x_inp,y_inp])
		else:
			raise ScrapException("x_inp or y_inp is out of bounds")


	### relative positioning
	def move_x(self, x_inp):
		if abs(int(x_inp)) <= self.X_MAX:
			return self.doCommand("mX",[x_inp])
		else:
			raise ScrapException("x_inp is too big a move")

	def move_y(self,y_inp):
		if abs(int(y_inp)) <= self.Y_MAX:
			return self.doCommand("mY",[y_inp])
		else:
			raise ScrapException("y_inp is too big a move")


	def move_coords(self,x_inp,y_inp):
		if abs(int(x_inp)) <= self.X_MAX and abs(int(y_inp)) <= self.Y_MAX:
			return self.doCommand("m",[x_inp,y_inp])
		else:
			raise ScrapException("x_inp or y_inp is too big to move")


	def move_coords(self,coords):
		return self.move_coords(coords[0],coords[1])
	
	# Send command through pipe
	def send_through_pipe(self, command):
		self.comm_pipe.send(command)

	# send command to process to perform it
	def doCommand(self, command,values=None):
		self.send_through_pipe(PrinterCommand(command,values))



def arduino_process(conf, comm_pipe):
	print("ARDUINO PROCESS STARTED")
	# do stuff
	command_queue = deque()
	current_command = None
	try:
		keep_running = True
		# create class for sending movement commands
		if not conf["DEBUG"]:
			movement = ArduinoInterface(conf["move_ard_port"],conf["move_ard_baud"])
		else:
			movement = ArduinoMockInterface()
		print("CONNECTED TO ARDUINO")
		# while alive, do stuff
		while keep_running:
			# check pipe for messages
			while keep_running and comm_pipe.poll():
				received = comm_pipe.recv()
				if received == "EXIT":
					keep_running = False
					continue
				# if print is cancelled, clear command queue
				elif received == "CANCEL":
					command_queue.clear()
					print("COMMAND QUEUE CLEARED")
				# if is a printer command, add to queue
				elif isinstance(received, PrinterCommand):
					command_queue.append(received)
					#print("ADDED COMMAND TO QUEUE: {}".format(str(received)))
			# if a command is not currently done, do nothing
			if (current_command is not None and not current_command.checkDone()):
				pass
			# otherwise, try to get next command and send it
			else:
				# if no more commands, do nothing
				if len(command_queue) == 0:
					pass
				# otherwise, send command and set it to current command
				else:
					printer_command = command_queue.popleft()
					current_command = movement.doCommand(printer_command.command,printer_command.values)
			# wait a bit to not max out the CPU
			sleep(0.001)

	except Exception as e:
		try:
			print("SENDING ERROR...")
			comm_pipe.send(e)
		except IOError as e:
			pass
	finally:
		# once done for whatever reason, be sure to stop movement class thread(s)
		movement.stop()

