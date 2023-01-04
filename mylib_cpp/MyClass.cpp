#include <iostream>
#include <stdexcept>

#include "MyClass.h"

MyClass::MyClass() :
  pyobject_(nullptr),
  owns_pyobject_(false)
{
  std::cout << "in MyClass::MyClass()" << std::endl;

  pyobj_interpreter_.load(std::memory_order_acquire)->disarm();
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
    if (pyobj_interpreter_ == nullptr || pyobject_ == nullptr) {
      throw std::runtime_error("Uh oh");
    }
    std::cout << "Calling python interpreter decref" << std::endl;
    pyobj_interpreter_.load(std::memory_order_acquire)->decref(pyobject_);
    pyobject_ = nullptr;
  }
}
