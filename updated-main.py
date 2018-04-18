from time import sleep
from scrap.UserInput import UserInput
from scrap.scrapinterface import ScrapInterface
from scrap.util import read_config
from scrap.ui import UI

# read configuration
conf_dict = read_config("conf.txt")
conf_dict["DEBUG"] = int(conf_dict["DEBUG"])
# create user input instance
userInput = UserInput()
# create ScrapInterface
scrap = ScrapInterface(conf_dict)
# create UI
ui = UI(conf_dict)

def main():
	keepGoing = True
	# start user input thread
	userInput.start()
	# perform actions
	while keepGoing:
		# perform UI actions
		ui.performUI()
		# check for user input
		user_inp = userInput.returnMessage()
		if user_inp != None:
			print user_inp
			if user_inp == "exit":
				keepGoing = False
				continue 
		sleep(0.01)

def stopAll():
	userInput.stop()
	scrap.stop()


if __name__ == "__main__":
	# run code
	main()
	# clean up processes/threads
	stopAll()
