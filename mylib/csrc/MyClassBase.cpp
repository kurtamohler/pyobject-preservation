#include <iostream>

#include "mylib.h"

#include "MyClassBase.h"
#include "PyObjectPreservation.h"

static PyObject* MyClassBase_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);

static int MyClassBase_clear(MyClassBase* self) {
  MYLIB_ASSERT(false, "MyClassBase_clear not implemented yet");
}

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

// In order to preserve the PyObject when the Python refcount goes to zero, we
// have to use a dealloc function `dealloc_or_preserve` that has the ability to
// cancel the deallocation if there are any other live references to the
// `mylib_cpp::MyClass` object.
//
// In order to properly cancel the deallocation, we must use a metaclass. There
// are two points that explain why:
//
//  * In Python, `mylib.MyClass` is a subclass of `MyClassBase`
//
//  * When using the default metaclass, the dealloc function of the subclass
//    is called before the dealloc function of the base class
//
// So if we used the default metaclass, in cases where the PyObject will be
// preserved, the subclass `mylib.MyClass` would first get deallocated, and
// then the `dealloc_or_preserve` function of the base class `MyClassBase`
// gets called. This would leave us with a messed up half preserved / half
// deallocated PyObject. Therefore, we use a metaclass that calls
// `dealloc_or_preserve` before the subclass is deallocated.

int MyClassMeta_init(PyObject* cls, PyObject* args, PyObject* kwargs) {
  ((PyTypeObject*)cls)->tp_dealloc = (destructor)pyobj_preservation::dealloc_or_preserve<MyClassBase, mylib_cpp::MyClass>;
  return 0;
}

struct MyClassMeta {
  PyHeapTypeObject base;
};

static PyTypeObject MyClassMetaType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  .tp_name = "MyClassMeta",
  .tp_basicsize = sizeof(MyClassMeta),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_init = MyClassMeta_init,
};

static PyTypeObject MyClassBaseType = {
  PyVarObject_HEAD_INIT(&MyClassMetaType, 0)
  .tp_name = "MyClassBase",
  .tp_basicsize = sizeof(MyClassBase),
  .tp_dealloc = nullptr,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,

  // TODO: Do we actually need this?
  //.tp_clear = (inquiry)MyClassBase_clear,

  .tp_methods = MyClassBase_methods,
  .tp_new = MyClassBase_new,
};

template <>
PyTypeObject* pyobj_preservation::get_pytype(MyClassBase*) {
  return &MyClassBaseType;
}

static PyObject* MyClassBase_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
  if (type == &MyClassBaseType) {
    PyErr_SetString(
      PyExc_RuntimeError,
      "MyClassBase cannot be instantiated directly. Please use a subclass");
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
  if (PyModule_AddObject(module, "MyClassMeta", (PyObject*)&MyClassMetaType) < 0) {
    Py_DECREF(&MyClassMetaType);
    return false;
  }

  if (PyType_Ready(&MyClassBaseType) < 0) {
    return false;
  }
  Py_INCREF(&MyClassBaseType);
  if (PyModule_AddObject(module, "MyClassBase", (PyObject*) &MyClassBaseType) < 0) {
    Py_DECREF(&MyClassBaseType);
    return false;
  }

  return true;
}
