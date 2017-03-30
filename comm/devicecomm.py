import threading, os, sys, serial, time


class CommRequest(object):
	# object for communication

	def __init__(self, request, returnAs=""):
		self.request = request
		self.returnAs = returnAs
		self.isDone = False
		self.response = None

	def checkDone(self):
		return self.isDone

	def markDone(self):
		self.isDone = True

	def setResponse(self, response):
		self.response = response

	def getResponse(self):
		return self.response


class DeviceComm(threading.Thread):
	# template for interfaces with some DeviceComm
	# ABSTRACT CLASS, should inherit into another class

	def __init__(self, comm):
		self.comm = comm
		self.keepRunning = threading.Event()
		self.deviceName = None
		self.commandLock = threading.Lock()
		threading.Thread.__init__(self)
		self.daemon = True
		self.commandList = []

	# add a command to the command queue	
	def requestCommand(self, commReq):
		#self.commandLock.acquire()
		self.commandList.append(commReq)
		#self.commandLock.release()

	# add a command directly, return created CommandObj
	def request(self, request, returnAs=""):
		newRequest = CommRequest(request,returnAs)
		self.requestCommand(newRequest)
		return newRequest

	# remove command from command queue
	def removeCommand(self, commReq):
		#self.commandLock.acquire()
		self.commandList.remove(commReq)
		#self.commandLock.release()

	def run(self):
		while (not self.keepRunning.is_set()):
			self.commandLock.acquire()
			if len(self.commandList) > 0:  # check if commands to perform
				# do the item in queue
				commandObj = self.commandList[0]
				self.commandLock.release()
				self.performCommand(commandObj)
				# now remove the command from queue
				self.commandLock.acquire()
				self.removeCommand(commandObj)
			self.commandLock.release()
			# wait a little
			time.sleep(0.01)
		# marked for stopping
		self.closeConnection()

	# used to mark particular comm thread to stop
	def stopThread(self):
		self.keepRunning.set()

	# fill out with closing connection
	def closeConnection(self):
		pass

	# fill out with particular behavior
	def performCommand(self, commReq):
		pass


# exception for reporting DeviceComm-related errors
class DeviceCommException(Exception):
	pass
