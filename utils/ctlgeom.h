/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998, 1999 Steven G. Johnson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 * Steven G. Johnson can be contacted at stevenj@alum.mit.edu.
 */

#ifndef GEOM_H
#define GEOM_H

#include <ctl-io.h>

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/**************************************************************************/

extern void geom_fix_object(geometric_object o);
extern void geom_fix_objects(void);
extern boolean point_in_objectp(vector3 p, geometric_object o);
extern boolean point_in_periodic_objectp(vector3 p, geometric_object o);
extern boolean point_in_fixed_objectp(vector3 p, geometric_object o);
extern boolean point_in_periodic_fixed_objectp(vector3 p, geometric_object o);
extern material_type material_of_point_inobject(vector3 p, boolean *inobject);
extern material_type material_of_point(vector3 p);
extern void display_geometric_object_info(int indentby, geometric_object o);
extern matrix3x3 square_basis(matrix3x3 lattice_basis, vector3 size);

typedef struct {
     vector3 low, high;
} geom_box;

typedef struct {
     geom_box box;
     geometric_object *o;
     vector3 shiftby;
} geom_box_object;

typedef struct geom_box_tree_struct {
     geom_box b, b1, b2;
     struct geom_box_tree_struct *t1, *t2;
     int nobjects;
     geom_box_object *objects;
} *geom_box_tree;

extern void destroy_geom_box_tree(geom_box_tree t);
extern geom_box_tree create_geom_box_tree(void);
extern material_type material_of_point_in_tree_inobject(vector3 p, geom_box_tree t, boolean *inobject);
extern material_type material_of_point_in_tree(vector3 p, geom_box_tree t);
extern void display_geom_box_tree(int indentby, geom_box_tree t);
extern void geom_box_tree_stats(geom_box_tree t, int *depth, int *nobjects);

/**************************************************************************/

#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */

#endif /* GEOM_H */
