#pragma once

#include <atomic>

#include "python_stub.h"
#include "PyInterpreter.h"

namespace mylib_cpp {

class PyObjectSlot {
public:
  PyObjectSlot() :
    pyobj_(nullptr),
    owns_pyobj_(false),
    pyobj_interpreter_(nullptr) {}

  void set_pyobj(PyObject* pyobj);
  PyObject* pyobj();
  void set_owns_pyobj(bool owns_pyobj);
  bool owns_pyobj();
  void maybe_decref_pyobj();
  void set_pyobj_interpreter(impl::PyInterpreter* pyobj_interpreter);

private:
  PyObject* pyobj_;
  bool owns_pyobj_;
  std::atomic<impl::PyInterpreter*> pyobj_interpreter_;
};

}
