#include <Python.h>

static PyMethodDef module_methods[] = {
    { NULL, NULL }
};

static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_preservation",
    .m_methods = module_methods
};

PyMODINIT_FUNC
PyInit__preservation(void)
{
    return PyModuleDef_Init(&module);
}
