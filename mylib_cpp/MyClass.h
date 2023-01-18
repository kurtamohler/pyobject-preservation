#pragma once

#include <atomic>

#include "intrusive_ptr.h"
#include "PyObjectSlot.h"

namespace mylib_cpp {

class MyClass : public intrusive_ptr_target {
public:
  MyClass();
  ~MyClass();
  void print_message();
  double id();
  PyObjectSlot* pyobj_slot();

private:
  PyObjectSlot pyobj_slot_;
  double id_;
};

}
