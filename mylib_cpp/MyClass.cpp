#include <iostream>

#include "MyClass.h"

namespace mylib_cpp {

static double get_new_id() {
  static double id_ = 0;

  return id_++;
}

MyClass::MyClass() :
  id_(get_new_id())
{
  std::cout << "in MyClass::MyClass()" << std::endl;
}

MyClass::~MyClass() {
  std::cout << "in MyClass::~MyClass()" << std::endl;
}

void MyClass::print_message() {
  std::cout << "in MyClass::print_message()" << std::endl;
}

double MyClass::id() {
  return id_;
}

PyObjectSlot* MyClass::pyobj_slot() {
  return &pyobj_slot_;
}


}
