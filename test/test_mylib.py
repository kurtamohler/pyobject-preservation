import mylib
import unittest
import gc

class MyClassTest(unittest.TestCase):
    def test_preserved_attributes(self):
        a = mylib.MyClass()
        property_val = 'My property'
        a.__dict__['my_property'] = property_val

        r = mylib.MyClassRef(a)

        del a
        gc.collect()

        b = r.get()

        del r
        gc.collect()

        self.assertTrue(hasattr(b, 'my_property'))
        self.assertEqual(b.my_property, property_val)

    def test_preserved_subclass(self):
        class MySubclass(mylib.MyClass):
            pass

        a = MySubclass()
        r = mylib.MyClassRef(a)

        del a
        gc.collect()

        b = r.get()

        del r
        gc.collect()

        self.assertTrue(isinstance(b, MySubclass))


if __name__ == '__main__':
    unittest.main()
