import mylib
import unittest
import gc

class MyClassTest(unittest.TestCase):
    def test_id(self):
        objects = [mylib.MyClass() for _ in range(10)]

        for a_idx, a in enumerate(objects):
            for b_idx, b in enumerate(objects):
                if a_idx == b_idx:
                    self.assertTrue(b is a)
                    self.assertEqual(b.id(), a.id())
                else:
                    self.assertFalse(b is a)
                    self.assertNotEqual(b.id(), a.id())

        ids = []

        for _ in range(10):
            a = mylib.MyClass()
            id_ = a.id()

            self.assertNotIn(id_, ids)

            del a
            gc.collect()

            ids.append(id_)

    def test_MyClassRef(self):
        a = mylib.MyClass()
        id_check = a.id()


        refs = [mylib.MyClassRef(a) for _ in range(10)]

        for r in refs:
            self.assertTrue(r.get() is a)
            self.assertEqual(r.get().id(), id_check)

        del a
        gc.collect()

        for r in refs:
            self.assertTrue(r.get() is refs[0].get())
            self.assertEqual(r.get().id(), id_check)

    def test_preserved_attributes(self):
        a = mylib.MyClass()
        property_val = 'My property'
        a.__dict__['my_property'] = property_val
        id_check = a.id()

        r = mylib.MyClassRef(a)

        del a
        gc.collect()

        b = r.get()

        del r
        gc.collect()

        self.assertTrue(hasattr(b, 'my_property'))
        self.assertEqual(b.my_property, property_val)
        self.assertEqual(b.id(), id_check)

    def test_preserved_subclass(self):
        class MySubclass(mylib.MyClass):
            pass

        a = MySubclass()
        r = mylib.MyClassRef(a)
        id_check = a.id()

        del a
        gc.collect()

        b = r.get()

        del r
        gc.collect()

        self.assertTrue(isinstance(b, MySubclass))
        self.assertEqual(b.id(), id_check)

if __name__ == '__main__':
    unittest.main()
