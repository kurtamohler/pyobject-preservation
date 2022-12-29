# pyobject-preservation
Preserving and resurrecting PyObjects in CPython

This repo shows an example implementation of a pattern called "PyObject
preservation", which can be used when writing libraries that offer both
a Python and a C/C++ interface into the same underlying objects.

The basic idea of PyObject preservation is that if the Python object's
reference count goes to zero while the corresponding C/C++ object still has
a nonzero reference count, the C/C++ object takes ownership of the PyObject so
that it is preserved, not deallocated. If the object ever needs to be
translated back into the Python context, the ownership is flipped again and the
same exact Python object is resurrected.

A full explanation of the pattern can be found below.

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
python setup.py build_clib
```
```shell
python setup.py install
```

## Run

Run the example script:

```shell
python example.py
```

## Explanation

When writing [C/C++ extensions for
Python](https://docs.python.org/3.12/extending/extending.html), it is common to
offer both a Python class and a C/C++ class (or struct) that have similar APIs.
Doing so means that your library will provide a consistent interface in Python
and C/C++. It is also useful for the Python and C/C++ objects to have shared
access to the same internal state of objects.

Let's say we want to implement this for some class `MyClass` in our own
library. The setup will go something like this:

  * Create a `MyClass` struct or class in C/C++ that has all the methods
    we want.

  * Define the Python class in C/C++ code using the CPython interface, rather
    than defining the class in actual Python code. We'll make use of the
    `PyTypeObject` and `PyObject` classes that CPython provides. We create
    a `PyTypeObject` that specifies all the properties and methods that we want
    `MyClass` to have in our Python library.

  * Set up the `PyTypeObject` such that a Python `MyClass` object will contain
    an owning reference to a C/C++ `MyClass` object. For each method that we want
    `MyClass` to have, we'll add one to the `PyTypeObject` that just calls into
    the corresponding method of the C/C++ `MyClass` object.

Now any time we instantiate `MyClass` from Python, a `PyObject` will be created
for it in C/C++, the `PyObject` will also contain an instance of the C/C++
`MyClass`, and calling a method on the Python `MyClass` object will call into
the underlying C/C++ `MyClass` object's corresponding method.

There's an issue with this though. At runtime, if all the references to the
Python `MyClass` object are deleted, the Python object's reference count goes
to zero and its finalizer gets called. Typically, a finalizer will just
deallocate the `PyObject`. But the problem is that we could still have live
references to the C/C++ `MyClass` object somewhere, and we may need to return
a `PyObject` for it again at some point.

One possible solution is to go through with deallocating just the Python
`MyClass` object and leave the C/C++ `MyClass` object alive. If we ever need to
make another reference to the object in the Python context later on, we can
just reinstantiate a new `PyObject` for it, give it an owning reference to the
C/C++ `MyClass` object, and return it to the Python context.  However, since
this creates an entirely new Python `MyClass` object, this solution is not
ideal in some cases.

For one thing, if any properties were ever added to the `__dict__` of the
original instance of the Python `MyClass`, they won't be restored for the new
instance.

Another issue is that if you want `MyClass` to support [weak
references](https://docs.python.org/3/library/weakref.html) in Python, you
can't do it properly with this set up. Let's say there is a weak reference to
a `MyClass` object at the time when its strong reference count goes to zero.
The object will be deallocated, and the weak reference is now invalid.
Furthermore, if the object is ever reinstantiated on the Python side, the weak
reference will still be invalid--it won't point to the new instance.

The solution is to use PyObject preservation. Any time the strong reference
count to a Python `MyClass` object goes to zero, we flip the ownership so that
the C/C++ `MyClass` object has an owning reference to the `PyObject`. This
avoids deallocating the Python object. If the reference count to the C/C++
`MyClass` object goes to zero, then we'll deallocate both the C/C++ and the
Python `MyClass` objects at the same time. Or if we ever need to create another
strong reference object in the Python context, we can resurrect the PyObject.
Ownership will be flipped back again and the new Python reference will point to
the same exact instance of the Python `MyClass` that we had originally. Weak
references will remain valid and the `__dict__` on the Python object will be
preserved.

One tricky part of this is that if someone wants to use only the C/C++ API from
your library, then they should not have to depend on CPython at all. So the C/C++
`MyClass` definition should not `#include <Python.h>`.
