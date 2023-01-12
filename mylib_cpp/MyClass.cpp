#include <iostream>
#include <stdexcept>

#include "MyClass.h"

namespace mylib_cpp {

static double get_new_id() {
  static double id_ = 0;

  return id_++;
}

MyClass::MyClass() :
  pyobject_(nullptr),
  owns_pyobject_(false),
  pyobj_interpreter_(nullptr),
  id_(get_new_id())
{
  std::cout << "in MyClass::MyClass()" << std::endl;
}

MyClass::~MyClass() {
  std::cout << "in MyClass::~MyClass()" << std::endl;

  maybe_decref_pyobj();
}

void MyClass::print_message() {
  std::cout << "in MyClass::print_message()" << std::endl;
}

void MyClass::set_pyobject(PyObject* pyobject) {
  pyobject_ = pyobject;
}

PyObject* MyClass::pyobject() {
  return pyobject_;
}

void MyClass::set_owns_pyobject(bool owns_pyobject) {
  owns_pyobject_ = owns_pyobject;
}

bool MyClass::owns_pyobject() {
  return owns_pyobject_;
}

void MyClass::maybe_decref_pyobj() {
  if (owns_pyobject()) {
    if (pyobject_ == nullptr) {
      throw std::runtime_error("Owns PyObject, but got a null PyObject");
    }
    if (pyobj_interpreter_ == nullptr) {
      throw std::runtime_error("Owns PyObject, but got a null interpreter");
    }
    std::cout << "Calling python interpreter decref" << std::endl;
    pyobj_interpreter_.load(std::memory_order_acquire)->decref(pyobject_);
    pyobject_ = nullptr;
  }
}

void MyClass::set_pyobj_interpreter(impl::PyInterpreter* pyobj_interpreter) {
  pyobj_interpreter_.store(pyobj_interpreter);
}

double MyClass::id() {
  return id_;
}

}
