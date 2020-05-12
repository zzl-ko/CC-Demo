#-*- coding=utf-8 -*-
__license__ = "GPL (General Public License)"
__author__  = "Kevin Oscar <zrzxlfe@gmail.com>"
__date__    = "2015-01-29 (version 0.1)"

import sys
import time
import Queue
import logging
import threading

from serial import Serial
from serial import SerialException, SerialTimeoutException
from serial import FIVEBITS, SIXBITS, SEVENBITS, EIGHTBITS
from serial import PARITY_NONE, PARITY_EVEN, PARITY_ODD, PARITY_MARK, PARITY_SPACE
from serial import STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO

#from __init__ import SerialError as se

_instance = None

########################################################################
class gplSerial():
	"""
	This is a class provide you some easier interfaces to operate serial port.
	"""

	#----------------------------------------------------------------------
	def __init__(self, port=None, baudrate=None, kwargs=None, sendQueneSize=0, recviveQueneSize=0, enable_logger=True):
		"""Constructor"""
		self._serial = None
		
		self._logger = logging.getLogger(__name__)
		if enable_logger:
			logging.basicConfig(level=logging.DEBUG)
		else:
			logging.basicConfig()
			
		self._alwaysSendChecksum = False
		
		self._sendQueneSize = sendQueneSize
		#Quene is thread-safely, so lock is not necessary
		self._write_lock = threading.Lock()
		self._write_quene = Queue.Queue(sendQueneSize)
		self._send_terminate = False
		self._sender = threading.Thread(target=self._send)
		
		self._recviveQueneSize = recviveQueneSize
		self._read_lock = threading.Lock()
		self._read_quene = Queue.Queue(recviveQueneSize)
		self._receive_terminate = False
		self._receiver = threading.Thread(target=self._receive)
		
		if port and baudrate:
			self._open(port, baudrate, kwargs)
	
	#----------------------------------------------------------------------		
	def __del__(self):
		self._close()
			
	#----------------------------------------------------------------------
	def _log(self, msg, level="DEBUG"):
		if self._logger:
			if level == "DEBUG":
				self._logger.debug(msg)
			elif level == "INFO":
				self._logger.info(msg)
			elif level == "ERROR":
				self._logger.error(msg)
				
	#----------------------------------------------------------------------
	def _start_rsloop(self):
		""" _start_rsloop """
		self._receiver.setDaemon(True)
		self._sender.setDaemon(True)
		self._receiver.start()
		self._sender.start()
		
	#----------------------------------------------------------------------
	def _open(self, port="COM10", baudrate=115200, kwargs=None):
		options= {"bytesize": EIGHTBITS, "parity": PARITY_NONE, "stopbits": STOPBITS_ONE, "timeout": None, 
				  "xonxoff": False, "rtscts": False, "writeTimeout": None, "dsrdtr": False, "interCharTimeout": None}
		if isinstance(kwargs, dict):
			for key in kwargs.iterkeys():
				if key in options.keys():
					options[key] = kwargs[key]
					
		try:
			self._log("Connecting to: %s" % port)
			self._serial = Serial(str(port), int(baudrate), options["bytesize"], options["parity"], options["stopbits"], options["timeout"], 
			                     options["xonxoff"], options["rtscts"], options["writeTimeout"], options["dsrdtr"], options["interCharTimeout"])
			self._serial.flushInput()
			self._serial.flushOutput()
			self._start_rsloop()
		except SerialException as e:
			self._log("Failed to open serial port: %s, cause: %s" %(port, str(e)), "ERROR")
			raise se("Failed to open serial port: %s, cause: %s" %(port, str(e)))
		except:
			self._log("Unexpected error while connecting to serial port: %s, cause: %s" %(port, str(sys.exc_info()[0])), "ERROR")
			raise se("Unexpected error while connecting to serial port: %s, cause: %s" %(port, str(sys.exc_info()[0])))
	
	#----------------------------------------------------------------------
	def _close(self):
		if self._serial:
			self._send_terminate = True
			self._receive_terminate = True
			self._serial.close()
			self._serial = None
	
	#----------------------------------------------------------------------		
	def _send(self):
		while not self._send_terminate:
			(data, sendCheckSum) = self._write_quene.get(True)
			if self._serial and self._serial.writable():
				self._doSend(data, sendCheckSum)
			elif not self._serial:
				self._log("Unexpected error while writing serial port!", level="ERROR")
				self._close()
	
	#----------------------------------------------------------------------
	def _doSend(self, data, sendCheckSum=False):
		if sendCheckSum or self._alwaysSendChecksum:
			checkSum = self._calcCheckSum(data, parity)
			self._doSendWithCheckSum(data, checkSum)
		else:
			self._doSendWithoutCheckSum(data)
	
	#----------------------------------------------------------------------		
	def _doSendWithCheckSum(self, data, checkSum):
		pass
	
	#----------------------------------------------------------------------
	def _doSendWithoutCheckSum(self, data):
		self._log("Send: %s" % data)
		try:
			self._serial.write(data)
		except SerialTimeoutException as e:
			self._log("Serial timeout while writing to serial port, trying again.", "INFO")
			try:
				self._serial.write(data)
			except SerialException as e:
				self._log("Unexpected error while writing serial port", "ERROR")
				raise se("Unexpected error while writing serial port")
		except:
			self._log("Unexpected error while writing serial port", "ERROR")
			self._close()
			raise se("Unexpected error while writing serial port")
			
	#----------------------------------------------------------------------
	def _receive(self):
		while not self._receive_terminate:
			data = self._doReceive()
			if data and not self._read_quene.full():
				self._read_quene.put(data, False)
	
	#----------------------------------------------------------------------
	def _doReceive(self):
		buf = []
		if not self._serial:
			self._log("Unexpected error while reading serial port!", level="ERROR")
			self._close()
			return None
		else:
			buf += self._serial.read()
		time.sleep(0.02)
		if self._serial and self._serial.readable() and self._serial.inWaiting():
			while self._serial.inWaiting():
				try:
					buf += self._serial.read()
				except:
					self._log("Unexpected error while reading serial port!", level="ERROR")
					self._close()
					return None
		self._log("Receive: %s" % buf)
		return buf

	#----------------------------------------------------------------------
	def connect(self, port="COM10", baudrate=115200, **kwargs):
		if self._serial:
			self._close()
		try:
			self._open(port, baudrate, kwargs)
		except SerialException as e:
			raise se("Failed to open serial port: %s, cause: %s" %(port, str(e)))
		except:
			raise se("Unexpected error while connecting to serial port: %s, cause: %s" %(port, str(sys.exc_info()[0])))
	
	#----------------------------------------------------------------------
	def isConnected(self):
		return self._serial is not None
		
	#----------------------------------------------------------------------
	def disconnect(self):
		if self.isConnected():
			self._close()
			
	#----------------------------------------------------------------------
	def leaveAndGetSerialInstance(self):
		if self._serial != None:
			self._send_terminate = True
			self._receive_terminate = True
			serial = self._serial
			self._serial = None
			return serial
		return None
	
	#----------------------------------------------------------------------	
	def send(self, data, sendCheckSum=False, timeout=0):
		#self._write_quene.put((data, sendCheckSum),False)
		if isinstance(data, (str, list)):
			if isinstance(data, list):
				for c in data:
					if isinstance(c, str):
						try:
							if len(c) != 1:
								return
						except:
							self._log("Unexpected error char", "ERROR")
					else:
						return
		else:
			return
		
		#self._log("data to hex: %r" %data.encode("hex"))
		
		if not self._write_quene.full():
			self._write_quene.put((data, sendCheckSum), False)
			return
		if self._write_quene.full():
			self._log("Send Quene is full!")
			if timeout > 0:
				self._write_quene.put((data, sendCheckSum), True, timeout)
			return
		
	#----------------------------------------------------------------------	
	def receive(self, timeout=None):
		data = None
		if timeout is None:
			data = self._read_quene.get(True)
		elif self._read_quene.empty():
			self._log("Send Quene is empty!")
			if timeout > 0:
				data = self._read_quene.get(timeout=timeout)
		return data


