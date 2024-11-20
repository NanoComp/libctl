import numpy as np
import pytest

from ctlgeom import (Block, BoundingBox, Cone, Cylinder, Ellipsoid,
                     ObjectGroup, Prism, SlantedPrism, Sphere, Wedge,
                     point_is_in_object)


class TestBoundingBox:
    """Tests for BoundingBox functionality."""

    def test_creation(self):
        """Test creating a BoundingBox object."""
        low = (-1.0, -2.0, -3.0)
        high = (1.0, 2.0, 3.0)
        box = BoundingBox(low=low, high=high)

        assert box.low == low
        assert box.high == high

        geom_box = box.to_geom_object()
        assert (geom_box.low.x, geom_box.low.y, geom_box.low.z) == low
        assert (geom_box.high.x, geom_box.high.y, geom_box.high.z) == high

    def test_invalid_box(self):
        """Test that creating an invalid bounding box (low > high) still works."""
        low = (1.0, 1.0, 1.0)
        high = (-1.0, -1.0, -1.0)
        box = BoundingBox(low=low, high=high)

        # The box should still be created, even though it's "invalid"
        assert box.low == low
        assert box.high == high


class TestSphere:
    """Tests for Sphere functionality."""

    def test_creation(self):
        """Test creating a Sphere object."""
        material = "test_material"
        center = (0.0, 0.0, 0.0)
        radius = 1.0
        sphere = Sphere(material=material, center=center, radius=radius)

        assert sphere.material == material
        assert sphere.center == center
        assert sphere.radius == radius

    def test_volume(self):
        """Test calculating the volume of a sphere."""
        sphere = Sphere(material="test", center=(0, 0, 0), radius=1.0)
        expected_volume = 4 / 3 * np.pi
        assert abs(sphere.volume() - expected_volume) < 1e-10

    def test_bounding_box(self):
        """Test getting the bounding box of a sphere."""
        center = (1.0, 2.0, 3.0)
        radius = 2.0
        sphere = Sphere(material="test", center=center, radius=radius)

        low, high = sphere.bounding_box()

        # Check that the bounding box extends radius units in each direction from center
        assert all(l == c - radius for l, c in zip(low, center))
        assert all(h == c + radius for h, c in zip(high, center))

    def test_point_in_sphere(self):
        """Test checking if points are inside or outside a sphere."""
        sphere = Sphere(material="test", center=(0, 0, 0), radius=1.0)

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), sphere)
        assert point_is_in_object((0.5, 0, 0), sphere)
        assert point_is_in_object((0, 0.5, 0), sphere)
        assert point_is_in_object((0, 0, 0.5), sphere)

        # Test points that should be outside
        assert not point_is_in_object((1.5, 0, 0), sphere)
        assert not point_is_in_object((0, 1.5, 0), sphere)
        assert not point_is_in_object((0, 0, 1.5), sphere)
        assert not point_is_in_object((1, 1, 1), sphere)

    def test_negative_radius(self):
        """Test creating a sphere with negative radius."""
        sphere = Sphere(material="test", center=(0, 0, 0), radius=-1.0)

        # volume is negative for negative radius
        assert sphere.volume() < 0


class TestCylinder:
    """Tests for Cylinder functionality."""

    def test_creation(self):
        """Test creating a Cylinder object."""
        material = "test_material"
        center = (0.0, 0.0, 0.0)
        radius = 1.0
        height = 2.0
        axis = (0.0, 0.0, 1.0)
        cylinder = Cylinder(
            material=material, center=center, radius=radius, height=height, axis=axis
        )

        assert cylinder.material == material
        assert cylinder.center == center
        assert cylinder.radius == radius
        assert cylinder.height == height
        assert cylinder.axis == axis

    def test_volume(self):
        """Test calculating the volume of a cylinder."""
        cylinder = Cylinder(
            material="test", center=(0, 0, 0), radius=1.0, height=2.0, axis=(0, 0, 1)
        )
        expected_volume = np.pi * 1.0**2 * 2.0
        assert abs(cylinder.volume() - expected_volume) < 1e-10

    def test_point_in_cylinder(self):
        """Test checking if points are inside or outside a cylinder."""
        cylinder = Cylinder(
            material="test", center=(0, 0, 0), radius=1.0, height=2.0, axis=(0, 0, 1)
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), cylinder)
        assert point_is_in_object((0.5, 0, 0.5), cylinder)
        assert point_is_in_object((0, 0, 0.9), cylinder)

        # Test points that should be outside
        assert not point_is_in_object((0, 0, 1.1), cylinder)
        assert not point_is_in_object((1.5, 0, 0), cylinder)
        assert not point_is_in_object((0, 0, -1.1), cylinder)


