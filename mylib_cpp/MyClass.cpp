#include <iostream>
#include "mylib.h"

MyClass::MyClass() :
  pyobject_(nullptr),
  owns_pyobject_(false)
{
  std::cout << "in MyClass::MyClass()" << std::endl;
}

MyClass::~MyClass() {
  std::cout << "in MyClass::~MyClass()" << std::endl;

  // TODO: If `owns_pyobject_ == true`, we need to decref it. At the moment,
  // the PyObject's memory just gets leaked. In PyTorch, TensorImpl obtains the
  // Python interpreter and calls the decref on the PyObject. But the stuff it
  // uses to accomplish that from within the C++ `MyClass` context is kind of
  // complicated and I don't fully understand it yet

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
