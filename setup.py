#from distutils.core import setup, Extension
from setuptools import setup, Extension

with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

mylib_cpython = Extension(
    '_mylib',
    sources=[
        'mylib/csrc/Module.cpp',
        'mylib/csrc/MyClassBase.cpp',
    ],
    include_dirs=['mylib_cpp'],
    libraries=['mylib_cpp'])

mylib_cpp = ['mylib_cpp', dict(
    sources=[
        'mylib_cpp/MyClass.cpp',
    ],
    include_dirs=['mylib_cpp'],
)]

setup(
    name='pyobject-preservation',
    version='0.0',
    description='PyObject preservation and resurrection in CPython',
    long_description=readme,
    author='Kurt Mohler',
    author_email='kurtamohler@gmail.com',
    url='https://github.com/kurtamohler/pyobject-preservation',
    license=license,
    packages=['mylib'],
    ext_modules=[mylib_cpython],
    libraries=[mylib_cpp],
)
