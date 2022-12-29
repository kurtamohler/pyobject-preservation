#from distutils.core import setup, Extension
from setuptools import setup, Extension

with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

my_lib_py_extension = Extension(
    '_my_lib',
    sources=[
        'my_lib_py/my_lib_py.cpp'
    ],
    include_dirs=['my_lib_cpp'],
    libraries=['my_lib_cpp'])

ext_modules = [my_lib_py_extension]

my_lib_cpp = ['my_lib_cpp', dict(
    sources=[
        'my_lib_cpp/my_lib_cpp.cpp',
    ],
    include_dirs=['my_lib_cpp'],
)]

libraries = [my_lib_cpp]

setup(
    name='pyobject-preservation',
    version='0.0',
    description='PyObject preservation and resurrection in CPython',
    long_description=readme,
    author='Kurt Mohler',
    author_email='kurtamohler@gmail.com',
    url='https://github.com/kurtamohler/pyobject-preservation',
    license=license,
    ext_modules=ext_modules,
    libraries=libraries,
)
