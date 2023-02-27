#include "PyInterpreter.h"

namespace mylib_cpp {
namespace impl {

void decref_nop(PyObject*) {
  return;
}

void PyInterpreter::disarm() {
  decref_fn_ = &decref_nop;
}

}
}
