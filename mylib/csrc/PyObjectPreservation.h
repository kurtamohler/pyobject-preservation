#pragma once

#include <iostream>
#include <Python.h>

#include "mylib.h"
#include "PyInterpreterDefs.h"

namespace pyobj_preservation {

template <typename BaseT, typename CppT>
void init_pyobj(PyObject* self) {
  BaseT* base = (BaseT*) self;

  if (base) {
    base->cdata = mylib_cpp::make_intrusive<CppT>();
    base->cdata->pyobj_slot()->set_pyobj(self);
    base->cdata->pyobj_slot()->set_pyobj_interpreter(&pyobj_interpreter);
  }
}

// A template specialization of this function needs to be definined for the BaseT
// TODO: Do I like doing it this way? Is there another way?
template <typename BaseT>
PyTypeObject* get_pytype(BaseT*);

static void _clear_slots(PyTypeObject* type, PyObject* self) {
  Py_ssize_t n = Py_SIZE(type);
  PyMemberDef* mp = type->tp_members;

  // TODO: For some reason this won't compile, with
  // `error: invalid use of incomplete type 'PyMemberDef'`

  //for (Py_ssize_t i = 0; i < n; i++) {
  //  if (mp[i].type == T_OBJECT_EX && !(mp[i].flags & READONLY)) {
  //    char* addr = (char*)self + mp[i].offset;
  //    PyObject* obj = *(PyObject**)addr;
  //    if (obj != nullptr) {
  //      *(PyObject**)addr = nullptr;
  //      Py_DECREF(obj);
  //    }
  //  }
  //}
}

template <typename BaseT, typename CppT>
void _dealloc(PyObject* self) {
  std::cout << "in _dealloc" << std::endl;
  BaseT* base = (BaseT*) self;

  PyTypeObject* type = Py_TYPE(self);
  MYLIB_ASSERT(type->tp_flags & Py_TPFLAGS_HEAPTYPE, "Must be on heap");
  MYLIB_ASSERT(PyType_IS_GC(type), "GC types not implemented");

  PyObject_GC_UnTrack(self);

  bool has_finalizer = type->tp_finalize || type->tp_del;

  if (type->tp_finalize) {
    PyObject_GC_Track(self);
    if (PyObject_CallFinalizerFromDealloc(self) < 0) {
      // Resurrected
      return;
    }
    PyObject_GC_UnTrack(self);
  }

  if (type->tp_del) {
    PyObject_GC_Track(self);
    type->tp_del(self);
    if (self->ob_refcnt > 0) {
      // Resurrected
      return;
    }
    PyObject_GC_UnTrack(self);
  }

  std::cout << "Deleting PyObject!!!! (" << Py_TYPE(self)->tp_name << ")" << std::endl;

  PyTypeObject* base_type = get_pytype<BaseT>((BaseT*)nullptr);

  // Clear the members of all subclasses until we reach the base python type
  {
    PyTypeObject* cur_type = type;
    while (cur_type != base_type) {
      if (Py_SIZE(cur_type)) {
        _clear_slots(cur_type, self);
      }
      cur_type = cur_type->tp_base;
      MYLIB_ASSERT(cur_type, "");
    }
  }

  if (type->tp_dictoffset) {
    PyObject** dictptr = _PyObject_GetDictPtr(self);
    if (dictptr != nullptr) {
      PyObject* dict = *dictptr;
      if (dict != nullptr) {
        Py_DECREF(dict);
        *dictptr = nullptr;
      }
    }
  }

  MYLIB_ASSERT(Py_TYPE(self) == type, "");

  if (base_type->tp_clear != nullptr) {
    base_type->tp_clear(self);
  }
  base->cdata.reset();
  Py_TYPE(self)->tp_free(self);

  MYLIB_ASSERT(
    type->tp_flags & Py_TPFLAGS_HEAPTYPE,
    "Python subclass was not on the heap");
  Py_DECREF(type);
}

template <typename BaseT, typename CppT>
void dealloc_or_preserve(PyObject* self) {
  std::cout << "in dealloc_or_preserve" << std::endl;

  BaseT* base = (BaseT*) self;

  if (Py_REFCNT(self) == 0) {
    // If there are still references to CppT when the PyObject is
    // being deallocated, then we have to swap ownership of the BaseT to
    // CppT to preserve it.
    //
    // TODO: This implicitly assumes that no two different BaseT instances
    // can ever point to the same CppT, but I should probably add support
    // for that, which will break this logic.
    if (base->cdata.use_count() > 1) {
      std::cout << "Preserving PyObject!!!!" << std::endl;
      CppT* myobj = base->cdata.get();

      myobj->pyobj_slot()->set_owns_pyobj(true);

      // Need to incref the PyObject to keep it in the zombie state
      Py_INCREF(self);

      base->cdata.reset();

    } else {
      _dealloc<BaseT, CppT>(self);
    }
  } else {
    std::cout << "PyObject refcount is nonzero, keeping alive" << std::endl;
  }
}

template<typename BaseT, typename CppT>
PyObject* get_pyobj_from_cdata(mylib_cpp::intrusive_ptr<CppT> cdata) {

  // TODO: This should be a valid case, for when a `CppT` instance is created
  // in pure-C++ context and then returned up to Python context.
  MYLIB_ASSERT(
    cdata->pyobj_slot()->pyobj(),
    "get_pyobj_from_cdata received cdata with a null PyObject");

  PyObject* pyobj = cdata->pyobj_slot()->pyobj();

  // If cdata owns the PyObject, we need to resurrect it
  if (cdata->pyobj_slot()->owns_pyobj()) {
    // The PyObject refcount should remain 1 the whole time it's a zombie. If it's
    // not, then something went wrong.
    MYLIB_ASSERT(
      Py_REFCNT(pyobj) == 1,
      (
        "For some reason, we're trying to resurrect a PyObject whose refcount "
        "is not equal to 1"
      ));

    std::cout << "Resurrecting PyObject!!!!" << std::endl;

    // The CppT won't have an owning ref any more, but we'll have
    // a new ref in Python when this function returns. So the Py_REFCNT should
    // not be changed in this case
    BaseT* base = (BaseT*) pyobj;
    base->cdata = cdata;

    cdata->pyobj_slot()->set_owns_pyobj(false);

  } else {
    Py_INCREF(pyobj);
  }

  return pyobj;
}

}
