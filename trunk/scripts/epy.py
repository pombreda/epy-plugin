# This file is part of epy-plugin
# Copyright (C) 2008 Marius BARBU
# 
# epy-plugin is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# epy-plugin is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
