import mylib
import gc

print('create')
a = mylib.MyClass()

print('\ncreate ref')
r = mylib.MyClassRef(a)

print('\ndelete obj')
del a
gc.collect()

print('\nget another obj')
a = r.get()

print('\ndel again')
del a
gc.collect()

print('\nget another obj')
a = r.get()

print('\nget second obj')
b = r.get()

print('\ndelete first one')
del a
gc.collect()

print('\ndelete second  one')
del b
gc.collect()

print('\ncreate second ref')
r2 = mylib.MyClassRef(r.get())
gc.collect()

print('\ndelete first ref')
del r
gc.collect()

print('\ndelete second ref')
del r2
gc.collect()

print('\ndone')
