#from distutils.core import setup, Extension
from setuptools import setup, Extension

with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

py_ext = Extension(
    '_my_lib',
    sources=['src/my_lib_py.cpp'],
    libraries=['my_lib_cpp'])

#c_ext = Extension()

modules = [py_ext]


setup(
    name='pyobject-preservation',
    version='0.0',
    description='PyObject preservation and resurrection in CPython',
    long_description=readme,
    author='Kurt Mohler',
    author_email='kurtamohler@gmail.com',
    url='https://github.com/kurtamohler/pyobject-preservation',
    license=license,
    ext_modules=modules,
    libraries=[['my_lib_cpp', {
        'sources': [
            'src/cpp/my_lib.cpp',
        ],
        'include_dirs': ['src/cpp'],
        }
    ]]
)
