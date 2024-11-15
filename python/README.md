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

## Usage Examples

### Basic Shapes

```python
from ctlgeom import Sphere, Cylinder, Block, Cone, Wedge, Ellipsoid, Prism, SlantedPrism

# Create a sphere
sphere = Sphere(
    material="metal",
    center=(0, 0, 0),
    radius=1.0
)

# Create a cylinder
cylinder = Cylinder(
    material=1 + 1j,
    center=(0, 0, 0),
    radius=1.0,
    height=2.0,
    axis=(0, 0, 1)
)

# Create a block (rectangular prism)
block = Block(
    material={"foo" : 42},
    center=(0, 0, 0),
    e1=(1, 0, 0),    # x-axis
    e2=(0, 1, 0),    # y-axis
    e3=(0, 0, 1),    # z-axis
    size=(2.0, 3.0, 4.0)
)

# Create a cone
cone = Cone(
    material="glass",
    center=(0, 0, 0),
    radius=1.0,      # base radius
    height=2.0,
    axis=(0, 0, 1),
    radius2=0.5      # top radius (0 for perfect cone)
)

# Create a wedge (partial cylinder)
wedge = Wedge(
    material="plastic",
    center=(0, 0, 0),
    radius=1.0,
    height=2.0,
    axis=(0, 0, 1),
    wedge_angle=1.57,  # 90 degrees in radians
    wedge_start=(1, 0, 0)
)

# Create an ellipsoid
ellipsoid = Ellipsoid(
    material="glass",
    center=(0, 0, 0),
    e1=(1, 0, 0),
    e2=(0, 1, 0),
    e3=(0, 0, 1),
    size=(2.0, 3.0, 4.0)  # semi-axes lengths
)

# Create a prism from vertices
vertices = [
    (1.0, 0.0, 0.0),
    (0.0, 1.0, 0.0),
    (-1.0, 0.0, 0.0)
]
prism = Prism(
    material="metal",
    vertices=vertices,
    height=2.0,
    axis=(0, 0, 1),
    center=(0, 0, 0)  # optional
)

# Create a slanted prism
slanted_prism = SlantedPrism(
    material="metal",
    vertices=vertices,
    height=2.0,
    axis=(0, 0, 1),
    sidewall_angle=0.523,  # 30 degrees in radians
    center=(0, 0, 0)  # optional
)
```

### Working with Shapes

```python
from ctlgeom import point_is_in_object, ObjectGroup

# Check if a point is inside an object
point = (0.5, 0.5, 0.5)
is_inside = point_is_in_object(point, sphere)

# Get object volume
volume = sphere.volume()

# Get object bounding box
low, high = sphere.bounding_box()

# Group multiple objects
group = ObjectGroup([sphere, cylinder, block])

# Query material at a point
material = group.material_at_point(point)

# Query the material at a numpy array of points. (Must faster than python loops!)
material = group.material_at_numpy_points(point)

# Query the material at a cartesian grid
material = group.material_on_grid(xx, yy, zz)

# Display debug information
sphere.debug_info()
```

### Vectorized Operations

```python
import numpy as np
import matplotlib.pyplot as plt
import time 

sphere1 = Sphere(material=1+3j, center=(0, -0.5, 0), radius=0.5)
sphere2 = Sphere(material=2+2j, center=(0, 0.5, 0), radius=1)
block = Block(material=3+1j, center=(0.5, 0.5, 0), e1=(1, 1, 0), e2=(1, -1, 0), e3=(0, 0, 1), size=(1, 1, 1))
group = ObjectGroup([sphere2, sphere1, block])

# Create a grid of points
x = np.linspace(-1, 1, 500)
y = np.linspace(-1, 1, 500)
xx, yy = np.meshgrid(x, y)
zz = np.zeros_like(xx)

start_time = time.time()
material_map = group.material_on_grid(xx, yy, zz)
end_time = time.time()
print(f"Time taken: {end_time - start_time} seconds")

plt.figure(figsize=(12, 5))
plt.subplot(1, 2, 1)
plt.imshow(material_map.real, cmap='viridis', extent=[-5, 5, -5, 5])
plt.colorbar(label='Material')
plt.title('Real')
plt.subplot(1, 2, 2)
plt.imshow(material_map.imag, cmap='viridis', extent=[-5, 5, -5, 5])
plt.colorbar(label='Material')
plt.title('Imaginary')
plt.show()
```

### Running tests
First, make sure pytest is installed. You can install it by `pip install pytest`.
```bash
pytest test_ctlgeom.py -v
```
