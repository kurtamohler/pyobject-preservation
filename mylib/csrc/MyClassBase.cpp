#include <iostream>
#include "MyClassBase.h"

static PyObject* MyClassBase_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);

static void MyClassBase_clear(MyClassBase* self) {
  // TODO: I might not actually need this function, we'll see
}

static PyObject* MyClassBase_print_message(MyClassBase* self, PyObject* Py_UNUSED(ignored)) {
  self->cdata->print_message();
  return Py_None;
}

static PyMethodDef MyClassBase_methods[] = {
  {
    "print_message",
    (PyCFunction) MyClassBase_print_message,
    METH_NOARGS,
    "Prints a message"
  },
  {NULL, NULL}
};

void MyClassBase_subclass_dealloc(PyObject* self) {
  std::cout << "in MyClassBase_subclass_dealloc" << std::endl;
  // Delete the `shared_pointer<MyClass>` properly before freeing
  // `MyClassBase`'s memory, so that the `MyClass::~MyClass` will be called now
  // if this was the last pointer
  // TODO: Does this really work? I won't really know until I try to either create
  // another `MyClassBase` that points to the same `MyClass` or, better yet, when I
  // change `MyClassRef` to hold a `shared_ptr<MyClass>` instead of a `MyClassBase*`.
  ((MyClassBase*)self)->cdata.reset();

  MyClassBase_clear((MyClassBase*)self);
  Py_TYPE(self)->tp_free((PyObject*) self);
}

int MyClassMeta_init(PyObject* cls, PyObject* args, PyObject* kwargs) {
  ((PyTypeObject*)cls)->tp_dealloc = (destructor)MyClassBase_subclass_dealloc;
  return 0;
}

struct MyClassMeta {
  PyHeapTypeObject base;
};

static PyTypeObject MyClassMetaType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  .tp_name = "_MyClassMeta",
  .tp_basicsize = sizeof(MyClassMeta),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_init = MyClassMeta_init,
};

static PyTypeObject MyClassBaseType = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  .tp_name = "_MyClassBase",
  .tp_basicsize = sizeof(MyClassBase),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_methods = MyClassBase_methods,
  .tp_new = MyClassBase_new,
};

static PyObject* MyClassBase_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  if (type == &MyClassBaseType) {
    PyErr_SetString(
      PyExc_RuntimeError,
      "_MyClassBase cannot be instantiated directly. Please use a subclass");
    return NULL;
  }
  MyClassBase* self;
  self = (MyClassBase*) type->tp_alloc(type, 0);
  if (self) {
    self->cdata = std::make_shared<MyClass>();
    self->cdata->set_pyobject((PyObject*) self);
  }

  return (PyObject*) self;
}

static PyObject* MyClassBaseClass = nullptr;

bool MyClassBase_Check(PyObject* obj) {
  if (!MyClassBaseClass) {
    auto myclass_module = PyImport_ImportModule("mylib._myclass");
    if (!myclass_module) {
      PyErr_SetString(
        PyExc_RuntimeError,
        "Cannot import 'mylib._myclass' for some reason");
      return false;
    }
    MyClassBaseClass = PyObject_GetAttrString(myclass_module, "MyClass");
    if (!MyClassBaseClass) {
      PyErr_SetString(
        PyExc_RuntimeError,
        "Cannot find 'mylib._myclass.MyClass' for some reason");
      return false;
    }
  }

  const auto result = PyObject_IsInstance(obj, MyClassBaseClass);
  if (result == -1) {
    PyErr_SetString(
      PyExc_RuntimeError,
      "Cannot check against 'mylib._myclass.MyClass' for some reason");
    return false;
  }

  return result;
}

bool MyClassBase_init_module(PyObject* module) {

  MyClassMetaType.tp_base = &PyType_Type;
  if (PyType_Ready(&MyClassMetaType) < 0) {
    return false;
  }
  Py_INCREF(&MyClassMetaType);
  if (PyModule_AddObject(module, "_MyClassMeta", (PyObject*)&MyClassMetaType) < 0) {
    Py_DECREF(&MyClassMetaType);
    return false;
  }

  if (PyType_Ready(&MyClassBaseType) < 0) {
    return false;
  }
  Py_INCREF(&MyClassBaseType);
  if (PyModule_AddObject(module, "_MyClassBase", (PyObject*) &MyClassBaseType) < 0) {
    Py_DECREF(&MyClassBaseType);
    return false;
  }

  return true;
}
