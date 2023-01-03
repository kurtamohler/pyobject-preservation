// `MyClassRef` is a class that can retain a reference to a `MyClass` object in
// C++ while all Python references to the object have been deleted. This
// provides a way to exercise the PyObject preservation and resurrection
// pattern.

#include <memory>
#include <iostream>
#include <stdexcept>

#include "MyClassRef.h"
#include "MyClassBase.h"

struct MyClassRef {
  PyObject_HEAD
  std::shared_ptr<MyClass> ptr;
};

static PyObject* MyClassRef_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  PyObject* pyobject;

  if (!PyArg_ParseTuple(args, "O", &pyobject)) {
    return nullptr;
  }

  if (!MyClassBase_Check(pyobject)) {
    PyErr_SetString(
      PyExc_TypeError,
      "Expected arg 0 to be a `MyClass`");
    return NULL;
  }

  MyClassBase* base = (MyClassBase*)pyobject;

  MyClassRef* self;
  self = (MyClassRef*) type->tp_alloc(type, 0);
  if (self) {
    self->ptr = base->cdata;
  }
  return (PyObject*) self;
}

static void MyClassRef_dealloc(PyObject* self) {
  std::cout << "in MyClassRef_dealloc" << std::endl;
  ((MyClassRef*)self)->ptr.reset();
  Py_TYPE(self)->tp_free(self);
}

static PyObject* MyClassRef_get(MyClassRef* self, PyObject* Py_UNUSED(ignored)) {
  PyObject* pyobject = self->ptr->pyobject();

  // If the C++ MyClass owns the PyObject, then we need to switch ownership
  // back to the PyObject
  // TODO: This next part shouldn't be implemented in MyClassRef_get. Maybe should be
  // in a new method like MyClassBase_get_new_reference() or something.
  if (self->ptr->owns_pyobject()) {
    std::cout << "Resurrecting PyObject!!!!" << std::endl;

    // The PyObject refcount should remain 1 the whole time it's a zombie.
    // Also, after we resurrect it, the C++ `MyClass` won't have an owning ref
    // any more, but we'll have a new ref in Python. So we don't want to change
    // the refcount here
    if (Py_REFCNT(pyobject) != 1) {
      throw std::runtime_error((
        "For some reason, we're resurrecting a PyObject whose refcount is not "
        "exactly equal to 1!"));
    }

    MyClassBase* base = (MyClassBase*)pyobject;
    base->cdata = self->ptr;
    self->ptr->set_owns_pyobject(false);

  } else {
    Py_INCREF(pyobject);
  }
  return pyobject;
}

static PyMethodDef MyClassRef_methods[] = {
  {
    "get",
    (PyCFunction) MyClassRef_get,
    METH_NOARGS,
    "Returns the MyClass that this MyClassRef points to"
  },
  {NULL, NULL}
};

static PyTypeObject MyClassRefType = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "MyClassRef",
  .tp_basicsize = sizeof(MyClassRef),
  .tp_dealloc = MyClassRef_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = MyClassRef_methods,
  .tp_new = MyClassRef_new,
};


bool MyClassRef_init_module(PyObject* module) {
  if (PyType_Ready(&MyClassRefType) < 0) {
    return false;
  }
  Py_INCREF(&MyClassRefType);
  if (PyModule_AddObject(module, "MyClassRef", (PyObject*) &MyClassRefType) < 0) {
    Py_DECREF(&MyClassRefType);
    return false;
  }
  return true;
}
