# pyobject-preservation
Preserving and resurrecting PyObjects in
[CPython](https://septatrix.github.io/cpython-dark-docs/extending/index.html#extending-index)

This repo shows an example implementation of a design pattern called "PyObject
preservation", which can be used when writing libraries that offer both
a Python and a C/C++ interface into the same underlying objects using CPython.

The example Python library is called `mylib` and the pure-C++ library is called
`mylib_cpp`. The Python library offers a class called `mylib.MyClass`, and the
C++ library offers a class called `mylib::MyClass`. Both classes have all the
same public methods and features to give consistency between the Python and C++
APIs.

`mylib.MyClass` is implemented with CPython, and it owns an instance of the
pure-C++ `mylib_cpp::MyClass`. Calling a public method of a `mylib.MyClass`
object will call into the same public method of the underlying
`mylib_cpp::MyClass` object. The implementations of methods are written in C++,
which can often give much better performance than writing them in Python. Also,
this lets us have live references in both C++ and Python to the same internal
state of a `MyClass`.

This could also be done in C. Instead of using a C++ class that has methods,
you could use a C struct and offer functions that take pointers to the C struct
as their first argument.

There's a problem that needs to be solved for the case when the reference count
of the Python object goes to zero while the underlying C/C++ object still has
a nonzero reference count. PyObject preservation is a solution to this problem.
In this case, the pure-C/C++ object takes ownership of the PyObject
([documentation for
PyObject](https://docs.python.org/3/c-api/structures.html#c.PyObject)) and the
PyObject's reference count is kept nonzero so that it is preserved, not
deallocated. There are no live references to the PyObject from the Python
context, so we can call it a "zombie" PyObject.

If the object ever needs to be translated back into the Python context, the
ownership is flipped back again so that the PyObject owns the C/C++ object and
the zombie PyObject is "resurrected". Or if the C/C++ object is deallocated
while it still owns the zombie PyObject, its destructor will decrement the
PyObject's reference count so that it is garbage collected properly.

See below for a more detailed explanation of when you may want to use this
pattern and how it can be implemented.


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

## Purpose

Why would you want PyObject preservation anyway?

The alternative to PyObject preservation is to just allow the PyObject to be
deallocated when its reference count goes to zero. If the C/C++ object is still
alive and if we ever need a Python reference to it again, we can create a whole
new PyObject that has an owning reference to the C/C++ object. This basically
works, and it might be good enough in some cases, but there are a few issues
with it.

### Property preservation

Let's say we have the Python class `MyClass`, which contains an owning
reference to an underlying pure-C++ object, as described earlier. We can also
have another Python class called `MyClassRef` which retains a reference to the
C++ object. The method `MyClassRef.get()` will return a `MyClass` object that
owns the C++ object.

```python
import gc
a = MyClass()
ref = MyClassRef(a)
del a
gc.collect()
b = ref.get()
```

In the above code example, when we call `del a`, the Python reference count to
for the `MyClass` object will go to zero, since `ref` only has a reference to
the C++ object. If `MyClass` does not have PyObject preservation, the garbage
collector will deallocate the `MyClass` instance. Then when `ref.get()` is
called, a new `MyClass` instance will be created for `b` to point to. But if
`MyClass` does have PyObject preservation, the C++ object will keep the
PyObject alive the whole time so that `b` points to the same exact instance of
`MyClass` that `a` pointed to.

Let's now modify this code example to store a property on the
`a.__dict__`, like so:

```python
import gc
a = MyClass()
a.__dict__['my_property'] = 'this is a property'
ref = MyClassRef(a)
del a
gc.collect()
b = ref.get()

# This only works if PyObject was preserved:
b.my_property
```

If `MyClass` does not have PyObject preservation, then `b.my_property` will
fail. Since the instance of `MyClass` that had `my_property` was deallocated,
there is no way for the `ref.get()` call to restore `my_property`. But if it
does have PyObject preservation, its properties are preserved and
`b.my_property` works just fine.

### Subclass preservation

PyObject preservation also preserves subclass information. For instance:

```python
import gc
class MySubclass(MyClass):
  pass
a = MySublass()
ref = MyClassRef(a)
del a
gc.collect()
b = ref.get()

# Fails if PyObject was not preserved
assert isinstance(b, MySubclass)
```

In the above example, if the PyObject was preserved, `b` will be an instance of
`MySubclass`. But if not, the subclass information will be lost, and `b` will
just be a `MyClass` instance, since `ref.get()` doesn't know about
`MySubclass`.

### Weak references

Another reason to have PyObject preservation is to properly support [weak
references](https://docs.python.org/3/library/weakref.html). If we have a weak
reference to a `MyClass` and all the strong Python references to the `MyClass`
have been deleted, the weakref should still point to the original `MyClass`
instance as long as the underlying C++ object is alive. If we ever need a new
strong reference, say if `ref.get()` is called, we wouldn't want this to create
a new `MyClass` instance, because the weakref would not be pointing to this new
instance.

## Implementation

TODO

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


