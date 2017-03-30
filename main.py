from scrap.util import read_config, wait_till_done, ScrapException
from comm.serialcomm import SerialComm
from scrap.scrapinterface import ScrapInterface

conf_dict = read_config("conf.txt")

# create ScrapInterface

scrap = ScrapInterface(conf_dict)

# for now, enter x and y coordinate to go to

while True:
	user_inp = raw_input("> ")
	if user_inp.lower().startswith("EXIT"):
		break
	else:
		try:
			x,y = user_inp.strip().split(",")
			wait_till_done(scrap.set_coords(x,y))
		except ValueError,e:
			print str(e)
		except ScrapException,e:
			print str(e)
		else:
			print 'done!'
