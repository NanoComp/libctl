from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import os
from sysconfig import get_paths

python_include = get_paths()["include"]


class CustomBuildExtCommand(build_ext):
    """Customized build_ext command"""

    def build_extensions(self):
        # Customize compiler settings
        if sys.platform == "darwin":
            for ext in self.extensions:
                ext.extra_link_args = ["-Wl,-rpath,/usr/local/lib"]
        build_ext.build_extensions(self)


libctlgeom_module = Extension(
    "_libctlgeom",
    sources=["libctlgeom_wrap.c"],
    include_dirs=["/usr/local/include", python_include],
    library_dirs=["/usr/local/lib"],
    libraries=["ctlgeom"],
    extra_compile_args=["-I/usr/local/include"],
)

setup(
    name="libctlgeom",
    version="0.1",
    author="SWIG Wrapper Generator",
    description="SWIG wrapper for libctlgeom",
    ext_modules=[libctlgeom_module],
    py_modules=["libctlgeom"],
    cmdclass={"build_ext": CustomBuildExtCommand},
)
