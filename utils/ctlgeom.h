/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, Steven G. Johnson
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

#ifdef CXX_CTL_IO
#define MATERIAL_TYPE ctlio::material_type
#define GEOMETRIC_OBJECT ctlio::geometric_object
#define GEOMETRIC_OBJECT_LIST ctlio::geometric_object_list
#else
#define MATERIAL_TYPE material_type
#define GEOMETRIC_OBJECT geometric_object
#define GEOMETRIC_OBJECT_LIST geometric_object_list
#endif

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/**************************************************************************/

extern void geom_fix_object(GEOMETRIC_OBJECT o);
extern void geom_fix_objects(void);
extern void geom_fix_objects0(GEOMETRIC_OBJECT_LIST geometry);
extern boolean point_in_objectp(vector3 p, GEOMETRIC_OBJECT o);
extern boolean point_in_periodic_objectp(vector3 p, GEOMETRIC_OBJECT o);
extern boolean point_in_fixed_objectp(vector3 p, GEOMETRIC_OBJECT o);
extern boolean point_in_periodic_fixed_objectp(vector3 p, GEOMETRIC_OBJECT o);
extern MATERIAL_TYPE material_of_point_inobject(vector3 p, boolean *inobject);
extern MATERIAL_TYPE material_of_point_inobject0(
     GEOMETRIC_OBJECT_LIST geometry, vector3 p, boolean *inobject);
extern MATERIAL_TYPE material_of_point(vector3 p);
extern MATERIAL_TYPE material_of_point0(GEOMETRIC_OBJECT_LIST geometry,
					vector3 p);
extern void display_GEOMETRIC_OBJECT_info(int indentby, GEOMETRIC_OBJECT o);
extern matrix3x3 square_basis(matrix3x3 lattice_basis, vector3 size);

typedef struct {
     vector3 low, high;
} geom_box;

typedef struct {
     geom_box box;
     GEOMETRIC_OBJECT *o;
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
extern geom_box_tree create_geom_box_tree0(GEOMETRIC_OBJECT_LIST geometry,
					   geom_box b0);
extern geom_box_tree restrict_geom_box_tree(geom_box_tree, const geom_box *);
extern MATERIAL_TYPE material_of_point_in_tree_inobject(vector3 p, geom_box_tree t, boolean *inobject);
extern MATERIAL_TYPE material_of_point_in_tree_inobject0(GEOMETRIC_OBJECT_LIST geometry, vector3 p, geom_box_tree t, boolean *inobject);
extern MATERIAL_TYPE material_of_point_in_tree(vector3 p, geom_box_tree t);
extern MATERIAL_TYPE material_of_point_in_tree0(GEOMETRIC_OBJECT_LIST geometry, vector3 p, geom_box_tree t);
extern void display_geom_box_tree(int indentby, geom_box_tree t);
extern void geom_box_tree_stats(geom_box_tree t, int *depth, int *nobjects);

extern vector3 get_grid_size(void);
extern void get_grid_size_n(int *nx, int *ny, int *nz);

/**************************************************************************/

#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */

#endif /* GEOM_H */
