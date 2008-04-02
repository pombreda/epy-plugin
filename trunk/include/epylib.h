#ifndef EMBED_PY_H__
#define EMBED_PY_H__

#undef _DEBUG // I don't want python_d.lib with trace_refs & stuff
#include "Python.h"

#ifndef NDEBUG
// re-enable _DEBUG if not on Release mode
#define _DEBUG
#endif

namespace epy {

class Object {
public:
	// unintialized object; default constructor is useful when creating arrays
	Object() : m_obj(0) {}
	explicit Object(PyObject *obj) : m_obj(obj) { }
	Object(const Object &o) : m_obj(o.m_obj) { Py_XINCREF(m_obj); }
	virtual ~Object() {	Py_XDECREF(m_obj); }

	PyObject* PyValue()  { return m_obj; }
	PyObject* operator ->() { return m_obj; }

	bool operator !=(PyObject *o) const { return m_obj != o; }
	bool operator ==(PyObject *o) const { return m_obj == o; }

	// helper method which acquires a new PyObject* or (if NULL) increases the refcount of the current object.
	void Keep(PyObject *o = 0);

	bool HasAttr(const char *name);
	bool HasAttr(PyObject *name);
	PyObject* GetAttr(const char *name);
	PyObject* GetAttr(PyObject *name);
	bool SetAttr(const char *name, PyObject *value);
	bool SetAttr(PyObject *name, PyObject *value);
	bool DelAttr(const char *name);
	bool DelAttr(PyObject *name);
	PyObject* Repr();
	PyObject* Str();
protected:
	PyObject *m_obj;
};

class Sequence : public Object {
public:
	Sequence() {}
	explicit Sequence(PyObject *obj) : Object(obj) {}
	PyObject* Get(int index);
	int Size();
};

class Module : public Object {
public:
	Module() {}
	explicit Module(PyObject *obj) : Object(obj) {}
};

class String : public Object {
public:
	String() {}
	explicit String(PyObject *obj) : Object(obj) {}
	const char* AsString();
	int Size();
};

class Error {
public:
	static bool Check();
	static void Fetch(Object &type, Object &value, Object &trace);
	static void Clear();
};

class Callable : public Object {
public:
	Callable(PyObject *obj);

	template<typename A>
	PyObject* Call(A a);
	template<typename A, typename B>
	PyObject* Call(A a, B b);
	template<typename A, typename B, typename C>
	PyObject* Call(A a, B b, C c);
	template<typename A, typename B, typename C, typename D>
	PyObject* Call(A a, B b, C c, D d);
};

class Interpreter {
public:
	bool Startup();
	void Shutdown();

	bool RunString(const char *cmd);

	PyObject* Import(const char *name);
	PyObject* Import(PyObject *name);
};

} // namespace epy

#endif // EMBED_PY_H__
