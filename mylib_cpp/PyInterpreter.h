#pragma once
#include "python_stub.h"

namespace mylib_cpp {
namespace impl {

struct PyInterpreter {
  using decref_signature = void (PyObject*);

  PyInterpreter(decref_signature* decref_fn)
    : decref_fn_(decref_fn) {}

  decref_signature* decref_fn_;

  void decref(PyObject* pyobj) const {
    return (*decref_fn_)(pyobj);
  }

  void disarm();
};

}
}
