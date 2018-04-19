from time import sleep
from scrap.UserInput import UserInput
from scrap.scrapinterfaceasync import ScrapInterfaceAsync
from scrap.util import read_config, read_commands
from scrap.uiasync import UI_Async
from scrap.visionasync import Vision

# read configuration
conf_dict = read_config("conf.txt")
conf_dict["DEBUG"] = int(conf_dict["DEBUG"])

# variables for classes
userInput = None
scrap = None
vision = None
ui = None


def main():
	global userInput,scrap,ui,vision
	keepGoing = True
	
	#### START PROCESSES

	# start user input thread
	userInput = UserInput()
	userInput.start()
	# start scrap interface
	scrap = ScrapInterfaceAsync(conf_dict)
	scrap.start()
	# start vision
	vision = Vision(conf_dict)
	vision.start()
	# start UI
	ui = UI_Async(conf_dict)
	ui.start()

	#### PERFORM ACTIONS

	while keepGoing:
		# check for user input
		user_inp = userInput.returnMessage()
		ui_inp = ui.returnMessage()
		vision_inp = vision.returnMessage()
		inputs = [user_inp, ui_inp, vision_inp]
		for inp in inputs:
			if inp:
				try:
					if inp.lower() == "exit":
						keepGoing = False
						continue 
					elif inp.lower() == "cancel":
						scrap.cancel()
					elif inp.lower().startswith("print"):
						# attempt to get second argument
						x,imgname = inp.split()
						vision.processImage(imgname)
					elif inp.lower().startswith("load"):
						# attempt to get second argument
						x,filename = inp.split()
						commands = read_commands(filename)
						inputs.append(commands)
						print("{} loaded! {} commands added.".format(filename,len(commands)))
					else:
						scrap.processInput(inp)
				except AttributeError:
					scrap.processInput(inp)
				except Exception as e:
					print("ERROR: {}".format(str(e)))
		sleep(0.001)

def stopAll():
	userInput.stop()
	scrap.stop()
	vision.stop()
	ui.stop()
	#print(userInput.get_raised_exception())
	print(scrap.get_raised_exception())
	print(ui.get_raised_exception())



if __name__ == "__main__":
	# run code
	main()
	# clean up processes/threads
	stopAll()
