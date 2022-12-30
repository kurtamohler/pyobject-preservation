import mylib
import gc

print('start')
obj = mylib.MyClass()
obj.print_message()
del obj
gc.collect()
print('done')
