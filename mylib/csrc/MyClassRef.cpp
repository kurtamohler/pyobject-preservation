// `MyClassRef` is a class that can retain a reference to a `MyClass` object in
// C++ while all Python references to the object have been deleted. This
// provides a way to exercise the PyObject preservation and resurrection
// pattern.
//
#include <memory>

#include "MyClassRef.h"
#include "MyClassBase.h"

struct MyClassRef {
  PyObject_HEAD

  // TODO: Once `MyClass` has PyObject preservation/resurrection, change this
  // to a `MyClass*`, and don't incref/decref the `MyClassBase` PyObject in the
  // new/dealloc methods below.
  PyObject* obj;
  //std::shared_ptr<MyClass> obj;
};

static PyObject* MyClassRef_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  MyClassRef* py_obj;

  if (!PyArg_ParseTuple(args, "O", &py_obj)) {
    return nullptr;
  }

  if (!MyClassBase_Check((PyObject*)py_obj)) {
    PyErr_SetString(
      PyExc_TypeError,
      "Expected arg 0 to be a `MyClass`");
    return NULL;
  }

  MyClassRef* self;
  self = (MyClassRef*) type->tp_alloc(type, 0);
  if (self) {
    self->obj = (PyObject*)py_obj;
    Py_INCREF(self->obj);
  }
  return (PyObject*) self;
}

static void MyClassRef_dealloc(PyObject* self) {
  Py_DECREF(((MyClassRef*)self)->obj);
  Py_TYPE(self)->tp_free(self);
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
  .tp_dealloc = MyClassRef_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = MyClassRef_methods,
  .tp_new = MyClassRef_new,
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
