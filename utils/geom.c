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

#include <math.h>

#include "geom.h"

/**************************************************************************/

/* Return whether or not the point p (in the lattice basis) is inside
   the object o.

   Requires that the global input var geometry_lattice already be
   initialized. */

boolean point_in_objectp(vector3 p, geometric_object o)
{
  vector3 r = vector3_minus(p,o.center);

  switch (o.which_subclass) {
  case GEOMETRIC_OBJECT_SELF:
    return 0;
  case SPHERE:
    {
      number radius = o.subclass.sphere_data->radius;
      return(radius > 0.0 &&
	     vector3_dot(r,matrix3x3_vector3_mult(geometry_lattice.metric, r))
	     <= radius*radius);
    }
  case CYLINDER:
    {
      vector3 rm = matrix3x3_vector3_mult(geometry_lattice.metric, r);
      number proj = vector3_dot(o.subclass.cylinder_data->axis, rm);
      if (fabs(proj) <= 0.5 * o.subclass.cylinder_data->height) {
	number radius = o.subclass.cylinder_data->radius;
	return(radius > 0.0 && vector3_dot(r,rm) - proj*proj <= radius*radius);
      }
    }
  case BLOCK:
    {
      vector3 proj =
	matrix3x3_vector3_mult(o.subclass.block_data->projection_matrix, r);
      switch (o.subclass.block_data->which_subclass) {
      case BLOCK_SELF:
	{
	  vector3 size = o.subclass.block_data->size;
	  return(fabs(proj.x) <= 0.5 * size.x &&
		 fabs(proj.y) <= 0.5 * size.y &&
		 fabs(proj.z) <= 0.5 * size.z);
	}
      case ELLIPSOID:
	{
	  vector3 isa =
	    o.subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes;
	  double
	    a = proj.x * isa.x,
	    b = proj.y * isa.y,
	    c = proj.z * isa.z;
	  return(a*a + b*b + c*c <= 1.0);
	}
      }
    }
  }
  return 0;
}

/**************************************************************************/

/* Like point_in_objectp, but also checks the object shifted
   by the lattice vectors: */
boolean point_in_periodic_objectp(vector3 p, geometric_object o)
{
     int i, j, k;

     switch (dimensions) {
	 case 1:
	      for (i = -1; i <= 1; ++i) {
		   vector3 shift1 = p;
		   shift1.x += i * geometry_lattice.size.x;
		   if (point_in_objectp(shift1, o))
			return 1;
	      }
	      break;
	 case 2:
	      for (i = -1; i <= 1; ++i) {
		   vector3 shift1 = p;
		   shift1.x += i * geometry_lattice.size.x;
		   for (j = -1; j <= 1; ++j) {
			vector3 shift2 = shift1;
			shift2.y += j * geometry_lattice.size.y;
			if (point_in_objectp(shift2, o))
			     return 1;
		   }
	      }
	      break;
	 case 3:
	      for (i = -1; i <= 1; ++i) {
		   vector3 shift1 = p;
		   shift1.x += i * geometry_lattice.size.x;
		   for (j = -1; j <= 1; ++j) {
			vector3 shift2 = shift1;
			shift2.y += j * geometry_lattice.size.y;
			for (k = -1; k <= 1; ++k) {
			     vector3 shift3 = shift2;
			     shift2.z += k * geometry_lattice.size.z;
			     if (point_in_objectp(shift3, o))
				  return 1;
			}
		   }
	      }
	      break;
     }
     return 0;
}

/**************************************************************************/

/* Return the material type corresponding to the point p (in the lattice
   basis).  Returns default_material if p is not in any object.

   Requires that the global input vars geometry_lattice, geometry,
   dimensions, default_material and ensure_periodicity already be
   initialized. */

material_type material_of_point(vector3 p)
{
     int index;
     
     /* loop in reverse order so that later items are given precedence: */
     for (index = geometry.num_items - 1; index >= 0; --index) {
	  if (ensure_periodicity
	      && point_in_periodic_objectp(p, geometry.items[index])
	      || point_in_objectp(p, geometry.items[index]))
	       return(geometry.items[index].material);
     }
     return default_material;
}

/**************************************************************************/

/* Given a geometric object o, display some information about it,
   indented by indentby spaces. */

