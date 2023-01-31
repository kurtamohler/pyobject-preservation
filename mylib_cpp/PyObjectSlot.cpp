#include <iostream>

#include "PyObjectSlot.h"
#include "Exceptions.h"

namespace mylib_cpp {

void PyObjectSlot::set_pyobj(PyObject* pyobj) {
  pyobj_ = pyobj;
}

PyObject* PyObjectSlot::pyobj() {
  return pyobj_;
}

void PyObjectSlot::set_owns_pyobj(bool owns_pyobj) {
  owns_pyobj_ = owns_pyobj;
}

bool PyObjectSlot::owns_pyobj() {
  return owns_pyobj_;
}

void PyObjectSlot::maybe_decref_pyobj() {
  if (owns_pyobj()) {
    MYLIB_ASSERT(
      pyobj_ != nullptr,
      "Owns PyObject, but got a null PyObject");
    MYLIB_ASSERT(
      pyobj_interpreter_ != nullptr,
      "Owns PyObject, but got a null interpreter");
    std::cout << "Calling python interpreter decref" << std::endl;
    pyobj_interpreter_.load(std::memory_order_acquire)->decref(pyobj_);
    pyobj_ = nullptr;
  }
}

void PyObjectSlot::set_pyobj_interpreter(impl::PyInterpreter* pyobj_interpreter) {
  pyobj_interpreter_.store(pyobj_interpreter);
}

}
