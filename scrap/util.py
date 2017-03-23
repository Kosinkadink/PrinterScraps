import os, sys, time

# add relevant dir to path
__location__ = os.path.realpath(
	os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
main_dir = os.path.realpath(os.path.join(__location__,'..'))
sys.path.insert(0, main_dir)


# use to block until command is done executing
def wait_till_done(commandObj):
	while not commandObj.checkDone():
		time.sleep(0.01)
	return commandObj.getResponse()

# returns dictionary of config values
def read_config(filename):
	c_dict = {}
	with open(os.path.join(main_dir,filename),"rb") as conf:
		for line in conf:
			try:
				key,val = line.strip().split('=')
				key = key.strip()
				val = val.strip()
				c_dict[key] = val
			except Exception,e:
				pass
	return c_dict

# exception used for scrap-related errors
class ScrapException(Exception):
	pass