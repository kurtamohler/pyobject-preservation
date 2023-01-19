#include <iostream>

#include "mylib.h"

#include "MyClassBase.h"
#include "PyObjectPreservation.h"

static PyObject* MyClassBase_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);

static PyObject* MyClassBase_print_message(MyClassBase* self, PyObject* Py_UNUSED(ignored)) {
  self->cdata->print_message();
  return Py_None;
}

static PyObject* MyClassBase_id(MyClassBase* self, PyObject* Py_UNUSED(ignored)) {
  double id = self->cdata->id();

  PyObject* id_obj = PyLong_FromDouble(id);

  return id_obj;
}

static PyMethodDef MyClassBase_methods[] = {
  {
    "print_message",
    (PyCFunction) MyClassBase_print_message,
    METH_NOARGS,
    "Prints a message"
  },
  {
    "id",
    (PyCFunction) MyClassBase_id,
    METH_NOARGS,
    "Returns the unique ID of the underlying `mylib::MyClass` instance"
  },
  {NULL, NULL}
};

int MyClassMeta_init(PyObject* cls, PyObject* args, PyObject* kwargs) {
  ((PyTypeObject*)cls)->tp_dealloc = (destructor)pyobj_preservation::dealloc_or_preserve<MyClassBase, mylib_cpp::MyClass>;
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
  PyVarObject_HEAD_INIT(&MyClassMetaType, 0)
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

  PyObject* self = type->tp_alloc(type, 0);
  pyobj_preservation::init_pyobj<MyClassBase, mylib_cpp::MyClass>(self);
  return self;
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

PyObject* MyClassBase_get_from_cdata(mylib_cpp::intrusive_ptr<mylib_cpp::MyClass> cdata) {
  return pyobj_preservation::get_pyobj_from_cdata<MyClassBase>(cdata);
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
