import math
import os
import copy
import cv2
import threading
import numpy as np
import datetime
from time import sleep

__location__ = os.path.realpath(
	os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
main_dir = os.path.realpath(os.path.join(__location__,'..'))
print_dir = os.path.join(main_dir,"prints")

def no_action(x):
	pass


# create commands from line list, returns command list
def create_commands(lines,saveAs=None):
	commands = []
	if saveAs is None:
		saveAs = datetime.datetime.now().strftime("%Y%m%d%H%M%S")

	for line in lines:
		for pointindx in range(0,len(line)):
			if pointindx == 0:
				commands.append("s {},{}".format(line[pointindx][0],line[pointindx][1]))
				commands.append("d")
				continue
			commands.append("s {},{}".format(line[pointindx][0],line[pointindx][1]))
		commands.append("u")
	commands.append("r")

	# create print directory
	if not os.path.isdir(print_dir): os.makedirs(print_dir)
	# write into file
	with open(os.path.join(print_dir,saveAs),"wb") as write_file:
		for command in commands:
			write_file.write("{}\n".format(command))
	return commands


class Vision(threading.Thread):

	def __init__(self,conf):
		self.conf = conf
		self.millidelay = int(self.conf["FRAME_MILLIDELAY"])
		threading.Thread.__init__(self)
		self.daemon = True
		# open video camera
		self.cap = cv2.VideoCapture(int(self.conf["CAMERA_INDEX"]))
		self.cap.set(3, int(self.conf["CAMERA_X"]))
		self.cap.set(4, int(self.conf["CAMERA_Y"]))
		# load face cascade
		self.face_cascade = cv2.CascadeClassifier(os.path.join(__location__,'resources/haarcascade_frontalface_default.xml'))
		self.keep_running = True
		self.latest_frame = None
		self.readyToDraw = False
		self.linesToDraw = []
		self.message = None
		self.newMessage = False
		self.allow_frameshot = True
		self.scale = float(self.conf["IMG_SCALE"])

	def returnMessage(self):
		if self.newMessage:
			self.newMessage = False
			return self.message
		else:
			return None

	def run(self):
		while self.keep_running:
			self.latest_frame = self.getFrame()
			# use cv2 wait in order to display window
			key = cv2.waitKey(self.millidelay) & 0xFF
			if (self.allow_frameshot):
				if (key == ord("q")) or cv2.getWindowProperty('frame', 0) < 0:
					self.message = "exit"
					self.newMessage = True
					self.stop()
				elif (key == ord("s")):
					self.processImage()
				elif (key == ord("u")):
					if self.latest_frame != None:
						h, w = self.latest_frame.shape[:2]
						print("{0},{1}".format(w, h))
		

	def processImage(self,image=None):
		# check if image was provided
		self.allow_frameshot = False
		try:
			if image:
				print image
				# try to load the image
				try:
					usable_frame = cv2.imread(os.path.join(main_dir,image))
				except Exception as e:
					print str(e)
					return None
			# if not, use latest frame from camera
			else:
				usable_frame = self.latest_frame
				# if not french girl mode, use only if face is detected
				if not int(self.conf["FRENCH_GIRL"]):
					usable_frame = self.getFacesFromFrame(usable_frame)
				# otherwise, use whole image
				else:
					usable_frame = usable_frame

			if usable_frame is None:
				print "No face found in image"
				return None
			else:
				contours,h,w = self.generateContoursCustom(usable_frame)
				linelist = self.returnLineListFromContour(contours,h,w)
				if image:
					imagename = image.replace(".","").split("/")[-1]
					self.message = create_commands(linelist,imagename)
				else:
					self.message = create_commands(linelist)
				self.newMessage = True
		except Exception as e:
			print str(e)
			return None
		finally:
			self.allow_frameshot = True

	def getLinesFromFace(self):
		if not int(self.conf["FRENCH_GIRL"]):
			faceImage = self.getFacesFromFrame(self.latest_frame)
		else:
			faceImage = self.latest_frame
		if faceImage is None:
			print "No face found in image"
			return None
		else:
			contours,h,w = self.generateContoursCustom(faceImage)
			linelist = self.returnLineListFromContour(contours,h,w)
			self.message = create_commands(linelist)
			self.newMessage = True

	def returnLineListFromContour(self,contours,height,width):
		linelist = []
		board_y = int(self.conf["Y_MAX"])
		board_x = int(self.conf["X_MAX"])
		# check if height/width are not equal
		if height != width:
			# if height is greater, shrink board width proportionally
			if height > width:
				board_x = int(board_x*(float(width)/height))
			# if width is greater, shrink board height proportionally
			else:
				board_y = int(board_y*(float(height)/width))
		print board_y,board_x
		# scale contours properly to size of usable board
		for c in contours:
			currentline = []
			for point in c:
				newY = self.scaleInt(point[0][0],newMax=board_y,oldMax=height)
				newX = self.scaleInt(point[0][1],newMax=board_x,oldMax=width)
				currentline.append([newY,newX])
			linelist.append(currentline)
		return linelist

	def scaleInt(self,value,newMin=0,newMax=0,oldMin=0,oldMax=0):
		oldDiff = oldMax-oldMin
		newDiff = newMax-newMin
		return (((value-oldMin)*newDiff)/oldDiff)+newMin

	def checkIfReadyToDraw(self):
		return (self.readyToDraw,self.linesToDraw)

	def markDoneDrawing(self):
		self.readyToDraw = False
		self.linesToDraw = []

	def getFrame(self):
		# get image
		ret = False
		ret = self.cap.grab()
		frame = None
		if ret:
			ret,frame = self.cap.retrieve()
			#orig_frame = copy.copy(frame)
			cv2.imshow('frame',frame)
		else:
			#print 'frame could not be grabbed'
			pass
		
		return frame

	def getFacesFromFrame(self, frame):
		frame = frame.copy()
		gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
		faces = self.face_cascade.detectMultiScale(gray, 1.3, 5)

		#for (x,y,w,h) in faces:
			#cv2.rectangle(frame,(x,y),(x+w,y+h),(255,0,0),2)
		largestFace = self.findLargestFace(faces)

		if largestFace != None:
			x,y,w,h = largestFace
			#print "{},{}".format(w,h)
			# save largest face
			largestFaceImg = frame[y:y+h,x:x+w].copy()

			return largestFaceImg
		else:
			#print 'frame could not be grabbed'
			return None


	def findLargestFace(self,faces):
		largestArea = 0
		largestFace = None
		for (x,y,w,h) in faces:
			area = w*h
			if (area > largestArea):
				largestArea = area
				largestFace = (x,y,w,h)
		return largestFace


	def generateContoursCustom(self,frame):
		# resize image to be managable
		hframe,wframe = frame.shape[:2]
		if hframe == wframe:
			pass
		elif hframe > wframe:
			hframe = 640
			wframe = int(640*(float(wframe)/hframe))
		else:
			wframe = 640
			hframe = int(640*(float(hframe)/wframe))
		frame = cv2.resize(frame, (wframe,hframe), interpolation=cv2.INTER_AREA)
		# turn gray
		gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
		#gray = cv2.cvtColor(cv2.resize(frame,(0,0),fx=self.scale,fy=self.scale), cv2.COLOR_BGR2GRAY)

		# sharpen it
		gray = self.sharpen_image(gray)
		hframe,wframe = frame.shape[:2]
		#cv2.imshow('gray',gray)
		cv2.namedWindow('edged')
		lowerDef,upperDef = self.getCannyMinMaxOnly(gray)
		cv2.createTrackbar('Lower','edged',lowerDef,255,no_action)
		cv2.createTrackbar('Upper','edged',upperDef,255,no_action)
		while self.keep_running:
			lower = cv2.getTrackbarPos('Lower','edged')
			upper = cv2.getTrackbarPos('Upper','edged')
			edged = cv2.Canny(gray, lower, upper)
			cv2.resizeWindow('edged', wframe,hframe)
			cv2.imshow('edged',edged)
			key = cv2.waitKey(self.millidelay) & 0xFF
			if (key == ord("a")):
				__, contours, hierarchy = cv2.findContours(edged.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
				#print contours
				print ("CONTOURS GENERATED")
				h,w = edged.shape[:2]
				contourImg = np.zeros((h,w,3), np.uint8)
				cv2.drawContours(contourImg,contours,-1,(0,255,0),1)
				cv2.imshow('edged',contourImg)
				cv2.waitKey(500) & 0xFF
				cv2.destroyWindow('edged')
				cv2.waitKey(self.millidelay) & 0xFF
				return contours,h,w




	def generateContours(self,frame):
		# resize and turn gray
		gray = cv2.cvtColor(cv2.resize(frame,(0,0),fx=self.scale,fy=self.scale), cv2.COLOR_BGR2GRAY)
		# blur it
		# sharpen it
		gray = self.sharpen_image(gray)
		#gray = cv2.GaussianBlur(gray, (3,3), 0)
		# get canny edges off of frame
		cv2.imshow("gray",gray)
		cannyVersion = self.getCanny(gray)
		# get contours off of frame
		__, contours, hierarchy = cv2.findContours(cannyVersion.copy(),cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
		cv2.imshow("canny",cannyVersion)
		#print contours
		h,w = cannyVersion.shape[:2]
		return contours,h,w

	def getCannyMinMaxOnly(self,frame,sigma=0.10):
		v = np.median(frame)
		# apply automatic Canny edge detection using the computed median
		lower = int(max(0, (1.0 - sigma) * v))
		upper = int(min(255, (1.0 + sigma) * v))
		return lower,upper


	def getCanny(self,frame,sigma=0.10):
		# return cv2.Canny(frame,40,180)
		# compute the median of the single channel pixel intensities
		v = np.median(frame)
		# apply automatic Canny edge detection using the computed median
		lower = int(max(0, (1.0 - sigma) * v))
		upper = int(min(255, (1.0 + sigma) * v))
		edged = cv2.Canny(frame, lower, upper)
		return edged

	def increase_contrast(self,img):
		phi = 1.0
		theta = 1.0
		maxIntensity = 255.0
		newimg = (maxIntensity / phi) * (img / (maxIntensity / theta)) ** 0.5
		return array(newimg, dtype=uint8)


	def sharpen_image(self,img):
		imgBlurred = cv2.GaussianBlur(img, (5, 5), 0)
		sharpened = cv2.addWeighted(img, 1.5, imgBlurred, -0.5, 0)
		return sharpened


	def stop(self):
		# When everything is done, release the capture
		self.keep_running = False
		self.cap.release()
		cv2.destroyAllWindows()
