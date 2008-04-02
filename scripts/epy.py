from ctypes import *
import operator, sys

__all__ = ["export", "add_export"]

def add_export(func, ret=c_void_p, args=(), name=None, type=CFUNCTYPE):
	func._cfuncname = name or func.__name__
	func._cproc = type(ret, *args)(func) # keep a reference to the wrapped callback to avoid GC
	func._cfuncptr = cast(func._cproc, c_void_p).value
	
	try:
		sys.modules[func.__module__].EPY_EXPORT_LIST.append(func)
	except Exception, e:
		sys.modules[func.__module__].EPY_EXPORT_LIST = [func]
	else:
		# DLL export names must be sorted; easier to do in python :P
		sys.modules[func.__module__].EPY_EXPORT_LIST.sort(key=operator.attrgetter("_cfuncname"))

def export(ret=c_void_p, args=(), name=None, type=CFUNCTYPE):
	def decorator(func):
		add_export(func, ret, args, name, type)
		return func
	return decorator
