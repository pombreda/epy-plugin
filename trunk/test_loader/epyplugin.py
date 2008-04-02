import sys
from ctypes import *

import epy
	
@epy.export(ret=c_int, args=(c_char_p,), name="TestHello", type=WINFUNCTYPE)
def hello(name):
	print "Hello %s!" % name
	return 0
	
@epy.export(args=(c_char_p,), name="TestGoodbye")
def goodbye(name):
	print "Goodbye %s!!!" % name
		
print "this module is called:", __name__
print "interpreter started by:", sys.executable