class TestWedge:
    """Tests for Wedge functionality."""

    def test_creation(self):
        """Test creating a Wedge object."""
        material = "test_material"
        center = (0.0, 0.0, 0.0)
        radius = 1.0
        height = 2.0
        axis = (0.0, 0.0, 1.0)
        wedge_angle = np.pi / 2
        wedge_start = (1.0, 0.0, 0.0)

        wedge = Wedge(
            material=material,
            center=center,
            radius=radius,
            height=height,
            axis=axis,
            wedge_angle=wedge_angle,
            wedge_start=wedge_start,
        )

        assert wedge.material == material
        assert wedge.center == center
        assert wedge.radius == radius
        assert wedge.height == height
        assert wedge.axis == axis
        assert wedge.wedge_angle == wedge_angle
        assert wedge.wedge_start == wedge_start

    def test_volume(self):
        """Test calculating the volume of a wedge."""
        wedge = Wedge(
            material="test",
            center=(0, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
            wedge_angle=np.pi / 2,
            wedge_start=(1, 0, 0),
        )
        expected_volume = np.pi * 1.0**2 * 2.0 / 4
        assert abs(wedge.volume() - expected_volume) < 1e-10

    def test_point_in_wedge(self):
        """Test checking if points are inside or outside a wedge."""
        wedge = Wedge(
            material="test",
            center=(0, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
            wedge_angle=np.pi / 2,
            wedge_start=(1, 0, 0),
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), wedge)
        assert point_is_in_object((0.5, 0.1, 0.5), wedge)
        assert point_is_in_object((0.1, 0.1, 0.9), wedge)

        # Test points that should be outside
        assert not point_is_in_object((0, 0, 1.1), wedge)
        assert not point_is_in_object((1.5, 0, 0), wedge)
        assert not point_is_in_object((0, 0, -1.1), wedge)
        assert not point_is_in_object((-0.5, 0, 0), wedge)
        assert not point_is_in_object((0, -0.5, 0), wedge)


class TestCone:
    """Tests for Cone functionality."""

    def test_creation(self):
        """Test creating a Cone object."""
        material = "test_material"
        center = (0.0, 0.0, 0.0)
        radius = 1.0
        height = 2.0
        axis = (0.0, 0.0, 1.0)
        radius2 = 0.5

        cone = Cone(
            material=material,
            center=center,
            radius=radius,
            height=height,
            axis=axis,
            radius2=radius2,
        )

        assert cone.material == material
        assert cone.center == center
        assert cone.radius == radius
        assert cone.height == height
        assert cone.axis == axis
        assert cone.radius2 == radius2

    def test_volume(self):
        """Test calculating the volume of a cone."""
        cone = Cone(
            material="test",
            center=(0, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
            radius2=0.5,
        )

        expected_volume = np.pi * 2.0 * (1.0**2 + 0.5**2 + 1.0 * 0.5) / 3
        assert abs(cone.volume() - expected_volume) < 1e-10

        perfect_cone = Cone(
            material="test",
            center=(0, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
            radius2=0.0,
        )

        expected_volume = np.pi * 1.0**2 * 2.0 / 3
        assert abs(perfect_cone.volume() - expected_volume) < 1e-10

    def test_point_in_cone(self):
        """Test checking if points are inside or outside a cone."""
        cone = Cone(
            material="test",
            center=(0, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
            radius2=0.5,
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), cone)
        assert point_is_in_object((0.4, 0, 0.5), cone)
        assert point_is_in_object((0, 0.2, 0.9), cone)

        # Test points that should be outside
        assert not point_is_in_object((0, 0, 1.1), cone)
        assert not point_is_in_object((1.5, 0, 0), cone)
        assert not point_is_in_object((0, 0, -1.1), cone)
        assert not point_is_in_object((0.9, 0, 0.9), cone)


class TestBlock:
    """Tests for Block functionality."""

    def test_creation(self):
        """Test creating a Block object."""
        material = "test_material"
        center = (0.0, 0.0, 0.0)
        e1 = (1.0, 0.0, 0.0)
        e2 = (0.0, 1.0, 0.0)
        e3 = (0.0, 0.0, 1.0)
        size = (2.0, 3.0, 4.0)

        block = Block(material=material, center=center, e1=e1, e2=e2, e3=e3, size=size)

        assert block.material == material
        assert block.center == center
        assert block.e1 == e1
        assert block.e2 == e2
        assert block.e3 == e3
        assert block.size == size

    def test_volume(self):
        """Test calculating the volume of a block."""
        block = Block(
            material="test",
            center=(0, 0, 0),
            e1=(1, 0, 0),
            e2=(0, 1, 0),
            e3=(0, 0, 1),
            size=(2.0, 3.0, 4.0),
        )
        expected_volume = 2.0 * 3.0 * 4.0
        assert abs(block.volume() - expected_volume) < 1e-10

    def test_point_in_block(self):
        """Test checking if points are inside or outside a block."""
        block = Block(
            material="test",
            center=(0, 0, 0),
            e1=(1, 0, 0),
            e2=(0, 1, 0),
            e3=(0, 0, 1),
            size=(2.0, 2.0, 2.0),
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), block)
        assert point_is_in_object((0.9, 0, 0), block)
        assert point_is_in_object((0, 0.9, 0), block)
        assert point_is_in_object((0, 0, 0.9), block)

        # Test points that should be outside
        assert not point_is_in_object((1.1, 0, 0), block)
        assert not point_is_in_object((0, 1.1, 0), block)
        assert not point_is_in_object((0, 0, 1.1), block)
        assert not point_is_in_object((1.1, 1.1, 1.1), block)


class TestEllipsoid:
    """Tests for Ellipsoid functionality."""

    def test_creation(self):
        """Test creating an Ellipsoid object."""
        material = "test_material"
        center = (0.0, 0.0, 0.0)
        e1 = (1.0, 0.0, 0.0)
        e2 = (0.0, 1.0, 0.0)
        e3 = (0.0, 0.0, 1.0)
        size = (2.0, 3.0, 4.0)

        ellipsoid = Ellipsoid(
            material=material, center=center, e1=e1, e2=e2, e3=e3, size=size
        )

        assert ellipsoid.material == material
        assert ellipsoid.center == center
        assert ellipsoid.e1 == e1
        assert ellipsoid.e2 == e2
        assert ellipsoid.e3 == e3
        assert ellipsoid.size == size

    def test_volume(self):
        """Test calculating the volume of an ellipsoid."""
        ellipsoid = Ellipsoid(
            material="test",
            center=(0, 0, 0),
            e1=(1, 0, 0),
            e2=(0, 1, 0),
            e3=(0, 0, 1),
            size=(2.0, 3.0, 4.0),
        )
        expected_volume = 4 / 3 * np.pi * 1.0 * 1.5 * 2.0
        assert abs(ellipsoid.volume() - expected_volume) < 1e-10

    def test_point_in_ellipsoid(self):
        """Test checking if points are inside or outside an ellipsoid."""
        ellipsoid = Ellipsoid(
            material="test",
            center=(0, 0, 0),
            e1=(1, 0, 0),
            e2=(0, 1, 0),
            e3=(0, 0, 1),
            size=(2.0, 2.0, 2.0),
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), ellipsoid)
        assert point_is_in_object((0.9, 0, 0), ellipsoid)
        assert point_is_in_object((0, 0.9, 0), ellipsoid)
        assert point_is_in_object((0, 0, 0.9), ellipsoid)

        # Test points that should be outside
        assert not point_is_in_object((1.1, 0, 0), ellipsoid)
        assert not point_is_in_object((0, 1.1, 0), ellipsoid)
        assert not point_is_in_object((0, 0, 1.1), ellipsoid)
        assert not point_is_in_object((1.1, 1.1, 1.1), ellipsoid)


class TestPrism:
    """Tests for Prism functionality."""

    def test_creation(self):
        """Test creating a Prism object."""
        material = "test_material"
        vertices = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0)]
        height = 2.0
        axis = (0.0, 0.0, 1.0)
        center = (0.0, 0.0, 0.0)

        # Test creation with center
        prism = Prism(
            material=material,
            vertices=vertices,
            height=height,
            axis=axis,
            center=center,
        )

        assert prism.material == material
        assert prism.vertices == vertices
        assert prism.height == height
        assert prism.axis == axis
        assert prism.center == center

        # Test creation without center
        prism_no_center = Prism(
            material=material,
            vertices=vertices,
            height=height,
            axis=axis,
        )

        assert prism_no_center.center is None

    def test_volume(self):
        """Test calculating the volume of a prism."""
        # Create a triangular prism
        vertices = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0)]
        height = 2.0
        prism = Prism(
            material="test",
            vertices=vertices,
            height=height,
            axis=(0, 0, 1),
        )

        expected_volume = 2.0
        assert abs(prism.volume() - expected_volume) < 1e-10

    def test_point_in_prism(self):
        """Test checking if points are inside or outside a prism."""
        vertices = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0)]
        height = 2.0
        prism = Prism(
            material="test",
            vertices=vertices,
            height=height,
            axis=(0, 0, 1),
            center=(0, 0, 0),
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), prism)
        assert point_is_in_object((0, 0.2, 0.5), prism)
        assert point_is_in_object((0.2, 0.2, 0.9), prism)

        # Test points that should be outside
        assert not point_is_in_object((0, 0, 1.1), prism)
        assert not point_is_in_object((1.5, 0, 0), prism)
        assert not point_is_in_object((0, 0, -1.1), prism)
        assert not point_is_in_object((0, -0.5, 0), prism)


