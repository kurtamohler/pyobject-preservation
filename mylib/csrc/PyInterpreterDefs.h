#pragma once

#include "mylib.h"

void concrete_decref_fn(PyObject* pyobj) {
  Py_DECREF(pyobj);
}

mylib_cpp::impl::PyInterpreter pyobj_interpreter(&concrete_decref_fn);
