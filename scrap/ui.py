import pygame
import threading
from multiprocessing import Pipe

pygame.init()

class UI(object):

	def __init__(self,conf):
		#threading.Thread.__init__(self)
		parent_conn,child_conn = Pipe()
		self.pipeOutside = parent_conn
		self.pipeInside = child_conn

		self.conf = conf
		self.screen = pygame.display.set_mode((int(self.conf["SCREEN_X"]),int(self.conf["SCREEN_Y"])))

		self.armScreen = ArmControlScreen(self.screen)
		
		pygame.display.set_caption("UMKC PrinterBot Control")
		self.clock = pygame.time.Clock()
		self.keepGoing = threading.Event()
		self.newMessage = threading.Event()

		self.message = ''
		self.message_prev = self.message
		
		self.messageLock = threading.Lock()
		self.daemon = True

	def getPipe(self):
		return self.pipeOutside

	def performUI(self):
		# perform actions
		screenValue = self.armScreen.handleEvents()
		self.armScreen.performActions()
		if screenValue[0] == 'QUIT':
			self.markToStop()
			self.message = 'exit'
			self.newMessage.set()
		elif screenValue[0] == 'COORDS':
			self.message = self.convertCoordToScrap(screenValue[1])
			if self.message == self.message_prev:
				pass
			else:
				self.newMessage.set()
				self.message_prev = self.message
		# update screen
		pygame.display.update()
		self.clock.tick(10)

	def convertCoordToScrap(self, coord):
		c1 = coord[0]*int(self.conf["X_MAX"])/int(self.conf["SCREEN_X"])
		c2 = coord[1]*int(self.conf["Y_MAX"])/int(self.conf["SCREEN_Y"])
		return (c1,c2)

	def markMessageRead(self):
		self.newMessage.clear()

	def markToStop(self):
		self.keepGoing.set()

	def isNewMessage(self):
		return self.newMessage.is_set()

	def returnMessage(self):
		if self.isNewMessage():
			self.markMessageRead()
			return self.message
		else:
			return None



class ArmControlScreen(object):

	def __init__(self,screen):
		self.global_objects = []
		self.screen = screen
		self.mouse = MouseEvents()

	def handleEvents(self):
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				return self.signal_QUIT()
			if event.type in [pygame.MOUSEBUTTONDOWN, pygame.MOUSEBUTTONUP]:
				self.mouse.handleMouseEvent(event)
		return self.signal_COORDS()

	def performActions(self):
		self.mouse.performActions()
		self.draw()

	def draw(self):
		self.screen.fill((220, 220, 220))
		pygame.draw.circle(self.screen,(155,0,0),self.mouse.coords,10)

	def signal_QUIT(self):
		return ('QUIT',None)

	def signal_COORDS(self):
		return ('COORDS',self.mouse.coords)


class MouseEvents(object):

	coords = (0,0)
	rel_coords = (0,0)
	held = False
	curObject = None

	def __init__(self):
		pass

	def handleMouseEvent(self,event):
		if event.type == pygame.MOUSEBUTTONDOWN:
			self.held = True
			self.coords = pygame.mouse.get_pos()
		elif event.type == pygame.MOUSEBUTTONUP:
			self.held = False

	def performActions(self):
		if self.held:
			self.coords = pygame.mouse.get_pos()
