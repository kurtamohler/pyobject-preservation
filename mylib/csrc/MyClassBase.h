#include <Python.h>
#include <memory>

#include "mylib.h"

struct MyClassBase {
  PyObject_HEAD
  std::shared_ptr<MyClass> cdata;
};

bool MyClassBase_Check(PyObject* obj);

PyObject* MyClassBase_get_from_cdata(std::shared_ptr<MyClass> cdata);

bool MyClassBase_init_module(PyObject* module);
