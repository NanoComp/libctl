/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998 Steven G. Johnson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
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

extern boolean point_in_objectp(vector3 p, geometric_object o);
extern material_type material_of_point(vector3 p);
extern matrix3x3 square_basis(matrix3x3 lattice_basis, vector3 size);

#ifdef MATERIAL_WEIGHT_PROPERTY
extern void material_weight_moments(vector3 p,
				    integer nmesh, vector3 mesh_delta,
				    vector3 *moment,
				    number *mean, number *inv_mean);
#endif

/**************************************************************************/

#ifdef __cplusplus
	   }                               /* extern "C" */
#endif                          /* __cplusplus */

#endif /* GEOM_H */
