#include <Python.h>
#include <iostream>

#include "my_lib_cpp.h"

struct MyClassPy {
  PyObject_HEAD
  MyClass* cdata;
};

static PyObject* MyClassPy_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  MyClassPy* self;
  self = (MyClassPy*) type->tp_alloc(type, 0);
  if (self != NULL) {
    self->cdata = new MyClass();
  }
  return (PyObject*) self;
}

static int MyClassPy_init(MyClassPy* self, PyObject* args, PyObject* kwargs) {
  return 0;
}

static void MyClassPy_dealloc(MyClassPy* self) {
  if (self->cdata) {
    delete self->cdata;
  }
  Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* MyClassPy_print_message(MyClassPy* self, PyObject* Py_UNUSED(ignored)) {
  self->cdata->print_message();
  return Py_None;
}

static PyMethodDef MyClassPy_methods[] = {
  {
    "print_message",
    (PyCFunction) MyClassPy_print_message,
    METH_NOARGS,
    "Prints a message"
  },
  {NULL, NULL}
};

static PyTypeObject MyClassPyType = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "MyClass",
  .tp_basicsize = sizeof(MyClassPy),
  .tp_dealloc = (destructor) MyClassPy_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = MyClassPy_methods,
  .tp_init = (initproc) MyClassPy_init,
  .tp_new = MyClassPy_new,
};

static PyMethodDef module_methods[] = {
  { NULL, NULL }
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_my_lib",
  .m_methods = module_methods
};

PyMODINIT_FUNC
PyInit__my_lib(void) {
  PyObject* m;
  if (PyType_Ready(&MyClassPyType) < 0) {
    return NULL;
  }

  m = PyModule_Create(&module);
  if (m == NULL) {
    return NULL;
  }

  Py_INCREF(&MyClassPyType);
  if (PyModule_AddObject(m, "MyClass", (PyObject*) &MyClassPyType) < 0) {
    Py_DECREF(&MyClassPyType);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
