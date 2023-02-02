# pyobject-preservation
Preserving and resurrecting PyObjects in
[CPython](https://septatrix.github.io/cpython-dark-docs/extending/index.html#extending-index)

This repo shows an example implementation of a design pattern called "PyObject
preservation", which can be used when writing libraries that offer both
a Python and a C/C++ interface into the same underlying objects using CPython.

## Build

To build this project, you must have
[Miniconda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html)
installed.

Then run the following:

```shell
conda env create -f environment.yaml -n pyobject-preservation
```
```shell
conda activate pyobject-preservation
```
```shell
python setup.py install
```

## Run

Run the tests:

```shell
python test/test_mylib.py
```

## What is PyObject preservation?

Let's say we want to make a Python library that is implemented in C++. We want
a user-facing C++ API that is consistent with the Python API, offering all the
same methods, classes, and features. Users should be able to build a pure-C++
application, a pure-Python application, or choose to write different parts in
Python or C++. We'll need to be able to pass objects back and forth between
Python and C++ contexts. So any class in our Python API will contain
a reference to the underlying C++ class. Calling a method on the Python class
will end up calling the corresponding method in the C++ implementation.

In a library like this, how should we handle the deallocations for such a pair
of Python and C++ objects? Specifically, what should we do if the Python
object's reference count goes to zero buy the C++ object needs to stay alive
because there are other references to it in the C++ context?

A good answer is to use "PyObject preservation".
[`PyObject`](https://docs.python.org/3/c-api/structures.html#c.PyObject) is the
C struct that CPython uses to represent all Python objects. We can implement
a deallocation function for our Python class that will cancel the deallocation
if the C++ object needs to stay alive. Then the Python object becomes
a "zombie PyObject", which can either be "resurrected" if we ever need to pass
the object back to the Python context, or it can be deallocated if the C++
object's refcount reaches zero.

## Example library `mylib`

In this repo, we have a Python library `mylib` and a pure-C++ library
`mylib_cpp`. The Python class `mylib.MyClass` contains an owning reference to
an underlying pure-C++ object `mylib_cpp::MyClass`.

We also have another Python class called `mylib.MyClassRef` which only exists
to allow us to exercise PyObject preservation. `mylib.MyClassRef` just retains
a reference to a `mylib_cpp::MyClass`. The method `mylib.MyClassRef.get()`
returns the `mylib.MyClass` object that corresponds with the underlying
`mylib_cpp::MyClass`. If the `mylib.MyClass` is in the zombie state, it will be
resurrected.

```python
import mylib
import gc

a = mylib.MyClass()
ref = mylib.MyClassRef(a)

# The PyObject becomes a zombie
del a
gc.collect()

# The zombie PyObject is resurrected
b = ref.get()
```

## Why not let the PyObject die?

In the previous code example, when we call `del a`, the Python reference count
for the `mylib.MyClass` object goes to zero, since `ref` only has a reference
to the C++ object. If `mylib.MyClass` did not have PyObject preservation, the
garbage collector would deallocate the `mylib.MyClass` instance. Then when
`ref.get()` is called, a new `mylib.MyClass` instance would be created for `b`
to point to. But since `mylib.MyClass` does have PyObject preservation, the
PyObject is kept alive the whole time so that `b` points to the same exact
instance of `mylib.MyClass` that `a` used to point to.

An alternative to PyObject preservation is to just allow the PyObject to be
deallocated when its reference count goes to zero, even if the C++ object is
still alive. If we ever need to pass the C++ object back up to Python, we can
create a whole new PyObject that has an owning reference to the C++ object.
Depending on how you expect the user to use your class, this might be good
enough in some cases, but there are a few issues with it.

When we deallocate the PyObject we will lose information about its pure-Python
properties and subclasses, and weak references won't work properly. These
issues are described below.

### Property preservation

We can add properties on a class's `__dict__`, which will be preserved if the
PyObject is preserved.

```python
import mylib
import gc

a = mylib.MyClass()
a.__dict__['my_property'] = 'this is a property'
ref = mylib.MyClassRef(a)
del a
gc.collect()
b = ref.get()

# This only works if PyObject was preserved:
b.my_property
```

In the above example, if `mylib.MyClass` did not have PyObject preservation,
then `b.my_property` would fail. Since the instance of `mylib.MyClass` that
held `my_property` would be deallocated, there would be no way for the
`ref.get()` call to restore `my_property`. But since it does have PyObject
preservation, its properties are preserved and `b.my_property` works just fine.

### Subclass preservation

PyObject preservation also preserves subclass information.

```python
import mylib
import gc

class MySubclass(mylib.MyClass):
  pass

a = MySublass()
ref = mylib.MyClassRef(a)
del a
gc.collect()
b = ref.get()

# Fails if PyObject was not preserved
assert isinstance(b, MySubclass)
```

In the above example, since the PyObject is preserved, `b` will be an instance
of `MySubclass`. But if it was not preserved, the subclass information would be
lost, and `b` would just be a `mylib.MyClass` instance, since `ref.get()` would
not know that it should be restored as a `MySubclass` instance.

### Weak references

Another reason to use PyObject preservation is to properly support [weak
references](https://docs.python.org/3/library/weakref.html). If we have a weak
reference to a `mylib.MyClass` and all the strong Python references to it have
been deleted, the weakref should still point to the original `mylib.MyClass`
instance as long as the underlying C++ object is alive. If we ever need a new
strong reference, say if `ref.get()` is called, we wouldn't want this to create
a new `mylib.MyClass` instance, because any weakrefs we had would not be
updated to point to this new instance, so they would become invalid.

## Implementation

### Project architecture

In this repo, `mylib` is a pure-Python library, starting in
[`mylib/__init__.py`](mylib/__init__.py). `mylib` calls into `_mylib`, which is
a Python library implemented in C++ with CPython in
[`mylib/csrc/Module.cpp`](mylib/csrc/Module.cpp) and the other files in
[`mylib/csrc`](mylib/csrc). `_mylib` calls into the classes and methods of
`mylib_cpp`, a pure-C++ library defined in [`mylib_cpp/`](mylib_cpp/).

`mylib.MyClass`, defined in [`mylib/_myclass.py`](mylib/_myclass.py), is just
a subclass of `_mylib._MyClassBase`, which is defined in
[`mylib/csrc/MyClassBase.h`](mylib/csrc/MyClassBase.h) and
[`mylib/csrc/MyClassBase.cpp`](mylib/csrc/MyClassBase.cpp).
`_mylib._MyClassBase` contains an `intrusive_ptr` that points to the underlying
pure-C++ `mylib_cpp::MyClass`, which is defined in
[`mylib_cpp/MyClass.h`](mylib_cpp/MyClass.h).

`intrusive_ptr` is a kind of smart pointer borrowed from
[PyTorch](https://github.com/pytorch/pytorch) which stores a reference count
within the object that it's pointing to, rather than storing it separately like
`std::shared_ptr` does.

TODO: Finish writing this

### PyTorch example

Let's look at a real example of PyObject preservation. The idea of PyObject
preservation originated in [PyTorch](https://github.com/pytorch/pytorch), which
has a Python class called `torch.Tensor` and a C++ class called
`torch::Tensor`. For the most part, they both have all the same public methods
and properties. Instances of `torch.Tensor` and `torch::Tensor` can share the
same internal state, which makes it possible to pass a `torch.Tensor` into
a function that is implemented in C++ which will operate on a corresponding
`torch::Tensor`. We can also go the opposite direction--starting with
a `torch::Tensor` that was created in the C++ context, we can propagate it into
the Python context as a `torch.Tensor`.

We won't get into the details of how exactly this is set up in PyTorch, because
these classes have lots of layers of abstraction and other things going on. But
at the bottom of all those layers, both `torch.Tensor` and `torch::Tensor` have
a reference to a `c10::TensorImpl` object which is implemented in C++. When
there are live references to a `torch.Tensor` in Python, its PyObject has an
owning reference to the `c10::TensorImpl`. If the Python references ever go to
zero but there are still references to the `c10::TensorImpl` in C++, then the
Python reference count to the PyObject for `torch.Tensor` will be incremented
once to keep it alive, and the `c10::TensorImpl` will take an owning reference
to the PyObject.

If the tensor ever needs to be returned to the Python context, the PyObject is
resurrected and ownership is flipped so that the `torch.Tensor` has an owning
reference to the `c10::TensorImpl` again. Or if the the reference count to
`c10::TensorImpl` ever goes to zero, its destructor will take care of
decrementing the PyObject's reference count so that it will be deallocated.

However, decrementing the reference count of a PyObject from
`c10::TensorImpl`'s destructor is a bit tricky. We can't just `#include
<Python.h>` and call `Py_DECREF(PyObject*)`. PyTorch supports building C++
applications that don't depend on Python, so `c10::TensorImpl` is defined in
a pure C++ context that cannot directly depend on the CPython library. Put very
simply, PyTorch solves this problem by using a function pointer that either
points to a decref function if the Python context exists or the function
pointer is null if the `c10::TensorImpl` is pure C++.

If you're interested in learning more about PyObject preservation and related
topics in PyTorch, here are some relevant PyTorch Developer Podcast episodes:

  * [PyObject preservation](https://pytorch-dev-podcast.simplecast.com/episodes/pyobject-preservation)

  * [Reference counting](https://pytorch-dev-podcast.simplecast.com/episodes/reference-counting)

  * [Weak references](https://pytorch-dev-podcast.simplecast.com/episodes/weak-references)


