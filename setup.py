from setuptools import setup

setup(
    name='projection_mapping',
    version='1',
    py_modules=['projection_mapping'],
    install_requires=['cffi'],
    setup_requires=['cffi'],
    cffi_modules=['build_pm.py:ffibuilder']
)