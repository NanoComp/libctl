import libctlgeom as geom
from abc import ABC, abstractmethod
from typing import Tuple, List, Literal, Any
import dataclasses
import numpy as np
import copy

MATERIAL_TYPE = Any


def make_vector3(x, y, z) -> geom.vector3:
    """Create a vector3 object.

    Args:
        x: The x-coordinate.
        y: The y-coordinate.
        z: The z-coordinate.

    Returns:
        A vector3 object.
    """
    v = geom.vector3()
    v.x = x
    v.y = y
    v.z = z
    return v


@dataclasses.dataclass(frozen=True)
class BoundingBox:
    """A bounding box.

    Attributes:
        low: The lower corner of the box.
        high: The upper corner of the box.
    """

    low: Tuple[float, float, float]
    high: Tuple[float, float, float]

    def __post_init__(self):
        self.box = geom.geom_box()
        self.box.low = make_vector3(*self.low)
        self.box.high = make_vector3(*self.high)

    def to_geom_object(self) -> geom.geom_box:
        """Convert the bounding box to a geometric object."""
        return self.box


class GeometricObject(ABC):
    """Abstract base class for geometric objects."""

    @abstractmethod
    def to_geom_object(self) -> geom.geometric_object:
        raise NotImplementedError

    def volume(self) -> float:
        """Get the volume of the geometric object."""
        return geom.geom_object_volume(self.to_geom_object())

    def debug_info(self) -> None:
        """Display debug information about the geometric object."""
        display_geom_object_info(self)

    def bounding_box(
        self,
    ) -> Tuple[Tuple[float, float, float], Tuple[float, float, float]]:
        """Get the bounding box of the geometric object.

        Returns:
            A tuple of the lower and upper corners of the bounding box.
        """
        box = geom.geom_box()
        geom.geom_get_bounding_box(self.to_geom_object(), box)
        return ((box.low.x, box.low.y, box.low.z), (box.high.x, box.high.y, box.high.z))


@dataclasses.dataclass(frozen=True)
class Cylinder(GeometricObject):
    """A cylinder.

    Attributes:
        material: Material properties.
        center: The center of the cylinder.
        radius: The radius of the cylinder.
        height: The height of the cylinder.
        axis: The axis of the cylinder.
    """

    material: MATERIAL_TYPE
    center: Tuple[float, float, float]
    radius: float
    height: float
    axis: Tuple[float, float, float]

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the cylinder to a geometric object."""
        return geom.make_cylinder(
            self.material,
            make_vector3(*self.center),
            self.radius,
            self.height,
            make_vector3(*self.axis),
        )


@dataclasses.dataclass(frozen=True)
class Wedge(GeometricObject):
    """A wedge (partial cylinder).

    Attributes:
        material: Material properties.
        center: The center of the wedge.
        radius: The radius of the wedge.
        height: The height of the wedge.
        axis: The axis of the wedge.
        wedge_angle: The angle of the wedge in radians.
        wedge_start: The starting direction vector of the wedge.
    """

    material: MATERIAL_TYPE
    center: Tuple[float, float, float]
    radius: float
    height: float
    axis: Tuple[float, float, float]
    wedge_angle: float
    wedge_start: Tuple[float, float, float]

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the wedge to a geometric object."""
        return geom.make_wedge(
            self.material,
            make_vector3(*self.center),
            self.radius,
            self.height,
            make_vector3(*self.axis),
            self.wedge_angle,
            make_vector3(*self.wedge_start),
        )


@dataclasses.dataclass(frozen=True)
class Cone(GeometricObject):
    """A cone.

    Attributes:
        material: Material properties.
        center: The center of the cone.
        radius: The radius at the base.
        height: The height of the cone.
        axis: The axis of the cone.
        radius2: The radius at the top (0 for a perfect cone).
    """

    material: MATERIAL_TYPE
    center: Tuple[float, float, float]
    radius: float
    height: float
    axis: Tuple[float, float, float]
    radius2: float

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the cone to a geometric object."""
        return geom.make_cone(
            self.material,
            make_vector3(*self.center),
            self.radius,
            self.height,
            make_vector3(*self.axis),
            self.radius2,
        )


@dataclasses.dataclass(frozen=True)
class Sphere(GeometricObject):
    material: MATERIAL_TYPE
    center: Tuple[float, float, float]
    radius: float

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the sphere to a geometric object."""
        return geom.make_sphere(self.material, make_vector3(*self.center), self.radius)


@dataclasses.dataclass(frozen=True)
class Block(GeometricObject):
    """A rectangular block.

    Attributes:
        material: Material properties.
        center: The center of the block.
        e1: First basis vector.
        e2: Second basis vector.
        e3: Third basis vector.
        size: Size along each basis vector.
    """

    material: MATERIAL_TYPE
    center: Tuple[float, float, float]
    e1: Tuple[float, float, float]
    e2: Tuple[float, float, float]
    e3: Tuple[float, float, float]
    size: Tuple[float, float, float]

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the block to a geometric object."""
        return geom.make_block(
            self.material,
            make_vector3(*self.center),
            make_vector3(*self.e1),
            make_vector3(*self.e2),
            make_vector3(*self.e3),
            make_vector3(*self.size),
        )


@dataclasses.dataclass(frozen=True)
class Ellipsoid(GeometricObject):
    """An ellipsoid.

    Attributes:
        material: Material properties.
        center: The center of the ellipsoid.
        e1: First basis vector.
        e2: Second basis vector.
        e3: Third basis vector.
        size: Size along each basis vector.
    """

    material: MATERIAL_TYPE
    center: Tuple[float, float, float]
    e1: Tuple[float, float, float]
    e2: Tuple[float, float, float]
    e3: Tuple[float, float, float]
    size: Tuple[float, float, float]

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the ellipsoid to a geometric object."""
        return geom.make_ellipsoid(
            self.material,
            make_vector3(*self.center),
            make_vector3(*self.e1),
            make_vector3(*self.e2),
            make_vector3(*self.e3),
            make_vector3(*self.size),
        )


