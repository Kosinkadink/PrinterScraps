from multiprocessing import Process, Pipe
from threading import Thread, Lock
from time import sleep


class DriverBase(Thread):

	def __init__(self, target, args=tuple(), kwargs=dict()):
		Thread.__init__(self)
		# Process Pipes
		self.comm_pipe = None
		self.keep_running = True
		self.raised_exception = None
		self.target = target
		self.args = args
		self.kwargs = kwargs
		# locks
		self.error_lock = Lock()
		# delay times
		self.process_check_delay = 0.001

	def run(self):
		"""
		Code that runs when thread instance gets started using .start()
		Returns: None
		"""
		try:
			self.comm_pipe, child_end = Pipe()
			# comm pipe of process is child end of main thread, same with error pipe
			self.kwargs["comm_pipe"] = child_end
			while self.keep_running:
				try:
					self.target(*self.args,**self.kwargs)
				except Exception as e:
					self.handle_error(e)
		except Exception as e:
			self.error_lock.acquire()
			self.raised_exception = e
			self.keep_running = False
			self.error_lock.release()

	def handle_input(self, input_obj):
		pass

	def handle_error(self, error):
		"""
		Either handles error in a safe way, or raises it to set self.raised_exception later on
		Returns: None
		Raises: Some form of exception, likely the error parameter
		"""
		print("RECEIVED ERROR")
		raise error

	def stop(self):
		"""
		Terminates thread cleanly
		Returns: None
		"""
		self.keep_running = False
		self.comm_pipe.send("EXIT")
		# close pipe
		self.comm_pipe.close()

	def set_delay_time(delay):
		self.process_check_delay = delay

	def is_properly_alive(self):
		"""
		Returns a boolean based on if thread is in an alive state
		Returns: boolean
		"""
		self.error_lock.acquire()
		keep_running = self.keep_running;
		raised_exception = self.raised_exception
		self.error_lock.release()
		return keep_running and raised_exception is None

	def get_raised_exception(self):
		self.error_lock.acquire()
		raised_exception = self.raised_exception
		self.error_lock.release()
		return raised_exception


class ThreadDriver(DriverBase):

	def __init__(self, target, args=tuple(), kwargs=dict()):
		# Initialization
		DriverBase.__init__(self, self.run_thread)
		# Process Data
		self.process_target = target
		self.process_args = args
		self.process_kwargs = kwargs
		self.process = None

	def run_thread(self, comm_pipe):
		"""
		Starts process and monitors its output
		Returns: None
		Raises: any exception sent by process via output pipe
		"""
		# comm pipe of process is child end of main thread, same with error pipe
		self.process_kwargs["comm_pipe"] = comm_pipe
		
		self.process = Thread(target=self.process_target,args=self.process_args,kwargs=self.process_kwargs)
		self.process.daemon = self.daemon
		self.process.start()
		# read input pipe
		while self.keep_running:
			# check if there is something to receive
			# if so, receive everything there is to receive
			while self.comm_pipe.poll() and self.keep_running:
				input_obj = self.comm_pipe.recv()
				# check if it is an exception
				if isinstance(input_obj, Exception):
					raise input_obj
				self.handle_input(input_obj)
			# sleep for a bit
			sleep(self.process_check_delay)

	def handle_input(self, input_obj):
		pass

class ProcessDriver(DriverBase):

	def __init__(self, target, args=tuple(), kwargs=dict()):
		# Initialization
		DriverBase.__init__(self, self.run_process)
		# Process Data
		self.process_target = target
		self.process_args = args
		self.process_kwargs = kwargs
		self.process = None
		# delay times
		self.process_check_delay = 0.001

	def run_process(self, comm_pipe):
		"""
		Starts process and monitors its output
		Returns: None
		Raises: any exception sent by process via output pipe
		"""
		# comm pipe of process is child end of main thread, same with error pipe
		self.process_kwargs["comm_pipe"] = comm_pipe
		
		self.process = Process(target=self.process_target,args=self.process_args,kwargs=self.process_kwargs)
		self.process.daemon = self.daemon
		self.process.start()
		# read input pipe
		while self.keep_running:
			# check if there is something to receive
			# if so, receive everything there is to receive
			while self.comm_pipe.poll() and self.keep_running:
				input_obj = self.comm_pipe.recv()
				# check if it is an exception
				if isinstance(input_obj, Exception):
					raise input_obj
				self.handle_input(input_obj)
			# sleep for a bit
			sleep(self.process_check_delay)

	def handle_input(self, input_obj):
		pass
