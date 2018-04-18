from serial import Serial, SerialException
from collections import namedtuple
from time import sleep
from AsyncDriver import ThreadDriver, ProcessDriver

class CommandException(Exception):
	pass

class ArduinoCommException(Exception):
	pass



class Command(object):

	special_chars = ["|","*"]

	def __init__(self, command, values=None):
		self.command = None
		self.values = None
		self.set_command(command)
		if values is not None:
			self.set_values(values)

	def set_command(self, command):
		"""
		Sets command, raises CommandException if proposed command is not valid
		Raises: CommandException on commands that contain special characters
		"""
		if not isinstance(command, str):
			raise CommandException("command must be a string, not {}".format(type(command)))
		elif any(ch in command for ch in self.special_chars):
			raise CommandException("command cannot include '|' or '*' characters")
		elif len(command) == 0:
			raise CommandException("command cannot be an empty string")
		else:
			self.command = command

	def set_values(self, values):
		"""
		Sets values, raises CommandException if proposed values are not valid
		:param values: lsit or string (string valid for a single value)
		Raises: CommandException on values that contain special characters
		"""
		proper_type = False
		if isinstance(values, str):
			proper_type = True
			values = [values]
			value = values[0]
			if not self.is_valid_string(value):
				raise CommandException("values cannot include '|' or '*' characters")
			elif len(value) == 0:
				raise CommandException("values cannot be an empty string")
		elif isinstance(values, list):
			proper_type = True
			for value in values:
				if not self.is_valid_string(value):
					raise CommandException("values cannot include '|' or '*' characters")
				elif len(value) == 0:
					raise CommandException("values cannot be an empty string")
		# if proper type, assign value
		if proper_type:
			self.values = values
		# otherwise, raise exception
		else:
			raise CommandException("values must be a string or a list, not {}".format(type(values)))

	def get_command(self):
		return self.command

	def get_values(self):
		return self.values

	def is_valid_string(self, string):
		"""
		Returns True/False depending on if string contains special characters
		Returns: boolean for validity
		"""
		return not any(ch in string for ch in self.special_chars)

	def get_formatted_string(self):
		"""
		Returns full string command to be sent to a device
		Preconditions: assumes command and values have been set properly
		Returns: string
		"""
		fullstring = ""
		argument_list = []
		argument_list.append(self.command)
		# if none, ignore
		if self.values is None:
			pass
		# if list, extend argument list
		elif isinstance(self.values,list):
			argument_list.extend(self.values)
		# if (hopefully) string, add it as an item
		else:
			argument_list.append(self.values)
		# add to fullstring
		fullstring = "|".join(argument_list)
		# add checksum to the end
		fullstring += "*{}".format(Command.get_checksum(fullstring))
		return fullstring

	def __str__(self):
		"""
		Wrapper for get_formatted_string()
		"""
		return self.get_formatted_string()

	@staticmethod
	def create_command(fullmessage):
		"""
		Returns a Command object initialized with a string, or None if string is not valid
		Returns: Command object
		Raises: Command Exception 
		"""
		try:
			# see if length of message is valid
			if len(fullmessage) < 3:
				raise CommandException("fullmessage must be at least 3 characters long; {}".format(fullmessage))
			# see if '*' is second to last character
			if fullmessage[-2] != "*":
				raise CommandException("the second to last character must be an '*': {}".format(fullmessage))
			# see if checksum on the end is valid
			provided_checksum = fullmessage[-1]
			fullmessage = fullmessage[:-2]
			expected_checksum = Command.get_checksum(fullmessage)
			if not provided_checksum == expected_checksum:
				raise CommandException("expected checksum to be {}; got {} instead".format(expected_checksum,provided_checksum))
			# check if contains values
			if "|" in fullmessage:
				full_list = fullmessage.split("|")
				return Command(full_list[0],full_list[1:])
			# otherwise, this is just a command
			else:
				return Command(fullmessage)
		except CommandException as e:
			raise e

	@staticmethod
	def get_checksum(message):
		"""
		Returns a single character corresponding to calculated XOR checksum of message
		Returns: a single character
		"""
		checksum = 0
		for ch in message:
			checksum ^= ord(ch)
		# change range of checksum
		checksum = (checksum % 94) + 33
		return str(unichr(checksum))



