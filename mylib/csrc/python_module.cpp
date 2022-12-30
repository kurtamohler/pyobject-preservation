#include <Python.h>

#include "PyMyClass.h"

static PyMethodDef module_methods[] = {
  { NULL, NULL }
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_mylib",
  .m_methods = module_methods
};

PyMODINIT_FUNC
PyInit__mylib(void) {
  PyObject* m;

  m = PyModule_Create(&module);
  if (m == NULL) {
    return NULL;
  }

  if (!PyMyClass_init_module(m)) {
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
