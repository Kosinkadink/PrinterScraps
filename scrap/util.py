import os, sys, time

# add relevant dir to path
__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))  # directory from which this script is ran
main_dir = os.path.realpath(os.path.join(__location__,'../..'))
sys.path.insert(0, main_dir)


# use to block until command is done executing
def wait_till_done(commandObj):
	while not commandObj.checkDone():
		time.sleep(0.01)
	return commandObj.getResponse()


# exception used for scrap-related errors
class ScrapException(Exception):
	pass