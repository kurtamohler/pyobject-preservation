#include <Python.h>

#include "mylib.h"

// TODO: I guess I don't need this in header necessarily?
struct MyClassBase {
  PyObject_HEAD
  MyClass* cdata;
};

bool MyClassBase_Check(PyObject* obj);

bool MyClassBase_init_module(PyObject* module);
