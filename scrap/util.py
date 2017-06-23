import os, sys, time, threading
from multiprocessing import Pipe

# add relevant dir to path
__location__ = os.path.realpath(
	os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
main_dir = os.path.realpath(os.path.join(__location__,'..'))
sys.path.insert(0, main_dir)

# read commands from file
def read_commands(filename):
	commands = []
	with open(os.path.join(main_dir,filename),"rb") as read_file:
		for line in read_file:
			if line.strip(): 
				commands.append(line.strip())
	return commands

# create commands from line list, returns command list
def create_commands(lines,saveAs=None):
	commands = []
	if saveAs is None:
		saveAs = "faceToDraw.txt"

	for line in lines:
		for pointindx in range(0,len(line)):
			if pointindx == 0:
				commands.append("s {},{}".format(line[pointindx][0],line[pointindx][1]))
				commands.append("d")
				continue
			commands.append("s {},{}".format(line[pointindx][0],line[pointindx][1]))
		commands.append("u")
	commands.append("r")

	# write into file
	with open(os.path.join(main_dir,saveAs),"wb") as write_file:
		for command in commands:
			write_file.write("{}\n".format(command))
	return commands

# use to block until command is done executing
def wait_till_done(commandObj):
	while not commandObj.checkDone():
		time.sleep(0.01)
	return commandObj.getResponse()

# returns dictionary of config values
def read_config(filename):
	c_dict = {}
	with open(os.path.join(main_dir,filename),"rb") as conf:
		for line in conf:
			try:
				key,val = line.strip().split('=')
				key = key.strip()
				val = val.strip()
				c_dict[key] = val
			except Exception,e:
				pass
	return c_dict

# exception used for scrap-related errors
class ScrapException(Exception):
	pass

# get user input in a separate thread
class UserInput(threading.Thread):
	
	def __init__(self):
		threading.Thread.__init__(self)
		parent_conn,child_conn = Pipe()
		self.pipeOutside = parent_conn
		self.pipeInside = child_conn
		self.keepGoing = threading.Event()
		self.newMessage = threading.Event()
		self.daemon = True # turn off if main thread turns off
	
	def getPipe(self):
		return self.pipeOutside
	
	def run(self):
		while not self.keepGoing.is_set():
			# wait until message is read
			if self.newMessage.is_set():
				time.sleep(0.05)
				continue
			userinp = raw_input('> ')
			if userinp.lower() == 'exit':
				self.markToStop()
			self.pipeInside.send(userinp)
			self.newMessage.set()
	
	def markMessageRead(self):
		self.newMessage.clear()
	
	def markToStop(self):
		self.keepGoing.set()
	
	def isNewMessage(self):
		return self.newMessage.is_set()
	
	def returnMessage(self):
		if self.isNewMessage():
			user_inp = self.pipeOutside.recv()
			self.markMessageRead()
			return user_inp
		else:
			return None
