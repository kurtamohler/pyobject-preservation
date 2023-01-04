#pragma once

#include <atomic>

#include "python_stub.h"
#include "PyInterpreter.h"

class MyClass {
public:
  MyClass();

  ~MyClass();

  void print_message();

  void set_pyobject(PyObject* pyobject);

  PyObject* pyobject();

  void set_owns_pyobject(bool owns_pyobject);

  bool owns_pyobject();

  void maybe_decref_pyobj();

private:
  PyObject* pyobject_;
  bool owns_pyobject_;

protected:
  std::atomic<PyInterpreter*> pyobj_interpreter_;
};
