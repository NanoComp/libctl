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

%include "cpointer.i"
%pointer_functions(int, intp);

/* Define MATERIAL_TYPE with proper typemap */
%typemap(in) MATERIAL_TYPE {
    Py_XINCREF($input);  // Increment reference count when passing to C
    $1 = (void*)$input;
}

%typemap(out) MATERIAL_TYPE {
    if ($1 == NULL) {
        Py_INCREF(Py_None);
        $result = Py_None;
    } else {
        $result = (PyObject*)$1;
        Py_XINCREF($result);  // Increment reference count when returning to Python
    }
}

extern void display_geometric_object_info(int indentby, geometric_object o);

extern void geom_initialize(void);
extern void geom_fix_object_ptr(geometric_object *o);
// extern void geom_fix_object(geometric_object o) CTLGEOM_DEPRECATED;
extern void geom_fix_objects(void);
// extern void geom_fix_objects0(geometric_object_list geometry) CTLGEOM_DEPRECATED;
extern void geom_fix_object_list(geometric_object_list geometry);
extern void geom_fix_lattice(void);
extern void geom_fix_lattice0(LATTICE *L);
extern void geom_cartesian_lattice(void);
extern void geom_cartesian_lattice0(LATTICE *L);
extern double geom_object_volume(geometric_object o);
extern boolean point_in_objectp(vector3 p, geometric_object o);
extern boolean point_in_periodic_objectp(vector3 p, geometric_object o);
extern boolean point_in_fixed_objectp(vector3 p, geometric_object o);
extern boolean point_in_fixed_pobjectp(vector3 p, geometric_object *o);
extern boolean point_in_periodic_fixed_objectp(vector3 p, geometric_object o);
extern vector3 to_geom_object_coords(vector3 p, geometric_object o);
extern vector3 from_geom_object_coords(vector3 p, geometric_object o);
extern vector3 normal_to_object(vector3 p, geometric_object o);
extern vector3 normal_to_fixed_object(vector3 p, geometric_object o);
extern int intersect_line_with_object(vector3 p, vector3 d, geometric_object o, double s[2]);
extern double intersect_line_segment_with_object(vector3 p, vector3 d, geometric_object o, double a,
                                                 double b);
extern MATERIAL_TYPE material_of_point_inobject(vector3 p, boolean *inobject);
extern MATERIAL_TYPE material_of_point_inobject0(geometric_object_list geometry, vector3 p,
                                                 boolean *inobject);
extern MATERIAL_TYPE material_of_point(vector3 p);
extern MATERIAL_TYPE material_of_point0(geometric_object_list geometry, vector3 p);
geometric_object object_of_point0(geometric_object_list geometry, vector3 p, vector3 *shiftby);
geometric_object object_of_point(vector3 p, vector3 *shiftby);
vector3 shift_to_unit_cell(vector3 p);
extern matrix3x3 square_basis(matrix3x3 lattice_basis, vector3 size);
extern void ctl_printf(const char *fmt, ...);
extern void (*ctl_printf_callback)(const char *s);

typedef struct {
  vector3 low, high;
} geom_box;

typedef struct {
  geom_box box;
  const geometric_object *o;
  vector3 shiftby;
  int precedence;
} geom_box_object;

typedef struct geom_box_tree_struct {
  geom_box b, b1, b2;
  struct geom_box_tree_struct *t1, *t2;
  int nobjects;
  geom_box_object *objects;
} * geom_box_tree;

extern void destroy_geom_box_tree(geom_box_tree t);
extern geom_box_tree create_geom_box_tree(void);
extern geom_box_tree create_geom_box_tree0(geometric_object_list geometry, geom_box b0);
extern geom_box_tree restrict_geom_box_tree(geom_box_tree, const geom_box *);
extern geom_box_tree geom_tree_search(vector3 p, geom_box_tree t, int *oindex);
extern geom_box_tree geom_tree_search_next(vector3 p, geom_box_tree t, int *oindex);
extern MATERIAL_TYPE material_of_point_in_tree_inobject(vector3 p, geom_box_tree t,
                                                        boolean *inobject);
extern MATERIAL_TYPE material_of_point_in_tree(vector3 p, geom_box_tree t);
extern MATERIAL_TYPE material_of_unshifted_point_in_tree_inobject(vector3 p, geom_box_tree t,
                                                                  boolean *inobject);
const geometric_object *object_of_point_in_tree(vector3 p, geom_box_tree t, vector3 *shiftby,
                                                int *precedence);
extern vector3 to_geom_box_coords(vector3 p, geom_box_object *gbo);
extern void display_geom_box_tree(int indentby, geom_box_tree t);
extern void geom_box_tree_stats(geom_box_tree t, int *depth, int *nobjects);

