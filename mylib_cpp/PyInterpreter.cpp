#include "PyInterpreter.h"

namespace mylib_cpp {
namespace impl {

void decref_nop(const PyInterpreter*, PyObject*) {
  return;
}

void PyInterpreter::disarm() {
  decref_fn_ = &decref_nop;
}

}
}
