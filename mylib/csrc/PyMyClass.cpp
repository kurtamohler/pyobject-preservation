#include <Python.h>

#include "PyMyClass.h"
#include "mylib.h"

struct PyMyClass {
  PyObject_HEAD
  MyClass* cdata;
};

static PyObject* PyMyClass_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);

static void PyMyClass_clear(PyMyClass* self) {
  if (self->cdata) {
    delete self->cdata;
  }
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

void PyMyClass_subclass_dealloc(PyObject* self) {
  PyMyClass_clear((PyMyClass*)self);
  Py_TYPE(self)->tp_free((PyObject*) self);
}

int PyMyClassMeta_init(PyObject* cls, PyObject* args, PyObject* kwargs) {
  ((PyTypeObject*)cls)->tp_dealloc = (destructor)PyMyClass_subclass_dealloc;
  return 0;
}

struct PyMyClassMeta {
  PyHeapTypeObject base;
};

static PyTypeObject PyMyClassMetaType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  .tp_name = "_MyClassMeta",
  .tp_basicsize = sizeof(PyMyClassMeta),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_init = PyMyClassMeta_init,
};

static PyTypeObject PyMyClassType = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "_MyClassBase",
  .tp_basicsize = sizeof(PyMyClass),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = PyMyClass_methods,
  .tp_new = PyMyClass_new,
};

static PyObject* PyMyClass_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  if (type == &PyMyClassType) {
    PyErr_SetString(
      PyExc_RuntimeError,
      "_MyClassBase cannot be instantiated directly. Please use a subclass");
    return NULL;
  }
  PyMyClass* self;
  self = (PyMyClass*) type->tp_alloc(type, 0);
  if (self) {
    self->cdata = new MyClass();
  }
  return (PyObject*) self;
}

bool PyMyClass_init_module(PyObject* module) {

  PyMyClassMetaType.tp_base = &PyType_Type;
  if (PyType_Ready(&PyMyClassMetaType) < 0) {
    return false;
  }
  Py_INCREF(&PyMyClassMetaType);
  if (PyModule_AddObject(module, "_MyClassMeta", (PyObject*)&PyMyClassMetaType) < 0) {
    Py_DECREF(&PyMyClassMetaType);
    return false;
  }

  if (PyType_Ready(&PyMyClassType) < 0) {
    return false;
  }
  Py_INCREF(&PyMyClassType);
  if (PyModule_AddObject(module, "_MyClassBase", (PyObject*) &PyMyClassType) < 0) {
    Py_DECREF(&PyMyClassType);
    return false;
  }

  return true;
}
