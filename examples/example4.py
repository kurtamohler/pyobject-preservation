import mylib
import gc

a = mylib.MyClass()
a.__dict__['my_property'] = "This is a property"

r = mylib.MyClassRef(a)
del a
gc.collect()

a = r.get()

del r
gc.collect()

assert a.my_property == "This is a property"
