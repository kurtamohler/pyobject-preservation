from distutils.core import setup, Extension

with open('README.md') as f:
    readme = f.read()

with open('LICENSE') as f:
    license = f.read()

module = Extension('_preservation',
                   sources = ['preservation.cpp'])

setup(
    name='pyobject-preservation',
    version='0.0',
    description='PyObject preservation and resurrection in CPython',
    long_description=readme,
    author='Kurt Mohler',
    author_email='kurtamohler@gmail.com',
    url='https://github.com/kurtamohler/pyobject-preservation',
    license=license,
    ext_modules=[module],
)
