import glob
# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
from setuptools_scm import get_version

__version__ = get_version()

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.

ext_modules = [
    Pybind11Extension("g722_1_mod",
        ["src/main.cpp"],
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)],
        include_dirs = ['src/common', 'src/au'],
        ),
]

setup(
    name="g722_1_mod",
    version=__version__,
    url="https://github.com/adafruit/g722_1_mod",
    description="A modified G722.1 encoder",
    long_description="This modified format is used by certain embedded designs.",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)
