%module libctlgeom

/* Include the header files first */
%{
#include "ctlgeom.h"
#include "ctlgeom-types.h"
%}

/* Include the type definitions */
%include "ctlgeom-types.h"

/* Properly wrap vector3 structure */
%include "ctl-math.h"

/* Define MATERIAL_TYPE with proper typemap */
%typemap(in) MATERIAL_TYPE {
    $1 = (void*)$input;
}

%typemap(out) MATERIAL_TYPE {
    $result = (PyObject*)$1;
}

/* Core initialization */
extern void geom_initialize(void);
extern void geom_fix_objects(void);
extern void geom_fix_lattice(void);
extern void geom_cartesian_lattice(void);

extern geometric_object make_sphere(MATERIAL_TYPE material, vector3 center, number radius);
extern geometric_object make_cylinder(MATERIAL_TYPE material, vector3 center, number radius, number height, vector3 axis);
extern geometric_object make_cone(MATERIAL_TYPE material, vector3 center, number radius, number height, vector3 axis, number radius2);
extern geometric_object make_block(MATERIAL_TYPE material, vector3 center, vector3 e1, vector3 e2, vector3 e3, vector3 size);
extern geometric_object make_ellipsoid(MATERIAL_TYPE material, vector3 center, vector3 e1, vector3 e2, vector3 e3, vector3 size);

/* Prism constructors */
extern geometric_object make_prism(MATERIAL_TYPE material, const vector3 *vertices, int num_vertices, double height, vector3 axis);
extern geometric_object make_prism_with_center(MATERIAL_TYPE material, vector3 center, const vector3 *vertices, int num_vertices, double height, vector3 axis);

/* Geometric calculations */
extern double geom_object_volume(geometric_object o);
extern boolean point_in_objectp(vector3 p, geometric_object o);
extern boolean point_in_periodic_objectp(vector3 p, geometric_object o);

/* Material/object lookup */
extern MATERIAL_TYPE material_of_point(vector3 p);
extern geometric_object object_of_point(vector3 p, vector3 *shiftby);
extern MATERIAL_TYPE material_of_point0(GEOMETRIC_OBJECT_LIST geometry, vector3 p);
geometric_object object_of_point0(GEOMETRIC_OBJECT_LIST geometry, vector3 p, vector3 *shiftby);

/* Coordinate transformations */
extern vector3 to_geom_object_coords(vector3 p, geometric_object o);
extern vector3 from_geom_object_coords(vector3 p, geometric_object o);
extern vector3 normal_to_object(vector3 p, geometric_object o);

/* Intersection calculations */
extern int intersect_line_with_object(vector3 p, vector3 d, geometric_object o, double s[2]);

extern void display_geometric_object_info(int indentby, geometric_object o);
