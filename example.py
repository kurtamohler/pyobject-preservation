import mylib
import gc

print('start')
obj = mylib.MyClass()
ref = mylib.MyClassRef(obj)
ref.get().print_message()
del obj
del ref
gc.collect()
print('done')