void display_geometric_object_info(int indentby, geometric_object o)
{
     printf("%*s", indentby, "");
     switch (o.which_subclass) {
	 case CYLINDER:
	      printf("cylinder");
	      break;
	 case SPHERE:
	      printf("sphere");
	      break;
	 case BLOCK:
	      switch (o.subclass.block_data->which_subclass) {
		  case ELLIPSOID:
		       printf("ellipsoid");
		       break;
		  case BLOCK_SELF:
		       printf("block");
		       break;
	      }
	      break;
	 default:
	      printf("geometric object");
              break;
     }
     printf(", center = (%g,%g,%g)\n",
	    o.center.x, o.center.y, o.center.z);
     switch (o.which_subclass) {
	 case CYLINDER:
	      printf("%*s     radius %g, height %g, axis (%g, %g, %g)\n",
		     indentby, "", o.subclass.cylinder_data->radius,
                     o.subclass.cylinder_data->height,
                     o.subclass.cylinder_data->axis.x,
                     o.subclass.cylinder_data->axis.y,
                     o.subclass.cylinder_data->axis.z);
	      break;
	 case SPHERE:
              printf("%*s     radius %g\n", indentby, "", 
		     o.subclass.sphere_data->radius);
              break;
	 case BLOCK:
	      printf("%*s     size (%g,%g,%g)\n", indentby, "",
		     o.subclass.block_data->size.x,
                     o.subclass.block_data->size.y,
                     o.subclass.block_data->size.z);
	      printf("%*s     axes (%g,%g,%g), (%g,%g,%g), (%g,%g,%g)\n",
		     indentby, "",
		     o.subclass.block_data->e1.x,
                     o.subclass.block_data->e1.y,
                     o.subclass.block_data->e1.z,
		     o.subclass.block_data->e2.x,
                     o.subclass.block_data->e2.y,
                     o.subclass.block_data->e2.z,
		     o.subclass.block_data->e3.x,
                     o.subclass.block_data->e3.y,
                     o.subclass.block_data->e3.z);
	      break;
	 default:
	      break;
     }
}

/**************************************************************************/

/* Given a basis (matrix columns are the basis unit vectors) and the
   size of the lattice (in basis vectors), returns a new "square"
   basis.  This corresponds to a region of the same volume, but made
   rectangular, suitable for outputing to an HDF file.

   Given a vector in the range (0..1, 0..1, 0..1), multiplying by
   the square basis matrix will yield the coordinates of a point
   in the rectangular volume, given in the lattice basis. */

matrix3x3 square_basis(matrix3x3 basis, vector3 size)
{
  matrix3x3 square;

  square.c0 = basis.c0;

  square.c1 = vector3_minus(basis.c1, vector3_scale(vector3_dot(basis.c0,
								basis.c1),
						    basis.c1));

  square.c2 = vector3_minus(basis.c2, vector3_scale(vector3_dot(basis.c0,
								basis.c2),
						    basis.c2));
  square.c2 = vector3_minus(square.c2, vector3_scale(vector3_dot(basis.c0,
								 square.c2),
						     unit_vector3(square.c2)));

  square.c0 = vector3_scale(size.x, square.c0);
  square.c1 = vector3_scale(size.y, square.c1);
  square.c2 = vector3_scale(size.z, square.c2);

  return matrix3x3_mult(matrix3x3_inverse(basis), square);
}

/**************************************************************************/

#ifdef MATERIAL_WEIGHT_PROPERTY

/* If MATERIAL_WEIGHT_PROPERTY is defined to be one of the numeric
   properties of the material type class, the following routine
   returns several local averages (or "moments") of the weight property
   near the point p.

   mean is the average value of the weight, inv_mean is the inverse of
   the average of the inverse of the weight, and moment is the
   "dipole moment" (average of weight times displacement) converted
   to a unit vector.

   The averages are performed on a regular mesh centered on p with
   spacing in each dimension given by mesh_delta (in the lattice
   basis).  The mesh has 2*nmesh+1 points in each lattice direction. */

void material_weight_moments(vector3 p, integer nmesh, vector3 mesh_delta,
			     vector3 *moment,number *mean,number *inv_mean)
{
  int i,j,k, n;
  vector3 shift = { 0, 0, 0 };
  number weight;

  *mean = *inv_mean = moment->x = moment->y = moment->z = 0.0;
  n = 2*nmesh + 1;

  switch (dimensions) {
  case 1:
    for (i = -nmesh; i <= nmesh; ++i) {
      shift.x = i * mesh_delta.x;
      weight =
	material_of_point(vector3_plus(p,shift)).MATERIAL_WEIGHT_PROPERTY;
      *mean += weight;
      *inv_mean += 1.0/weight;
      *moment = vector3_plus(*moment, vector3_scale(weight,shift));
    }
    break;
  case 2:
    for (i = -nmesh; i <= nmesh; ++i) {
      shift.x = i * mesh_delta.x;
      for (j = -nmesh; j <= nmesh; ++j) {
	shift.y = j * mesh_delta.y;
	weight =
	  material_of_point(vector3_plus(p,shift)).MATERIAL_WEIGHT_PROPERTY;
	*mean += weight;
	*inv_mean += 1.0/weight;
	*moment = vector3_plus(*moment, vector3_scale(weight,shift));
      }
    }
    n = n*n;
    break;
  case 3:
    for (i = -nmesh; i <= nmesh; ++i) {
      shift.x = i * mesh_delta.x;
      for (j = -nmesh; j <= nmesh; ++j) {
	shift.y = j * mesh_delta.y;
	for (k = -nmesh; k <= nmesh; ++k) {
	  shift.z = k * mesh_delta.z;
	  weight =
	    material_of_point(vector3_plus(p,shift)).MATERIAL_WEIGHT_PROPERTY;
	  *mean += weight;
	  *inv_mean += 1.0/weight;
	  *moment = vector3_plus(*moment, vector3_scale(weight,shift));
	}
      }
    }
    n = n*n*n;
    break;
  }

  *mean /= n;
  *inv_mean = n / *inv_mean;
  *moment = unit_vector3(*moment);
}

#endif

/**************************************************************************/
