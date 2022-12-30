#include <Python.h>

#include "PyMyClass.h"
#include "mylib.h"

struct PyMyClass {
  PyObject_HEAD
  MyClass* cdata;
};

static PyObject* PyMyClass_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  PyMyClass* self;
  self = (PyMyClass*) type->tp_alloc(type, 0);
  if (self != NULL) {
    self->cdata = new MyClass();
  }
  return (PyObject*) self;
}

static void PyMyClass_dealloc(PyMyClass* self) {
  if (self->cdata) {
    delete self->cdata;
  }
  Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* PyMyClass_print_message(PyMyClass* self, PyObject* Py_UNUSED(ignored)) {
  self->cdata->print_message();
  return Py_None;
}

static PyMethodDef PyMyClass_methods[] = {
  {
    "print_message",
    (PyCFunction) PyMyClass_print_message,
    METH_NOARGS,
    "Prints a message"
  },
  {NULL, NULL}
};

static PyTypeObject PyTypeMyClass = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "MyClass",
  .tp_basicsize = sizeof(PyMyClass),
  .tp_dealloc = (destructor) PyMyClass_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = PyMyClass_methods,
  .tp_new = PyMyClass_new,
};

bool PyMyClass_init_module(PyObject* module) {
  if (PyType_Ready(&PyTypeMyClass) < 0) {
    return false;
  }

  Py_INCREF(&PyTypeMyClass);
  if (PyModule_AddObject(module, "_MyClassBase", (PyObject*) &PyTypeMyClass) < 0) {
    Py_DECREF(&PyTypeMyClass);
    return false;
  }

  return true;
}
