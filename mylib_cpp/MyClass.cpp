#include <iostream>
#include "mylib.h"

MyClass::MyClass() {
  std::cout << "in MyClass::MyClass()" << std::endl;
}

MyClass::~MyClass() {
  std::cout << "in MyClass::~MyClass()" << std::endl;
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
