#include <Python.h>

#include "mylib.h"

struct MyClassBase {
  PyObject_HEAD
  MyClass* cdata;
};

bool MyClassBase_Check(PyObject* obj);

bool MyClassBase_init_module(PyObject* module);
