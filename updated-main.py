from time import sleep
from scrap.UserInput import UserInput
from scrap.scrapinterfaceasync import ScrapInterfaceAsync
from scrap.util import read_config
from scrap.uiasync import UI_Async

# read configuration
conf_dict = read_config("conf.txt")
conf_dict["DEBUG"] = int(conf_dict["DEBUG"])

# variables for classes
userInput = None
scrap = None
ui = None


def main():
	global userInput,scrap,ui
	keepGoing = True
	
	#### START PROCESSES

	# start user input thread
	userInput = UserInput()
	userInput.start()
	# start scrap interface
	scrap = ScrapInterfaceAsync(conf_dict)
	scrap.start()
	# start UI
	ui = UI_Async(conf_dict)
	ui.start()

	#### PERFORM ACTIONS

	while keepGoing:
		# check for user input
		user_inp = userInput.returnMessage()
		ui_inp = ui.returnMessage()
		inputs = [user_inp,ui_inp]
		for inp in inputs:
			if inp != None:
				print inp
				if inp.lower() == "exit":
					keepGoing = False
					continue 
				elif inp.lower() == "cancel":
					scrap.cancel()
				else:
					scrap.processInput(inp)

			sleep(0.001)

def stopAll():
	userInput.stop()
	scrap.stop()
	ui.stop()
	#print(userInput.get_raised_exception())
	print(scrap.get_raised_exception())
	print(ui.get_raised_exception())



if __name__ == "__main__":
	# run code
	main()
	# clean up processes/threads
	stopAll()