class TestSlantedPrism:
    """Tests for SlantedPrism functionality."""

    def test_creation(self):
        """Test creating a SlantedPrism object."""
        material = "test_material"
        vertices = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0)]
        height = 2.0
        axis = (0.0, 0.0, 1.0)
        sidewall_angle = np.pi / 6  # 30 degrees
        center = (0.0, 0.0, 0.0)

        # Test creation with center
        prism = SlantedPrism(
            material=material,
            vertices=vertices,
            height=height,
            axis=axis,
            sidewall_angle=sidewall_angle,
            center=center,
        )

        assert prism.material == material
        assert prism.vertices == vertices
        assert prism.height == height
        assert prism.axis == axis
        assert prism.sidewall_angle == sidewall_angle
        assert prism.center == center

        # Test creation without center
        prism_no_center = SlantedPrism(
            material=material,
            vertices=vertices,
            height=height,
            axis=axis,
            sidewall_angle=sidewall_angle,
        )

        assert prism_no_center.center is None

    def test_volume(self):
        """Test calculating the volume of a slanted prism."""
        # Create a triangular slanted prism
        vertices = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0)]
        height = 2.0
        sidewall_angle = np.pi / 6
        prism = SlantedPrism(
            material="test",
            vertices=vertices,
            height=height,
            axis=(0, 0, 1),
            sidewall_angle=sidewall_angle,
        )

        assert prism.volume() > 0

    def test_point_in_slanted_prism(self):
        """Test checking if points are inside or outside a slanted prism."""
        vertices = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (-1.0, 0.0, 0.0)]
        height = 2.0
        sidewall_angle = np.pi / 6
        prism = SlantedPrism(
            material="test",
            vertices=vertices,
            height=height,
            axis=(0, 0, 1),
            sidewall_angle=sidewall_angle,
            center=(0, 0, 0),
        )

        # Test points that should be inside
        assert point_is_in_object((0, 0, 0), prism)
        assert point_is_in_object((0, 0.2, 0.5), prism)
        assert point_is_in_object((0.2, 0.2, 0.9), prism)

        # Test points that should be outside
        assert not point_is_in_object((0, 0, 1.1), prism)
        assert not point_is_in_object((1.5, 0, 0), prism)
        assert not point_is_in_object((0, 0, -1.1), prism)
        assert not point_is_in_object((0, -0.5, 0), prism)


