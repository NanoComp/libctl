%module libctlgeom

/* Include all SWIG modules first */
%include "cpointer.i"
%include "carrays.i"
%include "ctl-math.h"

%{
typedef void *material_type;
#define SWIG_FILE_WITH_INIT
#include "ctlgeom.h"
#include "ctlgeom-types.h"
%}

%include "ctlgeom-types.h"
%include "ctl-math.h"
%include "cpointer.i"
%pointer_functions(int, intp);

/* Define material_type with proper typemap */
%typemap(in) material_type {
    Py_XINCREF($input);  // Increment reference count when passing to C
    $1 = (void*)$input;
}

%typemap(out) material_type {
    if ($1 == NULL) {
        Py_INCREF(Py_None);
        $result = Py_None;
    } else {
        $result = (PyObject*)$1;
        Py_XINCREF($result);  // Increment reference count when returning to Python
    }
}
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


%include <carrays.i>
%array_class(geometric_object, geometric_object_array);
%array_class(vector3, vector3_array);

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

%ignore geometric_object_list::items;
%ignore geom_fix_object;
%ignore geom_fix_objects0;
%ignore get_grid_size;
%ignore get_resolution;
%ignore get_grid_size_n;

%include "ctlgeom.h"

%include "numpy.i"
%init %{
    import_array();
%}

/* Add typemap for numpy array input */
%apply (double* IN_ARRAY2, int DIM1, int DIM2) {(double* points, int n_points, int dims)}

/* Add the new function declaration */
%inline %{
#include <numpy/arrayobject.h>
#include <numpy/npy_math.h>

PyObject* material_of_numpy_points_in_tree(double* points, int n_points, int dims, geom_box_tree t) {
    if (dims != 3) {
        PyErr_SetString(PyExc_ValueError, "Input array must have 3 columns (x,y,z)");
        return NULL;
    }

    PyObject* result = PyList_New(n_points);
    for (int i = 0; i < n_points; i++) {
        vector3 p = {points[i*3], points[i*3 + 1], points[i*3 + 2]};
        material_type material = material_of_point_in_tree(p, t);

        if (material == NULL) {
            PyObject* nan = PyFloat_FromDouble(NPY_NAN);  // Create numpy.nan
            PyList_SET_ITEM(result, i, nan);
        } else {
            PyObject* mat = (PyObject*)material;
            Py_XINCREF(mat);
            PyList_SET_ITEM(result, i, mat);
        }
    }
    return result;
}
%}
