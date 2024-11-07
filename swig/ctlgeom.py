import libctlgeom as geom
from abc import ABC, abstractmethod
from typing import Tuple, List, Literal
import dataclasses
import numpy as np


def make_vector3(x, y, z):
    v = geom.vector3()
    v.x = x
    v.y = y
    v.z = z
    return v


@dataclasses.dataclass(frozen=True)
class BoundingBox:
    low: Tuple[float, float, float]
    high: Tuple[float, float, float]

    def __post_init__(self):
        self.box = geom.geom_box()
        self.box.low = make_vector3(*self.low)
        self.box.high = make_vector3(*self.high)

    def to_geom_object(self):
        return self.box


@dataclasses.dataclass(frozen=True)
class Material:
    """Material properties."""

    epsilon: float


class GeometricObject(ABC):
    """Abstract base class for geometric objects."""

    @abstractmethod
    def to_geom_object(self):
        raise NotImplementedError

    def get_volume(self) -> float:
        return geom.geom_object_volume(self.to_geom_object())

    def debug_info(self) -> None:
        display_geom_object_info(self)

    def get_bounding_box(self) -> Tuple[geom.vector3, geom.vector3]:
        box = geom.geom_box()
        geom.geom_get_bounding_box(self.to_geom_object(), box)
        return (box.low, box.high)


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

    material: Material
    center: Tuple[float, float, float]
    radius: float
    height: float
    axis: Tuple[float, float, float]

    def to_geom_object(self):
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

    material: Material
    center: Tuple[float, float, float]
    radius: float
    height: float
    axis: Tuple[float, float, float]
    wedge_angle: float
    wedge_start: Tuple[float, float, float]

    def to_geom_object(self):
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

    material: Material
    center: Tuple[float, float, float]
    radius: float
    height: float
    axis: Tuple[float, float, float]
    radius2: float

    def to_geom_object(self):
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
    material: Material
    center: Tuple[float, float, float]
    radius: float

    def to_geom_object(self):
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

    material: Material
    center: Tuple[float, float, float]
    e1: Tuple[float, float, float]
    e2: Tuple[float, float, float]
    e3: Tuple[float, float, float]
    size: Tuple[float, float, float]

    def to_geom_object(self):
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

    material: Material
    center: Tuple[float, float, float]
    e1: Tuple[float, float, float]
    e2: Tuple[float, float, float]
    e3: Tuple[float, float, float]
    size: Tuple[float, float, float]

    def to_geom_object(self):
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

    material: Material
    vertices: List[Tuple[float, float, float]]
    height: float
    axis: Tuple[float, float, float]
    center: Tuple[float, float, float] = None

    def to_geom_object(self):
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

    material: Material
    vertices: List[Tuple[float, float, float]]
    height: float
    axis: Tuple[float, float, float]
    sidewall_angle: float
    center: Tuple[float, float, float] = None

    def to_geom_object(self):
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


@dataclasses.dataclass(frozen=False)
class GroupObject:
    """A group of geometric objects."""

    objects: List[GeometricObject]

    def __post_init__(self):
        self.objects_map = {
            obj.to_geom_object(): obj for obj in self.objects
        }  # Add this line
        geom_objects = [obj.to_geom_object() for obj in self.objects]
        self.geom_list = make_geom_object_list(geom_objects)
        self.bounding_box = make_geom_box(
            (float("-inf"), float("-inf"), float("-inf")),
            (float("inf"), float("inf"), float("inf")),
        )
        self.geom_box_tree = None

    def to_tree_object(self) -> Literal["geom.geom_box_tree"]:
        """Create a tree object from the group of geometric objects."""
        return geom.create_geom_box_tree0(self.geom_list, self.bounding_box)

    def get_tree_object(self) -> Literal["geom.geom_box_tree"]:
        """Get the tree object from the group of geometric objects."""
        if self.geom_box_tree is None:
            self.geom_box_tree = self.to_tree_object()
        return self.geom_box_tree

    def query_object_at_point(
        self, point: Tuple[float, float, float]
    ) -> GeometricObject:
        """Query the object at a point."""
        shiftby = geom.vector3()
        return geom.object_of_point0(self.geom_list, make_vector3(*point), shiftby)

    def query_object_at_point_in_tree(
        self, point: Tuple[float, float, float], precedence: int
    ) -> GeometricObject:
        """Unstable. Not outputting object as expected."""
        shiftby = geom.vector3()
        precedence_ptr = geom.new_intp()
        geom.intp_assign(precedence_ptr, precedence)
        return geom.object_of_point_in_tree(
            make_vector3(*point), self.get_tree_object(), shiftby, precedence_ptr
        )

    def query_material_at_point(self, point: Tuple[float, float, float]) -> Material:
        """Query the material at a point.

        Args:
            point: The point to query.
        """
        tree = self.get_tree_object()
        return geom.material_of_point_in_tree(make_vector3(*point), tree)

    def __del__(self):
        # Clean up the tree when the object is destroyed
        if hasattr(self, "tree"):
            geom.destroy_geom_box_tree(self.tree)


def point_is_in_object(
    point: Tuple[float, float, float], object: GeometricObject
) -> bool:
    """Check if a point is inside a geometric object.

    Args:
        point: The point to check.
        object: The geometric object to check against.
    """
    return bool(geom.point_in_objectp(make_vector3(*point), object.to_geom_object()))


def display_geom_object_info(object: GeometricObject) -> None:
    """Display information about the geometric object.

    Args:
        object: The geometric object to display information about.
    """
    geom.display_geometric_object_info(0, object.to_geom_object())


def make_geom_object_list(objects: list[geom.geometric_object]):
    objects_list = geom.geometric_object_list()
    objects_list.num_items = len(objects)
    objects_list.set_items(objects)  # Use set_items instead of direct assignment
    return objects_list


def make_geom_box(low: Tuple[float, float, float], high: Tuple[float, float, float]):
    """Create a geometric box.

    Args:
        low: The lower corner of the box.
        high: The upper corner of the box.
    """
    box = geom.geom_box()
    box.low = make_vector3(*low)
    box.high = make_vector3(*high)
    return box
