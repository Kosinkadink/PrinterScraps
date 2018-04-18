import pygame
import threading
from multiprocessing import Pipe
from AsyncDriver import ThreadDriver, ProcessDriver


class UI_Async(ProcessDriver):

	def __init__(self, conf):
		self.conf = conf
		ProcessDriver.__init__(self, ui_process, (conf,))
		self.daemon = True
		self.message = ''
		self.newMessage = threading.Event()
		self.messageLock = threading.Lock()

	def markMessageRead(self):
		self.newMessage.clear()

	def isNewMessage(self):
		return self.newMessage.is_set()

	def returnMessage(self):
		if self.isNewMessage():
			self.markMessageRead()
			return self.message
		else:
			return None


	# Input handler
	def handle_input(self, input_obj):
		if isinstance(input_obj,type("")):
			self.message = input_obj
			self.newMessage.set()
		elif isinstance(input_obj,tuple):
			self.message = "s {},{}".format(input_obj[0],input_obj[1])
			self.newMessage.set()
		elif input_obj is not None:
			print("RECEIVED WEIRD INPUT: {}".format(input_obj))


def ui_process(conf, comm_pipe):
	print("UI PROCESS STARTED")
	ui = UI(conf)
	try:
		keep_running = True

		while keep_running:
			# check pipe for messages
			if comm_pipe.poll():
				received = comm_pipe.recv()
				if received == "EXIT":
					keep_running = False
					continue
			# update screen
			ui.performUI()
			# get ui message
			ui_inp = ui.returnMessage()
			if ui_inp != None:
				comm_pipe.send(ui_inp)
			# delay is handled via UI delay
	except Exception as e:
		try:
			print("SENDING ERROR...")
			comm_pipe.send(e)
		except IOError as e:
			pass
	finally:
		ui.markToStop()



class UI(object):

	def __init__(self,conf):
		pygame.init()

		self.conf = conf
		self.screen = pygame.display.set_mode((int(self.conf["WINDOW_X"]),int(self.conf["WINDOW_Y"])))

		self.armScreen = ArmControlScreen(self.screen,self.conf)
		
		pygame.display.set_caption("UMKC PrinterBot Control")
		self.clock = pygame.time.Clock()
		self.keepGoing = threading.Event()
		self.newMessage = threading.Event()

		self.message = ''
		self.message_prev = self.message
		
		self.messageLock = threading.Lock()
		self.daemon = True

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
			elif self.message == None:
				pass
			else:
				self.newMessage.set()
				self.message_prev = self.message
		elif screenValue[0] == 'MESSAGE':
			self.message = screenValue[1]
			self.newMessage.set()
		# update screen
		pygame.display.update()
		self.clock.tick(int(self.conf["UI_FPS"]))

	def convertCoordToScrap(self, coord):
		if coord[0] > int(self.conf["SCREEN_X"]) or coord[1] > int(self.conf["SCREEN_Y"]):
			return None
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

	def __init__(self,screen,conf):
		self.screen = screen
		self.conf = conf
		self.global_objects = []
		# add buttons
		self.global_objects.append(Button(self.screen,(int(self.conf["SCREEN_X"])+20,50),(75,50),"r","Reset"))
		self.global_objects.append(Button(self.screen,(int(self.conf["SCREEN_X"])+20,150),(75,50),"u","Pen Up"))
		self.global_objects.append(Button(self.screen,(int(self.conf["SCREEN_X"])+20,250),(75,50),"d","Pen Down"))
		
		self.mouse = MouseEvents(self.global_objects)
		self.prev_coords = (0,0)
		self.touchArea = pygame.Rect(0,0,int(self.conf["SCREEN_X"]),int(self.conf["SCREEN_Y"]))

	def handleEvents(self):
		mouseCommand = None
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				return self.signal_QUIT()
			if event.type in [pygame.MOUSEBUTTONDOWN, pygame.MOUSEBUTTONUP]:
				mouseCommand = self.mouse.handleMouseEvent(event)
		if mouseCommand != None:
			return self.signal_MESSAGE(mouseCommand)
		return self.signal_COORDS()

	def performActions(self):
		self.mouse.performActions()
		self.draw()

	def draw(self):
		self.screen.fill((220, 220, 220))
		pygame.draw.rect(self.screen,(0,0,0),self.touchArea)
		if self.touchArea.collidepoint(self.mouse.coords):
			pygame.draw.circle(self.screen,(155,0,0),self.mouse.coords,10)
			self.prev_coords = self.mouse.coords
		else:
			pygame.draw.circle(self.screen,(155,0,0),self.prev_coords,10)
		for obj in self.global_objects:
			obj.draw()

	def signal_QUIT(self):
		return ('QUIT',None)

	def signal_COORDS(self):
		return ('COORDS',self.mouse.coords)

	def signal_MESSAGE(self,message):
		return ('MESSAGE',message)


class MouseEvents(object):

	coords = (0,0)
	rel_coords = (0,0)
	held = False
	curObject = None

	def __init__(self,objects=[]):
		self.OBJECTS = objects

	def handleMouseEvent(self,event):
		if event.type == pygame.MOUSEBUTTONDOWN:
			self.held = True
			self.coords = pygame.mouse.get_pos()
			for obj in self.OBJECTS:
				tempRect = pygame.Rect(obj.OFFSETS[0],obj.OFFSETS[1],obj.TOTAL_WIDTH,obj.TOTAL_HEIGHT) 
				if tempRect.collidepoint(self.coords):
					return obj.handleMouseEvent(event)
		elif event.type == pygame.MOUSEBUTTONUP:
			self.held = False
		return None

	def performActions(self):
		if self.held:
			self.coords = pygame.mouse.get_pos()


class Button(object):

	def __init__(self,screen,coords,size,message,text_content):
		self.screen = screen
		self.OFFSETS = coords
		self.TOTAL_WIDTH = size[0]
		self.TOTAL_HEIGHT = size[1]
		self.MESSAGE = message
		# button box
		self.obj = pygame.Rect(self.OFFSETS[0],self.OFFSETS[1],self.TOTAL_WIDTH,self.TOTAL_HEIGHT)
		# text stuff
		self.text_color = (255, 255, 255)
		self.font = pygame.font.Font(None, 20)
		self.text_content = text_content
		self.text = self.font.render(self.text_content, 1, self.text_color)
		self.textpos = self.text.get_rect()
		self.textpos.center = self.obj.center
		
	def handleMouseEvent(self,event):
		if event.type == pygame.MOUSEBUTTONDOWN:
			return self.MESSAGE
		else:
			return None

	def draw(self):
		pygame.draw.rect(self.screen,(255,0,0),self.obj)
		self.screen.blit(self.text, self.textpos)

		