extern void geom_get_bounding_box(geometric_object o, geom_box *box);
extern number box_overlap_with_object(geom_box b, geometric_object o, number tol, int maxeval);
extern number ellipsoid_overlap_with_object(geom_box b, geometric_object o, number tol,
                                            int maxeval);
extern number range_overlap_with_object(vector3 low, vector3 high, geometric_object o, number tol,
                                        int maxeval);

// extern vector3 get_grid_size(void);
// extern vector3 get_resolution(void);
// extern void get_grid_size_n(int *nx, int *ny, int *nz);

geometric_object make_geometric_object(MATERIAL_TYPE material, vector3 center);
geometric_object make_cylinder(MATERIAL_TYPE material, vector3 center, number radius, number height,
                               vector3 axis);
geometric_object make_wedge(MATERIAL_TYPE material, vector3 center, number radius, number height,
                            vector3 axis, number wedge_angle, vector3 wedge_start);
geometric_object make_cone(MATERIAL_TYPE material, vector3 center, number radius, number height,
                           vector3 axis, number radius2);
geometric_object make_sphere(MATERIAL_TYPE material, vector3 center, number radius);
geometric_object make_block(MATERIAL_TYPE material, vector3 center, vector3 e1, vector3 e2,
                            vector3 e3, vector3 size);
geometric_object make_ellipsoid(MATERIAL_TYPE material, vector3 center, vector3 e1, vector3 e2,
                                vector3 e3, vector3 size);

extern boolean node_in_or_on_polygon(vector3 q0, vector3 *nodes, int num_nodes,
                                     boolean include_boundaries);

// prism with `center` field computed automatically from vertices, height, axis
geometric_object make_prism(MATERIAL_TYPE material, const vector3 *vertices, int num_vertices,
                            double height, vector3 axis);

// as make_prism, but with a rigid translation so that the prism is centered at center
geometric_object make_prism_with_center(MATERIAL_TYPE material, vector3 center,
                                        const vector3 *vertices, int num_vertices, double height,
                                        vector3 axis);

// slanted prism with `center` field computed automatically from vertices, height, axis, sidewall_angle
geometric_object make_slanted_prism(MATERIAL_TYPE material, const vector3 *vertices, int num_vertices,
                            double height, vector3 axis, double sidewall_angle);

// as make_slanted_prism, but with a rigid translation so that the prism is centered at center
geometric_object make_slanted_prism_with_center(MATERIAL_TYPE material, vector3 center,
                                        const vector3 *vertices, int num_vertices, double height,
                                        vector3 axis, double sidewall_angle);

int vector3_nearly_equal(vector3 v1, vector3 v2, double tolerance);

/* Add typemap for geometric object arrays */
%typemap(in) (geometric_object *items) {
    if (!PyList_Check($input)) {
        SWIG_exception(SWIG_TypeError, "Expected a list of geometric objects");
    }
    int size = PyList_Size($input);
    $1 = (geometric_object *)malloc(size * sizeof(geometric_object));
    for (int i = 0; i < size; i++) {
        PyObject *obj = PyList_GetItem($input, i);
        geometric_object *item;
        int res = SWIG_ConvertPtr(obj, (void **)&item, SWIGTYPE_p_geometric_object, 0);
        if (!SWIG_IsOK(res)) {
            free($1);
            SWIG_exception(SWIG_TypeError, "List items must be geometric objects");
        }
        $1[i] = *item;
    }
}

%typemap(freearg) (geometric_object *items) {
    free($1);
}

/* Add these typemaps before the geometric_object_list declaration */

%include <carrays.i>
%array_class(geometric_object, geometric_object_array);

%extend geometric_object_list {
    void set_items(PyObject* list) {
        if (!PyList_Check(list)) {
            PyErr_SetString(PyExc_TypeError, "Expected a list");
            return;
        }
        
        int size = PyList_Size(list);
        if (size != $self->num_items) {
            PyErr_SetString(PyExc_ValueError, "List size must match num_items");
            return;
        }
        
        $self->items = (geometric_object*)malloc(size * sizeof(geometric_object));
        for (int i = 0; i < size; i++) {
            PyObject *obj = PyList_GetItem(list, i);
            geometric_object *item;
            int res = SWIG_ConvertPtr(obj, (void**)&item, $descriptor(geometric_object*), 0);
            if (!SWIG_IsOK(res)) {
                free($self->items);
                PyErr_SetString(PyExc_TypeError, "List items must be geometric objects");
                return;
            }
            $self->items[i] = *item;
        }
    }
    
    PyObject* get_items() {
        PyObject* list = PyList_New($self->num_items);
        for (int i = 0; i < $self->num_items; i++) {
            PyObject* obj = SWIG_NewPointerObj(&$self->items[i], $descriptor(geometric_object*), 0);
            PyList_SET_ITEM(list, i, obj);
        }
        return list;
    }
}

// Ignore the original items attribute to prevent conflicts
%ignore geometric_object_list::items;