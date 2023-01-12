#pragma once

#include <atomic>

#include "python_stub.h"
#include "PyInterpreter.h"

namespace mylib_cpp {

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

  void set_pyobj_interpreter(impl::PyInterpreter* pyobj_interpreter);

  double id();

private:
  PyObject* pyobject_;
  bool owns_pyobject_;
  std::atomic<impl::PyInterpreter*> pyobj_interpreter_;
  double id_;
};

}
