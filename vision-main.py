import time
from scrap.util import read_config, wait_till_done, ScrapException, UserInput, read_commands, create_commands
from comm.serialcomm import SerialComm
from scrap.scrapinterface import ScrapInterface
from scrap.ui import UI
from scrap.vision import Vision
import cv2

conf_dict = read_config("conf.txt")
conf_dict["DEBUG"] = int(conf_dict["DEBUG"])

# open video camera
camera_obj = cv2.VideoCapture(int(conf_dict["CAMERA_INDEX"]))
camera_obj.set(3, int(conf_dict["CAMERA_X"]))
camera_obj.set(4, int(conf_dict["CAMERA_Y"]))
# start vision
vision = Vision(conf_dict,camera_obj)
vision.start()
# create ScrapInterface
scrap = ScrapInterface(conf_dict)
# create UI
ui = UI(conf_dict)

# start user input thread
userInput = UserInput()
userInput.start()

keepGoing = True

while keepGoing:
	# wait a little
	#time.sleep(0.05)
	ui.performUI()
	# make empty command list
	parse_inputs = []

	# read message; if None, received nothing
	user_inp = userInput.returnMessage()
	if user_inp != None:
		parse_inputs.append(user_inp)
		
	# read UI message
	ui_inp = ui.returnMessage()
	if ui_inp != None:
		parse_inputs.append(ui_inp)
	
	# read Vision message
	drawing_ready,linesToDraw = vision.checkIfReadyToDraw()
	if drawing_ready:
		parse_inputs.extend(create_commands(linesToDraw))
		vision.markDoneDrawing()

	for parse_inp in parse_inputs:
		try:
			doneYet = False
			isFromUI = False
			# check if is from UI
			print parse_inp

			if isinstance(parse_inp,type("")):
				if parse_inp.lower() == 'exit':
					keepGoing = False
					break
				if parse_inp.lower() == 'read':
					parse_inputs.extend(read_commands("test_print"))
					continue
			elif isinstance(parse_inp,tuple):
				parse_inp = "s {0},{1}".format(parse_inp[0],parse_inp[1])
				isFromUI = True
			inp = parse_inp.strip()

			if inp.lower() == 'r':
				wait_till_done(scrap.reset())
				doneYet = True
			elif inp == 'u':
				wait_till_done(scrap.pen_up())
				doneYet = True
			elif inp == 'U':
				wait_till_done(scrap.pen_super_up())
				doneYet = True
			elif inp.lower() == 'd':
				wait_till_done(scrap.pen_down())
				doneYet = True

			if not doneYet:
				comm,coords = inp.split()
				coords = coords.strip().split(",")
				if len(coords) < 2:
					raise ValueError("use a comma to seperate coords")
				# do this if NOT from twitch
				if not isFromUI:
					if comm == 's':
						wait_till_done(scrap.set_coords(coords))
						pass
					elif comm == 'sp':
						wait_till_done(scrap.set_coords(coords,passive=True))
						pass
				# do this if from UI
				else:
					if comm == 's':
						wait_till_done(scrap.set_coords(coords,passive=True))
						pass
					elif comm == 'r':
						wait_till_done(scrap.reset())
						pass
		except ValueError,e:
			print str(e)
		except ScrapException,e:
			print str(e)

scrap.stop()
vision.stop()
time.sleep(0.25)
