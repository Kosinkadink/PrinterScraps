from DeviceComm import DeviceComm, CommRequest, DeviceCommException

# formatting:
# request = "<command char>\n"
# response = "d<response stuff...>\n"

class SerialComm(DeviceComm):
    # class for communication with devices
    # via serial communication
    # such as: arduinos

    def __init__(self, comm, maxTries = 2):
        DeviceComm.__init__(self, comm)
        # alias for comm
        self.serial = self.comm
        self.connected = False
        self.maxTries = maxTries
        self.initializeConnection()
        self.termChar = '\n'

    def initializeConnection(self):
        # wait until serial is connected
        if self.serial.isOpen():
            self.serial.close()
        self.serial.open()
        while not self.connected:
            time.sleep(0.1)
            self.serial.timeout = 1  # make serial non blocking
            serin = self.serial.read()
            if len(serin) > 0:
                self.connected = True
        self.serial.timeout = None  # make serial block
        print 'connection initialized'

    def performCommand(self, commReq):
        print("performing command in SerialComm performCommand")
        tries = 0
        response = None
        while tries < self.maxTries:
            self.serial.write(commReq.request + '\n')
            # wait for a response to change state of command Request
            serin = ""
            while not serin.endswith('\n'):
                serin += self.serial.read()
            
            print("got from serial read: %s" % serin)
            if serin[0] == 'n':
                response = 'BAD'
                time.sleep(0.1)
                self.serial.flushInput()
                self.serial.flushOutput()
                tries += 1
            else:
                response = serin.strip()[1:]  # remove new line char, take out first char
                print('command %s performed with response %s' % (commReq.request, str(response)))
                break
        if commReq.returnAsList:
            commReq.response = [response]  # set response
        else:
            commReq.response = response  # set response
        commReq.markDone()


# exception for reporting SerialComm-related errors
class SerialCommException(DeviceCommException):
    pass
