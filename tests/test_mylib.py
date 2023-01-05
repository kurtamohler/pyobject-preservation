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

        a = r.get()

        del r
        gc.collect()

        self.assertTrue(hasattr(a, 'my_property'))
        self.assertEqual(a.my_property, property_val)

if __name__ == '__main__':
    unittest.main()