#----------------------------------------------------------------------
def serialManager(port="/dev/ttyUSB0", baudrate=115200, kwargs=None, 
                  sendQueneSize=0, recviveQueneSize=0, enable_logger=False):
	global _instance
	if _instance is None:
		if port and baudrate:
			try:
				_instance = gplSerial(port, baudrate, kwargs, sendQueneSize, recviveQueneSize, enable_logger)
			except:
				print "serialManager: Unexpected Error"
	return _instance

		
#----------------------------------------------------------------------	
def debugSerial():
	serial = gplSerial(port="/dev/pts/4", baudrate=9600, enable_logger=True)

	#def DebugSend():
		#timer = threading.Timer()
	while True:
		recv = serial.receive()
		if recv is not None:
			print "recv: ", recv
			#serial.send("\x5a\xa5{0}{1}{2}".format(chr(len(recv)+3), "\x82\x03\x30", recv))
			#serial.send(recv)
			#serial.send("\ntest1\n") #true
			#serial.send(["\ntest2\n"]) #false
			#serial.send(["\n","t","e","s","t","3","\n"]) #true
			#serial.send(["\n","t","es","t","4","\n"]) #false
			#serial.send(["\n","\x74","\x65","\x73","\x74","\x35","\n"]) #true
			#serial.send(["\n","\x74","\x65","\x73\x74","\x36","\n"]) #false
			#serial.send("\n\x74\x65\x73\x74\x37\n") #true
			#serial.send("{0}{1}{2}".format('\xD7\xD3\xE8\xF9',chr(7), r"欣儿")) #串口以十六进制字符形式发送数字
				
	#serial2 = serial.leaveAndGetSerialInstance()
	#print serial2
	#serial2.write("serial test")
	#serial2.close()

    
if __name__ == "__main__":
	debugSerial()
