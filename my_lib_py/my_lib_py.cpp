#include <Python.h>
#include <iostream>

#include "my_lib_cpp.h"

static PyMethodDef module_methods[] = {
  { NULL, NULL }
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  .m_name = "_my_lib",
  .m_methods = module_methods
};

PyMODINIT_FUNC
PyInit__my_lib(void)
{
  std::cout << "in PyInit__my_lib()" << std::endl;
  MyClass();
  return PyModuleDef_Init(&module);
}
