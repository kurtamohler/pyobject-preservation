#pragma once

void concrete_decref_fn(const PyInterpreter* self, PyObject* pyobj) {
  Py_DECREF(pyobj);
}

PyInterpreter pyobj_interpreter(&concrete_decref_fn);
