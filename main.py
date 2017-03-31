import time
from scrap.util import read_config, wait_till_done, ScrapException
from comm.serialcomm import SerialComm
from scrap.scrapinterface import ScrapInterface

conf_dict = read_config("conf.txt")

# create ScrapInterface

scrap = ScrapInterface(conf_dict)

# for now, enter x and y coordinate to go to

while True:
	user_inp = raw_input("> ")
	if user_inp.lower().startswith("exit"):
		break
	else:
		try:
			comm,coords = user_inp.strip().split()
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
