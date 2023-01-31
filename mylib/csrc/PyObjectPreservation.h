#pragma once

#include "mylib.h"
#include "PyInterpreterDefs.h"

#include <iostream>

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

template <typename BaseT, typename CppT>
void dealloc_or_preserve(PyObject* self) {
  std::cout << "in PyObjectPreservation_dealloc" << std::endl;

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
      std::cout << "Deleting PyObject!!!!" << std::endl;
      base->cdata.reset();

      Py_TYPE(self)->tp_free((PyObject*) self);
    }
  } else {
    std::cout << "PyObject refcount is nonzero, keeping alive" << std::endl;
  }
}

template<typename BaseT, typename CppT>
PyObject* get_pyobj_from_cdata(mylib_cpp::intrusive_ptr<CppT> cdata) {
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
