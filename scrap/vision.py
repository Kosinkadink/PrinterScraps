import math
import os
import copy
import cv2
import threading
import numpy as np

__location__ = os.path.realpath(
	os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran


class Vision(threading.Thread):

	def __init__(self,conf,camera_object):
		self.conf = conf
		self.millidelay = int(self.conf["FRAME_MILLIDELAY"])
		threading.Thread.__init__(self)
		self.daemon = True
		self.cap = camera_object
		self.face_cascade = cv2.CascadeClassifier(os.path.join(__location__,'resources/haarcascade_frontalface_default.xml'))
		self.keep_running = True
		self.latest_frame = None
		self.readyToDraw = False
		self.linesToDraw = []
		self.scale = 0.33

	def run(self):
		while self.keep_running:
			self.latest_frame = self.getFrame()
			# use cv2 wait in order to display window
			key = cv2.waitKey(self.millidelay) & 0xFF
			if (key == ord("q")):
				self.stop()
			elif (key == ord("s")):
				self.getLinesFromFace()
				
	def getLinesFromFace(self):
		if not int(self.conf["FRENCH_GIRL"]):
			faceImage = self.getFacesFromFrame(self.latest_frame)
		else:
			faceImage = self.latest_frame
		if faceImage is None:
			print "No face found in image"
			return None
		else:
			contours,h,w = self.generateCountours(faceImage)
			linelist = self.returnLineListFromContour(contours,h,w)
			self.linesToDraw = linelist
			self.readyToDraw = True

	def returnLineListFromContour(self,contours,height,width):
		linelist = []
		board_y = int(self.conf["Y_MAX"])
		board_x = int(self.conf["X_MAX"])
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
			#cv2.rectangle(frame,(x,y),(x+w,y+h),(255,255,0),2)
			# height and width
			frameh,framew = frame.shape[:2]
			# figure out buffer area
			h_buffer = frameh/12;
			w_buffer = framew/12;
			# get center of face
			center_x,center_y = ((x+x+w)/2,(y+y+h)/2)
			#print "Buffer W: %s\nBuffer H: %s" % (w_buffer,h_buffer)
			#print "Center X: %s\nCenter Y: %s" % (center_x,center_y)
			#print "Frame W: %s\nFrame H: %s" % (framew,frameh)
			# find out where camera must move to focus better
			#return largest face
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

	def generateCountours(self,frame):
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

	def getCanny(self,frame):
		return cv2.Canny(frame,40,180)

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