class ArduinoComm(ProcessDriver):

	def __init__(self, conf):
		self.conf = conf
		self.button_pressed = False
		self.arduino_ready = False
		ProcessDriver.__init__(self, arduino_process, (conf,))
		self.daemon = conf["daemon"]
		self.turn_deadzone = conf["arduino"]["turn_deadzone"]

	# Steering Commands
	def commandTurn(self, angle):
		# if angle is less than turn deadzone, set it to zero
		if abs(angle) < self.turn_deadzone:
			angle = 0
		cmd = Command("t",str(round(angle)))
		self.send_through_pipe(cmd)

	def commandStraight(self):
		cmd = Command("s")
		self.send_through_pipe(cmd)

	# Throttle Commands
	def commandForward(self, speed):
		cmd = Command("f",str(speed))
		self.send_through_pipe(cmd)

	def commandBackward(self, speed):
		cmd = Command("b",str(speed))
		self.send_through_pipe(cmd)

	def commandStop(self):
		cmd = Command("stop")
		self.send_through_pipe(cmd)

	def commandBrake(self):
		cmd = Command("brake")
		self.send_through_pipe(cmd)

	# Getters
	def is_ready(self):
		return self.arduino_ready

	# Waiting Loops
	def wait_for_readiness(self):
		while self.is_properly_alive() and not self.is_ready():
			sleep(0.05)


	# Input handler
	def handle_input(self, input_obj):
		if isinstance(input_obj,Command):
			# check if Ready command
			if input_obj.command == "ready":
				self.arduino_ready = True
		elif input_obj is not None:
			print("RECEIVED WEIRD INPUT: {}".format(input_obj))

	# Send command through pipe
	def send_through_pipe(self, command):
		self.comm_pipe.send(command)

	@staticmethod
	def send_command_via_serial(serial_object, command, retries=3):
		"""
		Use a serial object to send a command
		:param serial_object: Serial instance
		:param command: Command instance
		:param retries: optional integer parameter for number of retries
		Returns: boolean corresponding to whether or not command was sent successfully
		Raises: command exception
		"""
		try:
			retry = 0
			while retry < retries:	
				# attempt to write to serial
				serial_object.write(command.get_formatted_string()+"\n")
				# wait for response
				response = serial_object.readline().strip()
				# if starts with 'n', attempt to resend
				if len(response) == 1:
					if response.startswith("n"):
						retry += 1
					else:
						return None
				# otherwise, attempt to get a Command as response 
				else:
					return Command.create_command(response)
		except CommandException as e:
			raise e

	@staticmethod
	def receive_command_via_serial(serial_object):
		try:
			response = serial_object.readline().strip()
			return Command.create_command(response)
		except CommandException as e:
			raise e




def arduino_process(conf, comm_pipe):
	arduino_serial = None
	print("ARDUINO PROCESS STARTED")
	try:
		keep_running = True
		# start serial process, raise a ArduinoComm exception if fails
		try:
			arduino_serial = Serial(conf["arduino"]["port"],conf["arduino"]["baud"],timeout=conf["arduino"]["timeout"])
		except SerialException as e:
			raise ArduinoCommException(e)
		print("CONNECTED TO ARDUINO")
		while keep_running:
			# check pipe for messages
			if comm_pipe.poll():
				received = comm_pipe.recv()
				if received == "EXIT":
					keep_running = False
					break
				elif isinstance(received, Command):
					response_command = ArduinoComm.send_command_via_serial(arduino_serial, received)
					# if response_command was None, then there is nothing real to deliver
					if received_command is not None:
						# send response command through pipe
						comm_pipe.send(response_command)
			# check if there is anything to receive in serial
			if arduino_serial.in_waiting:
				received_command = ArduinoComm.receive_command_via_serial(arduino_serial)
				comm_pipe.send(received_command)

	except Exception as e:
		try:
			print("SENDING ERROR...")
			comm_pipe.send(e)
		except IOError as e:
			pass
	finally:
		if isinstance(arduino_serial,Serial):
			arduino_serial.close()
