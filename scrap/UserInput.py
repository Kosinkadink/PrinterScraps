from multiprocessing import Pipe
import threading
from time import sleep

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
				sleep(0.05)
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

	def stop(self):
		self.markToStop()
	
	def isNewMessage(self):
		return self.newMessage.is_set()
	
	def returnMessage(self):
		if self.isNewMessage():
			user_inp = self.pipeOutside.recv()
			self.markMessageRead()
			return user_inp
		else:
			return None
