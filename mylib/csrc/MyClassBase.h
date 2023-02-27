#include <Python.h>
#include <memory>

#include "mylib.h"

struct MyClassBase {
  PyObject_HEAD
  mylib_cpp::intrusive_ptr<mylib_cpp::MyClass> cdata;

  // We keep a normal pointer to the underlying C++ object for when we
  // need to resurrect
  mylib_cpp::MyClass* cdata_weak;
};

bool MyClassBase_Check(PyObject* obj);

PyObject* MyClassBase_get_from_cdata(mylib_cpp::intrusive_ptr<mylib_cpp::MyClass> cdata);

bool MyClassBase_init_module(PyObject* module);