@dataclasses.dataclass(frozen=True)
class Prism(GeometricObject):
    """A prism defined by vertices.

    Attributes:
        material: Material properties.
        vertices: List of (x,y,z) vertices defining the prism base.
        height: The height of the prism.
        axis: The axis along which the prism extends.
        center: Optional center point. If None, computed automatically from vertices.
    """

    material: MATERIAL_TYPE
    vertices: List[Tuple[float, float, float]]
    height: float
    axis: Tuple[float, float, float]
    center: Tuple[float, float, float] = None

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the prism to a geometric object."""
        # Convert vertices list to array of vector3
        vertex_array = [make_vector3(*v) for v in self.vertices]

        if self.center is None:
            return geom.make_prism(
                self.material,
                vertex_array,
                len(vertex_array),
                self.height,
                make_vector3(*self.axis),
            )
        else:
            return geom.make_prism_with_center(
                self.material,
                make_vector3(*self.center),
                vertex_array,
                len(vertex_array),
                self.height,
                make_vector3(*self.axis),
            )


@dataclasses.dataclass(frozen=True)
class SlantedPrism(GeometricObject):
    """A prism with slanted sides.

    Attributes:
        material: Material properties.
        vertices: List of (x,y,z) vertices defining the prism base.
        height: The height of the prism.
        axis: The axis along which the prism extends.
        sidewall_angle: The angle of the sidewalls in radians.
        center: Optional center point. If None, computed automatically from vertices.
    """

    material: MATERIAL_TYPE
    vertices: List[Tuple[float, float, float]]
    height: float
    axis: Tuple[float, float, float]
    sidewall_angle: float
    center: Tuple[float, float, float] = None

    def to_geom_object(self) -> geom.geometric_object:
        """Convert the slanted prism to a geometric object."""
        raise NotImplementedError
        # This is not working yet
        # Convert vertices list to array of vector3
        vertex_array = [make_vector3(*v) for v in self.vertices]

        if self.center is None:
            return geom.make_slanted_prism(
                self.material,
                vertex_array,
                len(vertex_array),
                self.height,
                make_vector3(*self.axis),
                self.sidewall_angle,
            )
        else:
            return geom.make_slanted_prism_with_center(
                self.material,
                make_vector3(*self.center),
                vertex_array,
                len(vertex_array),
                self.height,
                make_vector3(*self.axis),
                self.sidewall_angle,
            )


@dataclasses.dataclass
class ObjectGroup:
    """A group of geometric objects."""

    objects: List[GeometricObject]

    def __post_init__(self):
        geom_objects = [obj.to_geom_object() for obj in self.objects]
        self.geom_list = copy.copy(make_geom_object_list(geom_objects))
        self.bounding_box = make_geom_box(
            (float("-inf"), float("-inf"), float("-inf")),
            (float("inf"), float("inf"), float("inf")),
        )
        self.geom_box_tree = self._create_tree_object()

    def _create_tree_object(self) -> Literal["geom.geom_box_tree"]:
        """Create a tree object from the group of geometric objects."""
        return geom.create_geom_box_tree0(self.geom_list, self.bounding_box)

    def material_at(self, point: Tuple[float, float, float]) -> MATERIAL_TYPE:
        """Query the material at a point.

        Args:
            point: The point to query.
        """
        return geom.material_of_point_in_tree(make_vector3(*point), self.geom_box_tree)

    def __del__(self):
        # Clean up the tree when the object is destroyed
        geom.destroy_geom_box_tree(self.geom_box_tree)


def point_is_in_object(
    point: Tuple[float, float, float], object: GeometricObject
) -> bool:
    """Check if a point is inside a geometric object.

    Args:
        point: The point to check.
        object: The geometric object to check against.
    """
    return bool(geom.point_in_objectp(make_vector3(*point), object.to_geom_object()))


def display_geom_object_info(object: GeometricObject):
    """Display information about the geometric object.

    Args:
        object: The geometric object to display information about.
    """
    geom.display_geometric_object_info(0, object.to_geom_object())


def make_geom_object_list(
    objects: list[geom.geometric_object],
) -> geom.geometric_object_list:
    """Create a list of geometric objects.

    Args:
        objects: The geometric objects to create a list of.
    """
    objects_list = geom.geometric_object_list()
    objects_list.num_items = len(objects)
    objects_list.set_items(objects)  # Use set_items instead of direct assignment
    return objects_list


def make_geom_box(
    low: Tuple[float, float, float], high: Tuple[float, float, float]
) -> geom.geom_box:
    """Create a geometric box.

    Args:
        low: The lower corner of the box.
        high: The upper corner of the box.
    """
    box = geom.geom_box()
    box.low = make_vector3(*low)
    box.high = make_vector3(*high)
    return box
