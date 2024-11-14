%module libctlgeom

/* Include the header files first */
%{
#define CTL_SWIG
#include "ctlgeom.h"
#include "ctlgeom-types-swig.h"
%}

/* Include the type definitions */
%include "ctlgeom-types-swig.h"

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
// Ignore deprecated functions
%ignore geom_fix_object;
%ignore geom_fix_objects0;
%ignore get_grid_size;
%ignore get_resolution;
%ignore get_grid_size_n;

%include "ctlgeom.h"