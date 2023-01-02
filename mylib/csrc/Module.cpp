#include <Python.h>

#include "MyClassBase.h"
#include "MyClassRef.h"

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

  if (!MyClassBase_init_module(m)) {
    Py_DECREF(m);
    return NULL;
  }

  if (!MyClassRef_init_module(m)) {
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
