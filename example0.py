import mylib
import gc

print('start')

print('\ncreate')
obj = mylib.MyClass()

print('\ncreate ref')
ref = mylib.MyClassRef(obj)

print('\ndelete from Python')
del obj

print('\ncall method')
ref.get().print_message()

print('\ndelete from C++')
del ref

print('\ncollect')
gc.collect()

print('\ndone')
