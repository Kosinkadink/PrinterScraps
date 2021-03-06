import time
from scrap.util import read_config, wait_till_done, ScrapException, UserInput
from comm.serialcomm import SerialComm
from scrap.scrapinterface import ScrapInterface

conf_dict = read_config("conf.txt")

# create ScrapInterface

scrap = ScrapInterface(conf_dict)
# for now, enter x and y coordinate to go to

# start user input thread
userInput = UserInput()
userInput.start()


while True:
	# wait a little
	time.sleep(0.05)
	# read message; if None, received nothing
	user_inp = userInput.returnMessage()
	if user_inp == None:
		continue
	
	if user_inp.lower().startswith("exit"):
		break
	else:
		try:
			doneYet = False
			inp = user_inp.strip()
			if inp.lower() == 'r':
				wait_till_done(scrap.reset())
				doneYet = True
			if not doneYet:	
				comm,coords = inp.split()
				coords = coords.strip().split(",")
				if len(coords) < 2:
					raise ValueError("use a comma to seperate coords")
				if comm == 's':
					wait_till_done(scrap.set_coords(coords))
				elif comm == 'sp':
					wait_till_done(scrap.set_coords(coords,passive=True))
		except ValueError,e:
			print str(e)
		except ScrapException,e:
			print str(e)
		else:
			print 'done!'

scrap.stop()
time.sleep(0.25)
