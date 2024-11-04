#!/bin/bash

# Check if running in a conda environment
if [ -n "${CONDA_PREFIX}" ]; then
    echo "Using Conda environment at: $CONDA_PREFIX"
    PYTHON_INCLUDE=$($CONDA_PREFIX/bin/python3 -c "from sysconfig import get_paths; print(get_paths()['include'])")
    swig -python -I/usr/local/include -I"$PYTHON_INCLUDE" libctlgeom.i
    $CONDA_PREFIX/bin/python3 setup.py build_ext --inplace
    $CONDA_PREFIX/bin/pip install .
else
    echo "Using system Python"
    PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths; print(get_paths()['include'])")
    swig -python -I/usr/local/include -I"$PYTHON_INCLUDE" libctlgeom.i
    python3 setup.py build_ext --inplace
    pip install .
fi