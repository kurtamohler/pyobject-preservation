import mylib
import unittest

class Tracker:
    def __init__(self, marker):
        self.marker = marker

    @staticmethod
    def make():
        marker = [False]
        return marker, Tracker(marker)

    def __del__(self):
        self.marker[0] = True

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

            ids.append(id_)

    def test_MyClassRef(self):
        a = mylib.MyClass()
        id_check = a.id()


        refs = [mylib.MyClassRef(a) for _ in range(10)]

        for r in refs:
            self.assertTrue(r.get() is a)
            self.assertEqual(r.get().id(), id_check)

        del a

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

        b = r.get()

        del r

        self.assertTrue(hasattr(b, 'my_property'))
        self.assertEqual(b.my_property, property_val)
        self.assertEqual(b.id(), id_check)

    def test_preserved_subclass(self):
        class MySubclass(mylib.MyClass):
            finalized_count = 0

            def __del__(self):
                MySubclass.finalized_count += 1

        m, t = Tracker.make()
        a = MySubclass()
        a.__dict__['tmp'] = t
        del t
        r = mylib.MyClassRef(a)
        id_check = a.id()

        del a

        self.assertFalse(m[0])

        b = r.get()

        del r

        self.assertFalse(m[0])
        self.assertTrue(isinstance(b, MySubclass))
        self.assertEqual(b.id(), id_check)

        self.assertEqual(MySubclass.finalized_count, 0)

        del b

        self.assertTrue(m[0])

        # Make sure that the Python defined finalizer was called
        self.assertEqual(MySubclass.finalized_count, 1)

    def test_tracker(self):
        m, t = Tracker.make()
        self.assertFalse(m[0])
        del t
        self.assertTrue(m[0])

    def test_dealloc(self):
        m, t = Tracker.make()
        a = mylib.MyClass()
        a.__dict__['tmp'] = t
        del t
        self.assertFalse(m[0])
        del a
        self.assertTrue(m[0])

    def test_dealloc_zombie(self):
        m, t = Tracker.make()
        a = mylib.MyClass()
        a.__dict__['tmp'] = t
        del t
        self.assertFalse(m[0])
        r = mylib.MyClassRef(a)
        del a
        self.assertFalse(m[0])
        del r
        self.assertTrue(m[0])

    def test_dealloc_resurrected(self):
        m, t = Tracker.make()
        a = mylib.MyClass()
        a.__dict__['tmp'] = t
        del t
        self.assertFalse(m[0])
        r = mylib.MyClassRef(a)
        del a
        self.assertFalse(m[0])
        b = r.get()
        del r
        self.assertFalse(m[0])
        del b
        self.assertTrue(m[0])
    
    def test_weakref(self):
        import weakref

        a = mylib.MyClass()
        id_check = a.id()

        r_weak = weakref.ref(a)
        r = mylib.MyClassRef(a)
        del a

        b = r_weak()
        b.id()

if __name__ == '__main__':
    unittest.main()
