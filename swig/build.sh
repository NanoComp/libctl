#!/bin/bash

# Define local include paths relative to script location
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOCAL_INCLUDES="-I$SCRIPT_DIR/../src -I$SCRIPT_DIR/../utils"
echo $LOCAL_INCLUDES

# Check if running in a conda environment
if [ -n "${CONDA_PREFIX}" ]; then
    echo "Using Conda environment at: $CONDA_PREFIX"
    PYTHON_INCLUDE=$($CONDA_PREFIX/bin/python3 -c "from sysconfig import get_paths; print(get_paths()['include'])")
    export CFLAGS="-I$SCRIPT_DIR/../src -I$SCRIPT_DIR/../utils -I$PYTHON_INCLUDE"
    swig -python $LOCAL_INCLUDES -I"$PYTHON_INCLUDE" libctlgeom.i
    $CONDA_PREFIX/bin/python3 setup.py build_ext --inplace
    $CONDA_PREFIX/bin/pip install .
else
    echo "Using system Python"
    PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths; print(get_paths()['include'])")
    export CFLAGS="-I$SCRIPT_DIR/../src -I$SCRIPT_DIR/../utils -I$PYTHON_INCLUDE"
    swig -python $LOCAL_INCLUDES -I"$PYTHON_INCLUDE" libctlgeom.i
    python3 setup.py build_ext --inplace
    pip install .
fi