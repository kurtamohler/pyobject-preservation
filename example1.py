import mylib
import gc

print('start')

print('\ncreate')
obj = mylib.MyClass()

print('\ncreate ref')
ref = mylib.MyClassRef(obj)


print('\ngetting obj from ref')
obj2 = ref.get()

print('\ndelete ref')
del ref

print('\ncall method on first obj')
obj.print_message()

print('\ndelete first obj')
del obj

print('\ncall method on second obj')
obj2.print_message()

print('\ndelete second obj')
del obj2

print('\ncollect')
gc.collect()

print('\ndone')
