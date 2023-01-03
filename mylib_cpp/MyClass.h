#pragma once

#include "python_stub.h"

class MyClass {
public:
  MyClass();

  ~MyClass();

  void print_message();

  void set_pyobject(PyObject* pyobject);

  PyObject* pyobject();

  void set_owns_pyobject(bool owns_pyobject);

  bool owns_pyobject();

private:
  PyObject* pyobject_;
  bool owns_pyobject_;
};
