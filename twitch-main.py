import time
from scrap.util import read_config, wait_till_done, ScrapException, UserInput
from comm.serialcomm import SerialComm
from scrap.scrapinterface import ScrapInterface
from twitch.irc import IRC

conf_dict = read_config("conf.txt")
twitch_dict = read_config("conf-twitch.txt")

# create ScrapInterface

scrap = ScrapInterface(conf_dict)
armIRC = IRC(twitch_dict)

# attempt connection
if not armIRC.connect():
	raise ValueError("could not connect to stream")

# for now, enter x and y coordinate to go to

# start user input thread
userInput = UserInput()
userInput.start()


while True:
	# wait a little
	time.sleep(0.2)
	parse_inputs = []
	twitch_inputs = armIRC.receiveMessages()
	#print twitch_inputs
	if isinstance(twitch_inputs, list):
		for twitch_dict in twitch_inputs:
			if twitch_dict != None:
				parse_inputs.append(twitch_dict)
	# read message; if None, received nothing
	user_inp = userInput.returnMessage()
	if user_inp == None:
		pass
	else:
		if user_inp.lower().startswith("exit"):
			break
		parse_inputs.append(user_inp)
	
	for parse_inp in parse_inputs:
		try:
			doneYet = False
			if isinstance(parse_inp,dict):
				parse_inp = parse_inp['message']
			inp = parse_inp.strip()
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
