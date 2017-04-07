import os
import socket
import re

# re inputs were obtained from here: https://pastebin.com/MDC0RZDp

# class used to read twitch chat messages 
class IRC:
	
	def __init__(self, conf):
		self.conf = conf
		self.username = self.conf['USERNAME']
		self.oauth = self.conf['OAUTH']
		self.s = None
	
	
	def receiveMessages(self):
		try:
			data = self.s.recv(1024)
		except Exception,e:
			#print str(e)
			return False
		# if no data received, connection was lost!
		if not data:
			self.connect()
			return None
		# check if has message
		if self.checkForMessage(data):
			try:
				data_list = data.split('\r\n')
			except Exception,e:
				#print str(e)
				return []
			else:
				message_list = []
				for line in data_list:
					message_list.append(self.parseMessage(line))
				return message_list
			#return [self.parse_message(line) for line in filter(None, data.split('\r\n'))]
	
	
	def connect(self):
		# create socket
		s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		s.settimeout(1);
		ip = "irc.twitch.tv"
		port = 6667
		# attempt to connect
		print 'attempting connection...'
		try:
			s.connect((ip,port))
		except Exception,e:
			print "Connection attempt failed: {}".format(str(e))
			return False
		print 'connection successful!'
	
		s.sendall("USER {}\r\n".format(self.username))
		s.sendall("PASS {}\r\n".format(self.oauth))
		s.sendall("NICK {}\r\n".format(self.username))
			
		# check if successfully logged in
		if not self.checkIfLoggedIn:
			print 'credentials were NOT accepted'
			return False
		# set class socket to this one
		self.s = s	
		# join into stream chat
		s.sendall("JOIN #{}\r\n".format(self.username))
		s.recv(1024);
		return True

	def checkIfLoggedIn(self, data):
		if not re.match(r'^:(testserver\.local|tmi\.twitch\.tv) NOTICE \* :Login unsuccessful\r\n$', data):
			return True
		else:
			return False
	
	# check if message exists
	def checkForMessage(self, data):
		return re.match(r'^:[a-zA-Z0-9_]+\![a-zA-Z0-9_]+@[a-zA-Z0-9_]+(\.tmi\.twitch\.tv|\.testserver\.local) PRIVMSG #[a-zA-Z0-9_]+ :.+$', data)

	# parse for message: 
	def parseMessage(self, data):
		try:
			return {
				'channel': re.findall(r'^:.+\![a-zA-Z0-9_]+@[a-zA-Z0-9_]+.+ PRIVMSG (.*?) :', data)[0],
				'user': re.findall(r'^:([a-zA-Z0-9_]+)\!', data)[0],
				'message': re.findall(r'PRIVMSG #[a-zA-Z0-9_]+ :(.+)', data)[0]
			}
		except Exception,e:
			return None	
