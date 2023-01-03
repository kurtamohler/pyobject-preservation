import mylib
import gc

print('create')
a = mylib.MyClass()

print('\ncall method')
a.print_message()

print('\ndeallocate')
del a
gc.collect()

print('\ndone')
