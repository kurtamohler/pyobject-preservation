#pragma once

#include "python_stub.h"

class MyClass {
public:
  MyClass();

  ~MyClass();

  void print_message();

private:
  PyObject* pyobj_;
};
