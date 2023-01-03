import mylib
import gc

print('start')

print('\ncreate')
obj = mylib.MyClass()

print('\ncreate ref')
ref = mylib.MyClassRef(obj)


print('\ncall method')
ref.get().print_message()

print('\ndelete ref')
del ref

print('\ndelete ')
del obj

print('\ncollect')
gc.collect()

print('\ndone')
