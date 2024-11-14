# Python Extension for libctl

This directory contains SWIG bindings to create a Python extension for libctl.

## Prerequisites
- SWIG
- Python 3.x
- pip
- A C compiler (gcc, clang, etc.)

## Installation

**Using Conda Environment**
   ```bash
   conda activate my_env
   ./build.sh
   ```

**Using System Python**
   ```bash
   ./build.sh
   ```

The build script will automatically:
- Generate Python bindings using SWIG
- Compile the extension module
- Install the package using pip

## Verification

To verify the installation, you can run Python and try the following:
```python
import ctlgeom
sphere = ctlgeom.Sphere(1, (0, 0.5, 0), 1)
ctlgeom.point_is_in_object((0.5, 0, 0), sphere)
```

