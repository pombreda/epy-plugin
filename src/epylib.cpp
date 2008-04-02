/*
This file is part of epy-plugin
Copyright (C) 2008 Marius BARBU

epy-plugin is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

epy-plugin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include "epylib.h"

using namespace epy;

void Object::Keep(PyObject *o /*= 0*/)
{
	if (o) {
		m_obj = o;
	} else {
		Py_XINCREF(m_obj);
	}
}

bool Object::HasAttr(const char *name)
{
	return PyObject_HasAttrString(m_obj, name) != 0;
}

bool Object::HasAttr(PyObject *name)
{
	return PyObject_HasAttr(m_obj, name) != 0;
}


PyObject* Object::GetAttr(const char *name)
{
	return PyObject_GetAttrString(m_obj, name);
}

PyObject* Object::GetAttr(PyObject *name)
{
	return PyObject_GetAttr(m_obj, name);
}

bool Object::SetAttr(const char *name, PyObject *value)
{
	return PyObject_SetAttrString(m_obj, name, value) != -1;
}

bool Object::SetAttr(PyObject *name, PyObject *value)
{
	return PyObject_SetAttr(m_obj, name, value) != -1;
}

bool Object::DelAttr(const char *name)
{
	return PyObject_DelAttrString(m_obj, name) != -1;
}

bool Object::DelAttr(PyObject *name)
{
	return PyObject_DelAttr(m_obj, name) != -1;
}

PyObject* Object::Repr()
{
	return PyObject_Repr(m_obj);
}

PyObject* Object::Str()
{
	return PyObject_Str(m_obj);
}

PyObject* Sequence::Get(int index)
{
	return PySequence_GetItem(m_obj, index);
}

int Sequence::Size()
{
	return PySequence_Size(m_obj);
}

const char* String::AsString()
{
	return PyString_AsString(m_obj);
}

int String::Size()
{
	return PyString_Size(m_obj);
}

bool Error::Check()
{
	return PyErr_Occurred() != 0;
}

void Error::Fetch(Object &type, Object &value, Object &trace)
{
	PyObject *t, *v, *tr;
	PyErr_Fetch(&t, &v, &tr);
	type.Keep(t);
	value.Keep(v);
	trace.Keep(tr);
}

void Error::Clear()
{
	PyErr_Clear();
}

bool Interpreter::Startup()
{
	Py_Initialize();
	return true;
}

void Interpreter::Shutdown()
{
	Py_Finalize();
}

PyObject* Interpreter::Import(const char *name)
{
	String modName(PyString_FromString(name));
	return Import(modName.PyValue());
}

PyObject* Interpreter::Import(PyObject *name)
{
	return PyImport_Import(name);
}

bool Interpreter::RunString(const char *cmd)
{
	return PyRun_SimpleString(cmd) != -1;
}

