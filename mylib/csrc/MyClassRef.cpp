#include "MyClassRef.h"
#include "MyClassBase.h"

struct MyClassRef {
  PyObject_HEAD

  // TODO: Change this to a `MyClass*` and add preservation and resurrection
  PyObject* obj;
};

static PyObject* MyClassRef_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  // TODO: This doesn't work because it's a tuple. Just use the proper arg parser
  if (!MyClassBase_Check(args)) {
    PyErr_SetString(
      PyExc_RuntimeError,
      "Expected arg 0 to be a `MyClass`");
    return NULL;
  }

  MyClassRef* self;
  self = (MyClassRef*) type->tp_alloc(type, 0);
  if (self) {
    // TODO: Change this to grab the underlying `MyClass*` and remove the
    // incref here
    self->obj = args;
    Py_INCREF(args);
  }
  return (PyObject*) self;
}

static PyObject* MyClassRef_get(MyClassRef* self, PyObject* Py_UNUSED(ignored)) {
  Py_INCREF(self->obj);
  return self->obj;
}

static PyMethodDef MyClassRef_methods[] = {
  {
    "get",
    (PyCFunction) MyClassRef_get,
    METH_NOARGS,
    "Returns the MyClass that this MyClassRef points to"
  },
  {NULL, NULL}
};

static PyTypeObject MyClassRefType = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "MyClassRef",
  .tp_basicsize = sizeof(MyClassRef),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = MyClassRef_methods,
  .tp_new = MyClassRef_new,

  // TODO: Add this:
  //.tp_dealloc = MyClassRef_dealloc,
};


bool MyClassRef_init_module(PyObject* module) {
  if (PyType_Ready(&MyClassRefType) < 0) {
    return false;
  }
  Py_INCREF(&MyClassRefType);
  if (PyModule_AddObject(module, "MyClassRef", (PyObject*) &MyClassRefType) < 0) {
    Py_DECREF(&MyClassRefType);
    return false;
  }
  return true;
}