class TestObjectGroup:
    """Tests for ObjectGroup functionality."""

    def test_creation(self):
        """Test creating an ObjectGroup object."""
        sphere = Sphere(material="test", center=(0, 0, 0), radius=1.0)
        cylinder = Cylinder(
            material="test", center=(2, 0, 0), radius=1.0, height=2.0, axis=(0, 0, 1)
        )

        group = ObjectGroup(objects=[sphere, cylinder])

        assert len(group.objects) == 2
        assert group.objects[0] == sphere
        assert group.objects[1] == cylinder

    def test_material_at_point(self):
        """Test getting material at different points in an ObjectGroup."""
        sphere = Sphere(material="sphere_mat", center=(0, 0, 0), radius=1.0)
        cylinder = Cylinder(
            material="cylinder_mat",
            center=(42, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
        )

        group = ObjectGroup(objects=[sphere, cylinder])

        assert group.material_at_point((0, 0, 0)) == "sphere_mat"
        assert group.material_at_point((42, 0, 0)) == "cylinder_mat"
        assert group.material_at_point((99, 0, 0)) is None

    def test_material_at_numpy_points(self):
        """Test getting materials at multiple points using numpy array."""
        sphere = Sphere(material="sphere_mat", center=(0, 0, 0), radius=1.0)
        cylinder = Cylinder(
            material="cylinder_mat",
            center=(2, 0, 0),
            radius=1.0,
            height=2.0,
            axis=(0, 0, 1),
        )

        group = ObjectGroup(objects=[sphere, cylinder])

        # Create a numpy array of test points
        points = np.array(
            [
                [0, 0, 0],  # Inside sphere
                [2, 0, 0],  # Inside cylinder
                [5, 5, 5],  # Outside both
                [0.5, 0, 0],  # Inside sphere
                [2, 0.5, 0.5],  # Inside cylinder
            ]
        )

        materials = group.material_at_numpy_points(points)

        assert materials[0] == "sphere_mat"
        assert materials[1] == "cylinder_mat"
        assert np.isnan(materials[2])
        assert materials[3] == "sphere_mat"
        assert materials[4] == "cylinder_mat"

    def test_material_at_numpy_points_invalid_input(self):
        """Test material_at_numpy_points with invalid input shapes."""
        group = ObjectGroup(
            objects=[Sphere(material="test", center=(0, 0, 0), radius=1.0)]
        )

        # Test with wrong number of dimensions
        with pytest.raises(ValueError, match="Points must be a 2D numpy array"):
            group.material_at_numpy_points(np.array([1, 2, 3]))

        # Test with wrong second dimension
        with pytest.raises(ValueError, match="Points must have shape"):
            group.material_at_numpy_points(np.array([[1, 2], [3, 4]]))

    def test_material_on_grid(self):
        """Test getting materials on a 3D grid."""
        sphere = Sphere(material=1.0, center=(0, 0, 0), radius=1.0)
        group = ObjectGroup(objects=[sphere])

        # Create a simple 3x3x3 grid
        x = np.linspace(-2, 2, 3)
        y = np.linspace(-2, 2, 3)
        z = np.linspace(-2, 2, 3)
        xx, yy, zz = np.meshgrid(x, y, z)

        materials = group.material_on_grid(xx, yy, zz, default_material=0.0)

        # Check shape
        assert materials.shape == (3, 3, 3)

        # Center point should be inside sphere (material 1.0)
        assert materials[1, 1, 1] == 1.0

        # Corner points should be outside sphere (material 0.0)
        assert materials[0, 0, 0] == 0.0
        assert materials[0, 0, 2] == 0.0
        assert materials[0, 2, 0] == 0.0
        assert materials[2, 0, 0] == 0.0
        assert materials[2, 2, 2] == 0.0

    def test_material_on_grid_invalid_input(self):
        """Test material_on_grid with invalid input shapes."""
        group = ObjectGroup(
            objects=[Sphere(material=1.0, center=(0, 0, 0), radius=1.0)]
        )

        # Create mismatched grid shapes
        x = np.linspace(-2, 2, 3)
        y = np.linspace(-2, 2, 4)
        z = np.linspace(-2, 2, 3)
        xx, yy, zz = np.meshgrid(x, y, z)

        # Test with mismatched shapes
        with pytest.raises(ValueError, match="xx, yy, and zz must have the same shape"):
            group.material_on_grid(xx[:2], yy, zz)
