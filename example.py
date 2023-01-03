import mylib
import gc

print('start')
obj = mylib.MyClass()
ref = mylib.MyClassRef(obj)
del obj
ref.get().print_message()
del ref
gc.collect()
print('done')
