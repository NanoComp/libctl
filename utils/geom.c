/* libctl: flexible Guile-based control files for scientific software
 * Copyright (C) 1998-2014 Massachusetts Institute of Technology and Steven G. Johnson
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef LIBCTLGEOM
#  include "ctl-io.h"
#else
#  define material_type void*
   static void material_type_copy(void **src, void **dest) { *dest = *src; }
#endif
#include "ctlgeom.h"

#ifdef CXX_CTL_IO
using namespace ctlio;
#  define CTLIO ctlio::
#  define GEOM geometric_object::
#  define BLK block::
#  define CYL cylinder::
#  define MAT material_type::
#else
#  define CTLIO
#  define GEOM
#  define BLK
#  define CYL
#  define MAT
#endif

#ifdef __cplusplus
#  define MALLOC(type,num) (new type[num])
#  define MALLOC1(type) (new type)
#  define FREE(p) delete[] (p)
#  define FREE1(p) delete (p)
#else
#  define MALLOC(type,num) ((type *) malloc(sizeof(type) * (num)))
#  define MALLOC1(type) MALLOC(type,1)
#  define FREE(p) free(p)
#  define FREE1(p) free(p)
#endif

#define K_PI 3.14159265358979323846
#define CHECK(cond, s) if (!(cond)){fprintf(stderr,s "\n");exit(EXIT_FAILURE);}

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// forward declarations of prism-related routines, at the bottom of this file
static vector3 prism_vector_p2c(prism *prsm, vector3 vp);
static vector3 prism_vector_c2p(prism *prsm, vector3 vc);
static void get_prism_bounding_box(prism *prsm, geom_box *box);
static double intersect_line_segment_with_prism(prism *prsm, vector3 p, vector3 d, double a, double b);
static boolean point_in_polygon(double px, double py, vector3 *vertices, int num_vertices);
static boolean point_in_prism(prism *prsm, vector3 p);
static vector3 normal_to_prism(prism *prsm, vector3 p);
static void display_prism_info(int indentby, prism *prsm);
/**************************************************************************/

/* If v is a vector in the lattice basis, normalize v so that
   its cartesian length is unity. */
static void lattice_normalize(vector3 *v)
{
     *v = vector3_scale(
	  1.0 /
	  sqrt(vector3_dot(*v,
			   matrix3x3_vector3_mult(geometry_lattice.metric,
						  *v))),
	  *v);
}

static vector3 lattice_to_cartesian(vector3 v)
{
     return matrix3x3_vector3_mult(geometry_lattice.basis, v);
}

static vector3 cartesian_to_lattice(vector3 v)
{
     return matrix3x3_vector3_mult(matrix3x3_inverse(geometry_lattice.basis),
				   v);
}

/* "Fix" the parameters of the given object to account for the
   geometry_lattice basis, which may be non-orthogonal.  In particular,
   this means that the normalization of several unit vectors, such
   as the cylinder or block axes, needs to be changed.

   Unfortunately, we can't do this stuff at object-creation time
   in Guile, because the geometry_lattice variable may not have
   been assigned to its final value at that point.  */
void geom_fix_object(geometric_object o)
{
     switch(o.which_subclass) {
	 case GEOM CYLINDER:
	      lattice_normalize(&o.subclass.cylinder_data->axis);
	      if (o.subclass.cylinder_data->which_subclass == CYL WEDGE) {
		   vector3 a = o.subclass.cylinder_data->axis;
		   vector3 s = o.subclass.cylinder_data->subclass.wedge_data->wedge_start;
		   double p = vector3_dot(s, matrix3x3_vector3_mult(geometry_lattice.metric, a));
		   o.subclass.cylinder_data->subclass.wedge_data->e1 =
			vector3_minus(s, vector3_scale(p, a));
		   lattice_normalize(&o.subclass.cylinder_data->subclass.wedge_data->e1);
		   o.subclass.cylinder_data->subclass.wedge_data->e2 =
			cartesian_to_lattice(
			     vector3_cross(lattice_to_cartesian(o.subclass.cylinder_data->axis),
					   lattice_to_cartesian(o.subclass.cylinder_data->subclass.wedge_data->e1)));
	      }
	      break;
	 case GEOM BLOCK:
	 {
	      matrix3x3 m;
	      lattice_normalize(&o.subclass.block_data->e1);
	      lattice_normalize(&o.subclass.block_data->e2);
	      lattice_normalize(&o.subclass.block_data->e3);
	      m.c0 = o.subclass.block_data->e1;
	      m.c1 = o.subclass.block_data->e2;
	      m.c2 = o.subclass.block_data->e3;
	      o.subclass.block_data->projection_matrix = matrix3x3_inverse(m);
	      break;
	 }
	 case GEOM PRISM:
	 {
	 }
	 break;
	 case GEOM COMPOUND_GEOMETRIC_OBJECT:
	 {
	      int i;
	      int n = o.subclass.compound_geometric_object_data
		   ->component_objects.num_items;
	      geometric_object *os = o.subclass.compound_geometric_object_data
		   ->component_objects.items;
	      for (i = 0; i < n; ++i) {
#if MATERIAL_TYPE_ABSTRACT
		   if (os[i].material.which_subclass == MAT MATERIAL_TYPE_SELF)
			material_type_copy(&o.material, &os[i].material);
#endif
		   geom_fix_object(os[i]);
	      }
	      break;
	 }
	 case GEOM GEOMETRIC_OBJECT_SELF: case GEOM SPHERE:
	      break; /* these objects are fine */
     }
}

/* fix all objects in the geometry list as described in
   geom_fix_object, above */
void geom_fix_objects0(geometric_object_list geometry)
{
     int index;

     for (index = 0; index < geometry.num_items; ++index)
	  geom_fix_object(geometry.items[index]);
}

void geom_fix_objects(void)
{
     geom_fix_objects0(geometry);
}

void geom_fix_lattice0(lattice *L)
{
     L->basis1 = unit_vector3(L->basis1);
     L->basis2 = unit_vector3(L->basis2);
     L->basis3 = unit_vector3(L->basis3);
     L->b1 = vector3_scale(L->basis_size.x, L->basis1);
     L->b2 = vector3_scale(L->basis_size.y, L->basis2);
     L->b3 = vector3_scale(L->basis_size.z, L->basis3);
     L->basis.c0 = L->b1;
     L->basis.c1 = L->b2;
     L->basis.c2 = L->b3;
     L->metric = matrix3x3_mult(matrix3x3_transpose(L->basis), L->basis);
}

void geom_fix_lattice(void)
{
     geom_fix_lattice0(&geometry_lattice);
}

void geom_cartesian_lattice0(lattice *L)
{
     L->basis1.x = 1; L->basis1.y = 0; L->basis1.z = 0;
     L->basis2.x = 0; L->basis2.y = 1; L->basis2.z = 0;
     L->basis3.x = 0; L->basis3.y = 0; L->basis3.z = 1;
     L->basis_size.x = L->basis_size.y = L->basis_size.z = 1;
     geom_fix_lattice0(L);
}

void geom_cartesian_lattice(void)
{
     geom_cartesian_lattice0(&geometry_lattice);
}

void geom_initialize(void)
{
     /* initialize many of the input variables that are normally
	initialized from Scheme, except for default_material and
	geometry_lattice.size. */
     geom_cartesian_lattice();
     geometry_center.x = geometry_center.y = geometry_center.z = 0;
     dimensions = 3;
     ensure_periodicity = 1;
     geometry.num_items = 0;
     geometry.items = 0;
}

/**************************************************************************/

/* Return whether or not the point p (in the lattice basis) is inside
   the object o.

   Requires that the global input var geometry_lattice already be
   initialized.

   point_in_fixed_objectp additionally requires that geom_fix_object
   has been called on o (if the lattice basis is non-orthogonal).  */

boolean CTLIO point_in_objectp(vector3 p, geometric_object o)
{
     geom_fix_object(o);
     return point_in_fixed_objectp(p, o);
}

boolean point_in_fixed_objectp(vector3 p, geometric_object o)
{
     return point_in_fixed_pobjectp(p, &o);
}

/* as point_in_fixed_objectp, but sets o to the object in question (if true)
   (which may be different from the input o if o is a compound object) */
boolean point_in_fixed_pobjectp(vector3 p, geometric_object *o)
{
  vector3 r = vector3_minus(p,o->center);

  switch (o->which_subclass) {
  case GEOM GEOMETRIC_OBJECT_SELF:
    return 0;
  case GEOM SPHERE:
    {
      number radius = o->subclass.sphere_data->radius;
      return(radius > 0.0 &&
	     vector3_dot(r,matrix3x3_vector3_mult(geometry_lattice.metric, r))
	     <= radius*radius);
    }
  case GEOM CYLINDER:
    {
      vector3 rm = matrix3x3_vector3_mult(geometry_lattice.metric, r);
      number proj = vector3_dot(o->subclass.cylinder_data->axis, rm);
      number height = o->subclass.cylinder_data->height;
      if (fabs(proj) <= 0.5 * height) {
	number radius = o->subclass.cylinder_data->radius;
	if (o->subclass.cylinder_data->which_subclass == CYL CONE)
	     radius += (proj/height + 0.5) *
		  (o->subclass.cylinder_data->subclass.cone_data->radius2
		   - radius);
	else if (o->subclass.cylinder_data->which_subclass == CYL WEDGE) {
	     number x = vector3_dot(rm, o->subclass.cylinder_data->subclass.wedge_data->e1);
	     number y = vector3_dot(rm, o->subclass.cylinder_data->subclass.wedge_data->e2);
	     number theta = atan2(y, x);
	     number wedge_angle = o->subclass.cylinder_data->subclass.wedge_data->wedge_angle;
	     if (wedge_angle > 0) {
		  if (theta < 0) theta = theta + 2 * K_PI;
		  if (theta > wedge_angle) return 0;
	     }
	     else {
		  if (theta > 0) theta = theta - 2 * K_PI;
		  if (theta < wedge_angle) return 0;
	     }
	}
	return(radius != 0.0 && vector3_dot(r,rm) - proj*proj <= radius*radius);
      }
      else
	return 0;
    }
  case GEOM BLOCK:
    {
      vector3 proj =
	matrix3x3_vector3_mult(o->subclass.block_data->projection_matrix, r);
      switch (o->subclass.block_data->which_subclass) {
      case BLK BLOCK_SELF:
	{
	  vector3 size = o->subclass.block_data->size;
	  return(fabs(proj.x) <= 0.5 * size.x &&
		 fabs(proj.y) <= 0.5 * size.y &&
		 fabs(proj.z) <= 0.5 * size.z);
	}
      case BLK ELLIPSOID:
	{
	  vector3 isa =
	    o->subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes;
	  double
	    a = proj.x * isa.x,
	    b = proj.y * isa.y,
	    c = proj.z * isa.z;
	  return(a*a + b*b + c*c <= 1.0);
	}
      }
    }
  case GEOM PRISM:
    {
      return point_in_prism(o->subclass.prism_data, p);
    }
  case GEOM COMPOUND_GEOMETRIC_OBJECT:
    {
	 int i;
	 int n = o->subclass.compound_geometric_object_data
	      ->component_objects.num_items;
	 geometric_object *os = o->subclass.compound_geometric_object_data
	      ->component_objects.items;
	 vector3 shiftby = o->center;
	 for (i = 0; i < n; ++i) {
	      *o = os[i];
	      o->center = vector3_plus(o->center, shiftby);
	      if (point_in_fixed_pobjectp(p, o))
		   return 1;
	 }
	 break;
    }
  }
  return 0;
}

/**************************************************************************/

/* convert a point p inside o to a coordinate in [0,1]^3 that
   is some "natural" coordinate for the object */
vector3 to_geom_object_coords(vector3 p, geometric_object o)
{
  const vector3 half = {0.5, 0.5, 0.5};
  vector3 r = vector3_minus(p,o.center);

  switch (o.which_subclass) {
  default: {
       vector3 po = {0,0,0};
       return po;
  }
  case GEOM SPHERE:
    {
      number radius = o.subclass.sphere_data->radius;
      return vector3_plus(half, vector3_scale(0.5 / radius, r));
    }
  /* case GEOM CYLINDER:
     NOT YET IMPLEMENTED */
  case GEOM BLOCK:
    {
      vector3 proj =
	matrix3x3_vector3_mult(o.subclass.block_data->projection_matrix, r);
      vector3 size = o.subclass.block_data->size;
      if (size.x != 0.0) proj.x /= size.x;
      if (size.y != 0.0) proj.y /= size.y;
      if (size.z != 0.0) proj.z /= size.z;
      return vector3_plus(half, proj);
    }
/* case GEOM PRISM:
    NOT YET IMPLEMENTED */
  }
}

/* inverse of to_geom_object_coords */
vector3 from_geom_object_coords(vector3 p, geometric_object o)
{
  const vector3 half = {0.5, 0.5, 0.5};
  p = vector3_minus(p, half);
  switch (o.which_subclass) {
  default:
       return o.center;
  case GEOM SPHERE:
    {
      number radius = o.subclass.sphere_data->radius;
      return vector3_plus(o.center, vector3_scale(radius / 0.5, p));
    }
  /* case GEOM CYLINDER:
     NOT YET IMPLEMENTED */
  case GEOM BLOCK:
    {
      vector3 size = o.subclass.block_data->size;
      return vector3_plus(
	   o.center,
	   vector3_plus(
		vector3_scale(size.x * p.x, o.subclass.block_data->e1),
		vector3_plus(
		     vector3_scale(size.y * p.y, o.subclass.block_data->e2),
		     vector3_scale(size.z * p.z, o.subclass.block_data->e3))
		));
    }
/* case GEOM PRISM:
    NOT YET IMPLEMENTED */
  }
}

/**************************************************************************/
/* Return the normal vector from the given object to the given point,
   in lattice coordinates, using the surface of the object that the
   point is "closest" to for some definition of "closest" that is
   reasonable (at least for points near to the object). The length and
   sign of the normal vector are arbitrary. */

vector3 CTLIO normal_to_object(vector3 p, geometric_object o)
{
     geom_fix_object(o);
     return normal_to_fixed_object(p, o);
}

vector3 normal_to_fixed_object(vector3 p, geometric_object o)
{
  vector3 r = vector3_minus(p,o.center);

  switch (o.which_subclass) {
  case GEOM CYLINDER:
    {
      vector3 rm = matrix3x3_vector3_mult(geometry_lattice.metric, r);
      double proj = vector3_dot(o.subclass.cylinder_data->axis, rm),
	   height = o.subclass.cylinder_data->height,
	   radius, prad;
      if (fabs(proj) > height * 0.5)
	   return o.subclass.cylinder_data->axis;
      radius = o.subclass.cylinder_data->radius;
      prad = sqrt(fabs(vector3_dot(r,rm) - proj*proj));
      if (o.subclass.cylinder_data->which_subclass == CYL CONE)
	   radius += (proj/height + 0.5) *
		(o.subclass.cylinder_data->subclass.cone_data->radius2
		 - radius);
      if (fabs(fabs(proj) - height * 0.5) < fabs(prad - radius))
	   return o.subclass.cylinder_data->axis;
      if (o.subclass.cylinder_data->which_subclass == CYL CONE)
	   return vector3_minus(r, vector3_scale(proj + prad * (o.subclass.cylinder_data->subclass.cone_data->radius2 - radius) / height, o.subclass.cylinder_data->axis));
      else
	   return vector3_minus(r, vector3_scale(proj, o.subclass.cylinder_data->axis));
    }
  case GEOM BLOCK:
    {
      vector3 proj =
	matrix3x3_vector3_mult(o.subclass.block_data->projection_matrix, r);
      switch (o.subclass.block_data->which_subclass) {
      case BLK BLOCK_SELF:
	{
	  vector3 size = o.subclass.block_data->size;
	  double d1 = fabs(fabs(proj.x) - 0.5 * size.x);
	  double d2 = fabs(fabs(proj.y) - 0.5 * size.y);
	  double d3 = fabs(fabs(proj.z) - 0.5 * size.z);
	  if (d1 < d2 && d1 < d3)
	       return matrix3x3_row1(o.subclass.block_data->projection_matrix);
	  else if (d2 < d3)
	       return matrix3x3_row2(o.subclass.block_data->projection_matrix);
	  else
	       return matrix3x3_row3(o.subclass.block_data->projection_matrix);
	}
      case BLK ELLIPSOID:
	{
	  vector3 isa =
	    o.subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes;
	  proj.x *= isa.x * isa.x;
	  proj.y *= isa.y * isa.y;
	  proj.z *= isa.z * isa.z;
	  return matrix3x3_transpose_vector3_mult(
	       o.subclass.block_data->projection_matrix, proj);
	}
      } // switch
    } // case GEOM BLOCK
  case GEOM PRISM:
    {
      return normal_to_prism(o.subclass.prism_data, p);
    }
  default:
       return r;
  }
}

/**************************************************************************/

/* Here is a useful macro to loop over different possible shifts of
   the lattice vectors.  body is executed for each possible shift,
   where the shift is given by the value of shiftby (which should
   be a vector3 variable).  I would much rather make this a function,
   but C's lack of lambda-like function construction or closures makes
   this easier to do as a macro.  (One could at least wish for
   an easier way to make multi-line macros.)  */

#define LOOP_PERIODIC(shiftby, body) { \
     switch (dimensions) { \
	 case 1: \
	 { \
	      int iii; \
	      shiftby.y = shiftby.z = 0; \
	      for (iii = -1; iii <= 1; ++iii) { \
		   shiftby.x = iii * geometry_lattice.size.x; \
		   body; \
	      } \
	      break; \
	 } \
	 case 2: \
	 { \
	      int iii, jjj; \
	      shiftby.z = 0; \
	      for (iii = -1; iii <= 1; ++iii) { \
		   shiftby.x = iii * geometry_lattice.size.x; \
		   for (jjj = -1; jjj <= 1; ++jjj) { \
			shiftby.y = jjj * geometry_lattice.size.y; \
			body; \
		   } \
	      } \
	      break; \
	 } \
	 case 3: \
	 { \
	      int iii, jjj, kkk; \
	      for (iii = -1; iii <= 1; ++iii) { \
		   shiftby.x = iii * geometry_lattice.size.x; \
		   for (jjj = -1; jjj <= 1; ++jjj) { \
			shiftby.y = jjj * geometry_lattice.size.y; \
			for (kkk = -1; kkk <= 1; ++kkk) { \
			     shiftby.z = kkk * geometry_lattice.size.z; \
			     body; \
			} \
		   } \
	      } \
	      break; \
	 } \
     } \
}

/**************************************************************************/

/* Like point_in_objectp, but also checks the object shifted
   by the lattice vectors: */

boolean CTLIO point_in_periodic_objectp(vector3 p, geometric_object o)
{
     geom_fix_object(o);
     return point_in_periodic_fixed_objectp(p, o);
}

boolean point_in_periodic_fixed_objectp(vector3 p, geometric_object o)
{
     vector3 shiftby;
     LOOP_PERIODIC(shiftby,
		   if (point_in_fixed_objectp(vector3_minus(p, shiftby), o))
		        return 1);
     return 0;
}

boolean point_shift_in_periodic_fixed_pobjectp(vector3 p, geometric_object *o,
					       vector3 *shiftby)
{
     geometric_object o0 = *o;
     LOOP_PERIODIC((*shiftby),
		   {
			*o = o0;
			if (point_in_fixed_pobjectp(
			     vector3_minus(p, *shiftby), o))
			     return 1;
		   });
     return 0;
}

/**************************************************************************/

/* Functions to return the object or material type corresponding to
   the point p (in the lattice basis).  Returns default_material if p
   is not in any object.

   Requires that the global input vars geometry_lattice, geometry,
   dimensions, default_material and ensure_periodicity already be
   initialized.

   Also requires that geom_fix_objects() has been called!

   material_of_point_inobject is a variant that also returns whether
   or not the point was in any object.  */

geometric_object object_of_point0(geometric_object_list geometry, vector3 p,
				  vector3 *shiftby)
{
     geometric_object o;
     int index;
     shiftby->x = shiftby->y = shiftby->z = 0;
     /* loop in reverse order so that later items are given precedence: */
     for (index = geometry.num_items - 1; index >= 0; --index) {
	  o = geometry.items[index];
	  if ((ensure_periodicity
	       && point_shift_in_periodic_fixed_pobjectp(p, &o, shiftby))
	      || point_in_fixed_pobjectp(p, &o))
	       return o;
     }
     o.which_subclass = GEOM GEOMETRIC_OBJECT_SELF; /* no object found */
     return o;
}

geometric_object object_of_point(vector3 p, vector3 *shiftby)
{
     return object_of_point0(geometry, p, shiftby);
}

material_type material_of_point_inobject0(geometric_object_list geometry,
					  vector3 p, boolean *inobject)
{
     vector3 shiftby;
     geometric_object o = object_of_point0(geometry, p, &shiftby);
     *inobject = o.which_subclass != GEOM GEOMETRIC_OBJECT_SELF;;
     return (*inobject ? o.material : default_material);
}

material_type material_of_point_inobject(vector3 p, boolean *inobject)
{
     return material_of_point_inobject0(geometry, p, inobject);
}

material_type material_of_point0(geometric_object_list geometry, vector3 p)
{
     boolean inobject;
     return material_of_point_inobject0(geometry, p, &inobject);
}

material_type material_of_point(vector3 p)
{
     return material_of_point0(geometry, p);
}

/**************************************************************************/

/* Given a geometric object o, display some information about it,
   indented by indentby spaces. */

void CTLIO display_geometric_object_info(int indentby, geometric_object o)
{
     geom_fix_object(o);
     printf("%*s", indentby, "");
     switch (o.which_subclass) {
	 case GEOM CYLINDER:
	      switch (o.subclass.cylinder_data->which_subclass) {
		  case CYL WEDGE:
		       printf("wedge");
		       break;
		  case CYL CONE:
		       printf("cone");
		       break;
		  case CYL CYLINDER_SELF:
		       printf("cylinder");
		       break;
	      }
	      break;
	 case GEOM SPHERE:
	      printf("sphere");
	      break;
	 case GEOM BLOCK:
	      switch (o.subclass.block_data->which_subclass) {
		  case BLK ELLIPSOID:
		       printf("ellipsoid");
		       break;
		  case BLK BLOCK_SELF:
		       printf("block");
		       break;
	      }
	      break;
	 case GEOM PRISM:
	      printf("prism");
	      break;
	 case GEOM COMPOUND_GEOMETRIC_OBJECT:
	      printf("compound object");
	      break;
	 default:
	      printf("geometric object");
              break;
     }
     printf(", center = (%g,%g,%g)\n",
	    o.center.x, o.center.y, o.center.z);
     switch (o.which_subclass) {
	 case GEOM CYLINDER:
	      printf("%*s     radius %g, height %g, axis (%g, %g, %g)\n",
		     indentby, "", o.subclass.cylinder_data->radius,
                     o.subclass.cylinder_data->height,
                     o.subclass.cylinder_data->axis.x,
                     o.subclass.cylinder_data->axis.y,
                     o.subclass.cylinder_data->axis.z);
	      if (o.subclass.cylinder_data->which_subclass == CYL CONE)
		   printf("%*s     radius2 %g\n", indentby, "",
		        o.subclass.cylinder_data->subclass.cone_data->radius2);
	      else if (o.subclass.cylinder_data->which_subclass == CYL WEDGE)
		   printf("%*s     wedge-theta %g\n", indentby, "",
		        o.subclass.cylinder_data->subclass.wedge_data->wedge_angle);
	      break;
	 case GEOM SPHERE:
              printf("%*s     radius %g\n", indentby, "",
		     o.subclass.sphere_data->radius);
              break;
	 case GEOM BLOCK:
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
	 case GEOM PRISM:
	      display_prism_info(indentby, o.subclass.prism_data);
	      break;
	 case GEOM COMPOUND_GEOMETRIC_OBJECT:
	 {
	      int i;
	      int n = o.subclass.compound_geometric_object_data
		   ->component_objects.num_items;
	      geometric_object *os = o.subclass.compound_geometric_object_data
		   ->component_objects.items;
	      printf("%*s     %d components:\n", indentby, "", n);
	      for (i = 0; i < n; ++i)
		   display_geometric_object_info(indentby + 5, os[i]);
	      break;
	 }
	 default:
	      break;
     }
}

/**************************************************************************/

/* Compute the intersections with o of a line along p+s*d, returning
   the number of intersections (at most 2) and the two intersection "s"
   values in s[0] and s[1].   (Note: o must not be a compound object.) */
int intersect_line_with_object(vector3 p, vector3 d, geometric_object o,
			       double s[2])
{
     vector3 p0=p;
     p = vector3_minus(p, o.center);
     s[0] = s[1] = 0;
     switch (o.which_subclass) {
	 case GEOM SPHERE: {
	      number radius = o.subclass.sphere_data->radius;
	      vector3 dm = matrix3x3_vector3_mult(geometry_lattice.metric, d);
	      double a = vector3_dot(d, dm);
	      double b2 = -vector3_dot(dm, p);
	      double c = vector3_dot(p, matrix3x3_vector3_mult(
		   geometry_lattice.metric, p)) - radius * radius;
	      double discrim = b2*b2 - a*c;
	      if (discrim < 0)
		   return 0;
	      else if (discrim == 0) {
		   s[0] = b2 / a;
		   return 1;
	      }
	      else {
		   discrim = sqrt(discrim);
		   s[0] = (b2 + discrim) / a;
		   s[1] = (b2 - discrim) / a;
		   return 2;
	      }
	 } // case GEOM SPHERE
	 case GEOM CYLINDER: {
	      vector3 dm = matrix3x3_vector3_mult(geometry_lattice.metric, d);
	      vector3 pm = matrix3x3_vector3_mult(geometry_lattice.metric, p);
	      number height = o.subclass.cylinder_data->height;
	      number radius = o.subclass.cylinder_data->radius;
	      number radius2 = o.subclass.cylinder_data->which_subclass == CYL CONE ? o.subclass.cylinder_data->subclass.cone_data->radius2 : radius;
	      double dproj = vector3_dot(o.subclass.cylinder_data->axis, dm);
	      double pproj = vector3_dot(o.subclass.cylinder_data->axis, pm);
	      double D = (radius2 - radius) / height;
	      double L = radius + (radius2 - radius) * 0.5 + pproj*D;
	      double a = vector3_dot(d,dm) - dproj*dproj * (1 + D*D);
	      double b2 = dproj * (pproj + D*L) - vector3_dot(p,dm);
	      double c = vector3_dot(p,pm) - pproj*pproj - L*L;
	      double discrim = b2*b2 - a*c;
	      int ret;
	      if (a == 0) { /* linear equation */
		   if (b2 == 0) {
			if (c == 0) { /* infinite intersections */
			     s[0] = ((height * 0.5) - pproj) / dproj;
			     s[1] = -((height * 0.5) + pproj) / dproj;
			     return 2;
			}
			else
			     ret = 0;
		   }
		   else {
			s[0] = 0.5 * c / b2;
			ret = 1;
		   }
	      }
              else if (discrim < 0)
                   ret = 0;
              else if (discrim == 0) {
                   s[0] = b2 / a;
                   ret = 1;
              }
              else {
                   discrim = sqrt(discrim);
                   s[0] = (b2 + discrim) / a;
                   s[1] = (b2 - discrim) / a;
                   ret = 2;
	      }
	      if (ret == 2 && fabs(pproj + s[1] * dproj) > height * 0.5)
		   ret = 1;
	      if (ret >= 1 && fabs(pproj + s[0] * dproj) > height * 0.5) {
		   --ret;
		   s[0] = s[1];
	      }
	      if (ret == 2 || dproj == 0)
		   return ret;
	      /* find intersections with endcaps */
	      s[ret] = (height * 0.5 - pproj) / dproj;
	      if (a * s[ret]*s[ret] - 2*b2 * s[ret] + c <= 0)
		   ++ret;
	      if (ret < 2) {
		   s[ret] = -(height * 0.5 + pproj) / dproj;
		   if (a * s[ret]*s[ret] - 2*b2 * s[ret] + c <= 0)
			++ret;
	      }
	      if (ret == 2 && s[0] == s[1]) ret = 1;
	      return ret;
	 } // case GEOM CYLINDER
	 case GEOM BLOCK:
	 {
	      vector3 dproj = matrix3x3_vector3_mult(o.subclass.block_data->projection_matrix, d);
	      vector3 pproj = matrix3x3_vector3_mult(o.subclass.block_data->projection_matrix, p);
	      switch (o.subclass.block_data->which_subclass) {
		  case BLK BLOCK_SELF:
		  {
		       vector3 size = o.subclass.block_data->size;
		       int ret = 0;
		       size.x *= 0.5; size.y *= 0.5; size.z *= 0.5;
		       if (dproj.x != 0) {
			    s[ret] = (size.x - pproj.x) / dproj.x;
			    if (fabs(pproj.y+s[ret]*dproj.y) <= size.y &&
				fabs(pproj.z+s[ret]*dproj.z) <= size.z)
				 ++ret;
			    s[ret] = (-size.x - pproj.x) / dproj.x;
			    if (fabs(pproj.y+s[ret]*dproj.y) <= size.y &&
				fabs(pproj.z+s[ret]*dproj.z) <= size.z)
				 ++ret;
			    if (ret == 2) return 2;
		       }
		       if (dproj.y != 0) {
			    s[ret] = (size.y - pproj.y) / dproj.y;
			    if (fabs(pproj.x+s[ret]*dproj.x) <= size.x &&
				fabs(pproj.z+s[ret]*dproj.z) <= size.z)
				 ++ret;
			    if (ret == 2) return 2;
			    s[ret] = (-size.y - pproj.y) / dproj.y;
			    if (fabs(pproj.x+s[ret]*dproj.x) <= size.x &&
				fabs(pproj.z+s[ret]*dproj.z) <= size.z)
				 ++ret;
			    if (ret == 2) return 2;
		       }
		       if (dproj.z != 0) {
			    s[ret] = (size.z - pproj.z) / dproj.z;
			    if (fabs(pproj.x+s[ret]*dproj.x) <= size.x &&
				fabs(pproj.y+s[ret]*dproj.y) <= size.y)
				 ++ret;
			    if (ret == 2) return 2;
			    s[ret] = (-size.z - pproj.z) / dproj.z;
			    if (fabs(pproj.x+s[ret]*dproj.x) <= size.x &&
				fabs(pproj.y+s[ret]*dproj.y) <= size.y)
				 ++ret;
		       }
		       return ret;
		  }
		  case BLK ELLIPSOID:
		  {
		       vector3 isa = o.subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes;
		       double a, b2, c, discrim;
		       dproj.x *= isa.x; dproj.y *= isa.y; dproj.z *= isa.z;
		       pproj.x *= isa.x; pproj.y *= isa.y; pproj.z *= isa.z;
		       a = vector3_dot(dproj, dproj);
		       b2 = -vector3_dot(dproj, pproj);
		       c = vector3_dot(pproj, pproj) - 1;
		       discrim = b2*b2 - a*c;
		       if (discrim < 0)
			    return 0;
		       else if (discrim == 0) {
			    s[0] = b2 / a;
			    return 1;
		       }
		       else {
			    discrim = sqrt(discrim);
			    s[0] = (b2 + discrim) / a;
			    s[1] = (b2 - discrim) / a;
			    return 2;
		       }
		  }
	      }
	 } // case GEOM BLOCK

	 default:
	      return 0;
     }
}

/* Compute the intersections with o of a line along p+s*d in the interval s in [a,b], returning
    the length of the s intersection in this interval.  (Note: o must not be a compound object.) */
double intersect_line_segment_with_object(vector3 p, vector3 d, geometric_object o, double a, double b)
{
  if (o.which_subclass==GEOM PRISM)
   {
     return intersect_line_segment_with_prism(o.subclass.prism_data, p, d, a, b);
   }
  else
   { double s[2];
     if (2 == intersect_line_with_object(p, d, o, s))
      { double ds = (s[0] < s[1] ? MIN(s[1],b) - MAX(s[0],a)
	           	         : MIN(s[0],b) - MAX(s[1],a)
                    );
	return (ds > 0 ? ds : 0.0);
      }
     else
      return 0.0;
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

matrix3x3 CTLIO square_basis(matrix3x3 basis, vector3 size)
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
/**************************************************************************/

		     /* Fast geometry routines */

/* Using the above material_of_point routine is way too slow, especially
   when there are lots of objects to test.  Thus, we develop the following
   replacement routines.

   The basic idea here is twofold.  (1) Compute bounding boxes for
   each geometric object, for which inclusion tests can be computed
   quickly.  (2) Build a tree that recursively breaks down the unit cell
   in half, allowing us to perform searches in logarithmic time. */

/**************************************************************************/

/* geom_box utilities: */
static void geom_box_union(geom_box *bu,
			   const geom_box *b1, const geom_box *b2)
{
     bu->low.x = MIN(b1->low.x, b2->low.x);
     bu->low.y = MIN(b1->low.y, b2->low.y);
     bu->low.z = MIN(b1->low.z, b2->low.z);
     bu->high.x = MAX(b1->high.x, b2->high.x);
     bu->high.y = MAX(b1->high.y, b2->high.y);
     bu->high.z = MAX(b1->high.z, b2->high.z);
}

static void geom_box_intersection(geom_box *bi,
				  const geom_box *b1,
				  const geom_box *b2)
{
     bi->low.x = MAX(b1->low.x, b2->low.x);
     bi->low.y = MAX(b1->low.y, b2->low.y);
     bi->low.z = MAX(b1->low.z, b2->low.z);
     bi->high.x = MIN(b1->high.x, b2->high.x);
     bi->high.y = MIN(b1->high.y, b2->high.y);
     bi->high.z = MIN(b1->high.z, b2->high.z);
}

static void geom_box_add_pt(geom_box *b, vector3 p)
{
     b->low.x = MIN(b->low.x, p.x);
     b->low.y = MIN(b->low.y, p.y);
     b->low.z = MIN(b->low.z, p.z);
     b->high.x = MAX(b->high.x, p.x);
     b->high.y = MAX(b->high.y, p.y);
     b->high.z = MAX(b->high.z, p.z);
}

#define BETWEEN(x, low, high) ((x) >= (low) && (x) <= (high))

static int geom_box_contains_point(const geom_box *b, vector3 p)
{
     return (BETWEEN(p.x, b->low.x, b->high.x) &&
	     BETWEEN(p.y, b->low.y, b->high.y) &&
	     BETWEEN(p.z, b->low.z, b->high.z));
}

/* return whether or not the given two boxes intersect */
static int geom_boxes_intersect(const geom_box *b1, const geom_box *b2)
{
     /* true if the x, y, and z ranges all intersect. */
     return ((BETWEEN(b1->low.x, b2->low.x, b2->high.x) ||
	      BETWEEN(b1->high.x, b2->low.x, b2->high.x) ||
	      BETWEEN(b2->low.x, b1->low.x, b1->high.x)) &&
	     (BETWEEN(b1->low.y, b2->low.y, b2->high.y) ||
	      BETWEEN(b1->high.y, b2->low.y, b2->high.y) ||
	      BETWEEN(b2->low.y, b1->low.y, b1->high.y)) &&
	     (BETWEEN(b1->low.z, b2->low.z, b2->high.z) ||
	      BETWEEN(b1->high.z, b2->low.z, b2->high.z) ||
	      BETWEEN(b2->low.z, b1->low.z, b1->high.z)));
}

static void geom_box_shift(geom_box *b, vector3 shiftby)
{
     b->low = vector3_plus(b->low, shiftby);
     b->high = vector3_plus(b->high, shiftby);
}

/**************************************************************************/

/* Computing a bounding box for a geometric object: */

/* compute | (b x c) / (a * (b x c)) |, for use below */
static number compute_dot_cross(vector3 a, vector3 b, vector3 c)
{
     vector3 bxc = vector3_cross(b, c);
     return fabs(vector3_norm(bxc) / vector3_dot(a, bxc));
}

/* Compute a bounding box for the object o, preferably the smallest
   bounding box.  The box is a parallelepiped with axes given by
   the geometry lattice vectors, and its corners are given in the
   lattice basis.

   Requires that geometry_lattice global has been initialized,
   etcetera.  */
void geom_get_bounding_box(geometric_object o, geom_box *box)
{
     geom_fix_object(o);

     /* initialize to empty box at the center of the object: */
     box->low = box->high = o.center;

     switch (o.which_subclass) {
	 case GEOM GEOMETRIC_OBJECT_SELF:
	      break;
	 case GEOM SPHERE:
	 {
	      /* Find the parallelepiped that the sphere inscribes.
		 The math comes out surpisingly simple--try it! */

	      number radius = o.subclass.sphere_data->radius;
	      /* actually, we could achieve the same effect here
		 by inverting the geometry_lattice.basis matrix... */
	      number r1 = compute_dot_cross(geometry_lattice.b1,
					    geometry_lattice.b2,
					    geometry_lattice.b3) * radius;
	      number r2 = compute_dot_cross(geometry_lattice.b2,
					    geometry_lattice.b3,
					    geometry_lattice.b1) * radius;
	      number r3 = compute_dot_cross(geometry_lattice.b3,
					    geometry_lattice.b1,
					    geometry_lattice.b2) * radius;
	      box->low.x -= r1;
	      box->low.y -= r2;
	      box->low.z -= r3;
	      box->high.x += r1;
	      box->high.y += r2;
	      box->high.z += r3;
	      break;
	 }
	 case GEOM CYLINDER:
	 {
	      /* Find the bounding boxes of the two (circular) ends of
		 the cylinder, then take the union.  Again, the math
		 for finding the bounding parallelepiped of a circle
		 comes out suprisingly simple in the end.  Proof left
		 as an exercise for the reader. */

	      number radius = o.subclass.cylinder_data->radius;
	      number h = o.subclass.cylinder_data->height * 0.5;
	      vector3 axis = /* cylinder axis in cartesian coords */
		   matrix3x3_vector3_mult(geometry_lattice.basis,
					  o.subclass.cylinder_data->axis);
	      vector3 e12 = vector3_cross(geometry_lattice.basis1,
					  geometry_lattice.basis2);
	      vector3 e23 = vector3_cross(geometry_lattice.basis2,
					  geometry_lattice.basis3);
	      vector3 e31 = vector3_cross(geometry_lattice.basis3,
					  geometry_lattice.basis1);
	      number elen2, eproj;
	      number r1, r2, r3;
	      geom_box tmp_box;

	      /* Find bounding box dimensions, in lattice coords,
		 for the circular ends of the cylinder: */

	      elen2 = vector3_dot(e23, e23);
	      eproj = vector3_dot(e23, axis);
	      r1 = fabs(sqrt(fabs(elen2 - eproj*eproj)) /
			vector3_dot(e23, geometry_lattice.b1));

	      elen2 = vector3_dot(e31, e31);
	      eproj = vector3_dot(e31, axis);
	      r2 = fabs(sqrt(fabs(elen2 - eproj*eproj)) /
			vector3_dot(e31, geometry_lattice.b2));

	      elen2 = vector3_dot(e12, e12);
	      eproj = vector3_dot(e12, axis);
	      r3 = fabs(sqrt(fabs(elen2 - eproj*eproj)) /
			vector3_dot(e12, geometry_lattice.b3));

	      /* Get axis in lattice coords: */
	      axis = o.subclass.cylinder_data->axis;

	      tmp_box = *box; /* set tmp_box to center of object */

	      /* bounding box for -h*axis cylinder end: */
	      box->low.x -= h * axis.x + r1*radius;
	      box->low.y -= h * axis.y + r2*radius;
	      box->low.z -= h * axis.z + r3*radius;
	      box->high.x -= h * axis.x - r1*radius;
	      box->high.y -= h * axis.y - r2*radius;
	      box->high.z -= h * axis.z - r3*radius;

	      if (o.subclass.cylinder_data->which_subclass == CYL CONE)
		   radius =
		   fabs(o.subclass.cylinder_data->subclass.cone_data->radius2);

	      /* bounding box for +h*axis cylinder end: */
	      tmp_box.low.x += h * axis.x - r1*radius;
	      tmp_box.low.y += h * axis.y - r2*radius;
	      tmp_box.low.z += h * axis.z - r3*radius;
	      tmp_box.high.x += h * axis.x + r1*radius;
	      tmp_box.high.y += h * axis.y + r2*radius;
	      tmp_box.high.z += h * axis.z + r3*radius;

	      geom_box_union(box, box, &tmp_box);
	      break;
	 }
	 case GEOM BLOCK:
	 {
	      /* blocks are easy: just enlarge the box to be big enough to
		 contain all 8 corners of the block. */

	      vector3 s1 = vector3_scale(o.subclass.block_data->size.x,
					 o.subclass.block_data->e1);
	      vector3 s2 = vector3_scale(o.subclass.block_data->size.y,
					 o.subclass.block_data->e2);
	      vector3 s3 = vector3_scale(o.subclass.block_data->size.z,
					 o.subclass.block_data->e3);
	      vector3 corner =
		   vector3_plus(o.center,
		      vector3_scale(-0.5,
                                    vector3_plus(s1, vector3_plus(s2, s3))));

	      geom_box_add_pt(box, corner);
	      geom_box_add_pt(box, vector3_plus(corner, s1));
	      geom_box_add_pt(box, vector3_plus(corner, s2));
	      geom_box_add_pt(box, vector3_plus(corner, s3));
	      geom_box_add_pt(box, vector3_plus(corner, vector3_plus(s1, s2)));
	      geom_box_add_pt(box, vector3_plus(corner, vector3_plus(s1, s3)));
	      geom_box_add_pt(box, vector3_plus(corner, vector3_plus(s3, s2)));
	      geom_box_add_pt(box,
	        vector3_plus(corner, vector3_plus(s1, vector3_plus(s2, s3))));
	      break;
	 }
	 case GEOM PRISM:
	 {
           get_prism_bounding_box(o.subclass.prism_data, box);
           break;
	 }
	 case GEOM COMPOUND_GEOMETRIC_OBJECT:
	 {
	      int i;
	      int n = o.subclass.compound_geometric_object_data
		   ->component_objects.num_items;
	      geometric_object *os = o.subclass.compound_geometric_object_data
		   ->component_objects.items;
	      for (i = 0; i < n; ++i) {
		   geom_box boxi;
		   geom_get_bounding_box(os[i], &boxi);
		   geom_box_shift(&boxi, o.center);
		   geom_box_union(box, box, &boxi);
	      }
	      break;
	 }
     }
}

/**************************************************************************/
/* Compute the fraction of a box's volume (or area/length in 2d/1d) that
   overlaps an object.   Instead of a box, we also allow an ellipsoid
   inscribed inside the box (or a skewed ellipsoid if the box is not
   orthogonal). */

typedef struct {
     geometric_object o;
     vector3 p, dir;
     int pdim[2]; /* the (up to two) integration directions */
     double scx[2]; /* scale factor (e.g. sign flip) for x coordinates */
     unsigned dim;
     double a0, b0; /* box limits along analytic direction */
     int is_ellipsoid; /* 0 for box, 1 for ellipsoid */
     double winv[2], c[2]; /* ellipsoid width-inverses/centers in int. dirs */
     double w0, c0; /* width/center along analytic direction */
} overlap_data;

static double overlap_integrand(integer ndim, number *x, void *data_)
{
     overlap_data *data = (overlap_data *) data_;
     double s[2];
     const double *scx = data->scx;
     vector3 p = data->p;
     double a0 = data->a0, b0 = data->b0;
     double scale_result = 1.0;

     if (ndim > 0) {
	  switch (data->pdim[0]) {
	      case 0: p.x = scx[0] * x[0]; break;
	      case 1: p.y = scx[0] * x[0]; break;
	      case 2: p.z = scx[0] * x[0]; break;
	  }
	  if (ndim > 1) {
	       switch (data->pdim[1]) {
		   case 0: p.x = scx[1] * x[1]; break;
		   case 1: p.y = scx[1] * x[1]; break;
		   case 2: p.z = scx[1] * x[1]; break;
	       }
	  }
     }

     if (data->is_ellipsoid && ndim > 0) {
	  /* compute width of ellipsoid at this point, along the
	     analytic-intersection direction */
	  double dx = (x[0] - data->c[0]) * data->winv[0];
	  double w = 1.0 - dx * dx;
	  if (ndim > 1) { /* rescale 2nd dimension to stay inside ellipsoid */
	       double x1;
	       if (w < 0) return 0.0; /* outside the ellipsoid */
	       scale_result = sqrt(w);
	       x1 = data->c[1] + (x[1] - data->c[1]) * scale_result;
	       switch (data->pdim[1]) {
		   case 0: p.x = scx[1] * x1; break;
		   case 1: p.y = scx[1] * x1; break;
		   case 2: p.z = scx[1] * x1; break;
               }
	       dx = (x1 - data->c[1]) * data->winv[1];
	       w -= dx * dx;
	  }
	  if (w < 0) return 0.0; /* outside the ellipsoid */
	  w = data->w0 * sqrt(w);
	  a0 = data->c0 - w; b0 = data->c0 + w;
     }

    return intersect_line_segment_with_object(p, data->dir, data->o, a0, b0) * scale_result;
}

number overlap_with_object(geom_box b, int is_ellipsoid, geometric_object o,
			   number tol, integer maxeval)
{
     overlap_data data;
     int empty_x = b.low.x == b.high.x;
     int empty_y = b.low.y == b.high.y;
     int empty_z = b.low.z == b.high.z;
     double V0 = ((empty_x ? 1 : b.high.x - b.low.x) *
		  (empty_y ? 1 : b.high.y - b.low.y) *
		  (empty_z ? 1 : b.high.z - b.low.z));
     vector3 ex = {1,0,0}, ey = {0,1,0}, ez = {0,0,1};
     geom_box bb;
     double xmin[2] = {0,0}, xmax[2] = {0,0}, esterr;
     int errflag;
     unsigned i;

     geom_get_bounding_box(o, &bb);
     geom_box_intersection(&bb, &b, &bb);
     if (bb.low.x > bb.high.x || bb.low.y > bb.high.y || bb.low.z > bb.high.z
	 || (!empty_x && bb.low.x == bb.high.x)
	 || (!empty_y && bb.low.y == bb.high.y)
	 || (!empty_z && bb.low.z == bb.high.z))
	  return 0.0;

     data.winv[0] = data.winv[1] = data.w0 = 1.0;
     data.c[0] = data.c[1] = data.c0 = 0;

     data.o = o;
     data.p.x = data.p.y = data.p.z = 0;
     data.dim = 0;
     if (!empty_x) {
	  data.dir = ex;
	  data.a0 = bb.low.x;
	  data.b0 = bb.high.x;
	  data.w0 = 0.5 * (b.high.x - b.low.x);
	  data.c0 = 0.5 * (b.high.x + b.low.x);
	  if (!empty_y) {
	       xmin[data.dim] = bb.low.y;
	       xmax[data.dim] = bb.high.y;
	       data.winv[data.dim] = 2.0 / (b.high.y - b.low.y);
	       data.c[data.dim] = 0.5 * (b.high.y + b.low.y);
	       data.pdim[data.dim++] = 1;
	  }
	  if (!empty_z) {
	       xmin[data.dim] = bb.low.z;
	       xmax[data.dim] = bb.high.z;
	       data.winv[data.dim] = 2.0 / (b.high.z - b.low.z);
	       data.c[data.dim] = 0.5 * (b.high.z + b.low.z);
	       data.pdim[data.dim++] = 2;
	  }
     }
     else if (!empty_y) {
	  data.dir = ey;
	  data.a0 = bb.low.y;
	  data.b0 = bb.high.y;
	  data.w0 = 0.5 * (b.high.y - b.low.y);
	  data.c0 = 0.5 * (b.high.y + b.low.y);
	  if (!empty_x) {
	       xmin[data.dim] = bb.low.x;
	       xmax[data.dim] = bb.high.x;
	       data.winv[data.dim] = 2.0 / (b.high.x - b.low.x);
	       data.c[data.dim] = 0.5 * (b.high.x + b.low.x);
	       data.pdim[data.dim++] = 0;
	  }
	  if (!empty_z) {
	       xmin[data.dim] = bb.low.z;
	       xmax[data.dim] = bb.high.z;
	       data.winv[data.dim] = 2.0 / (b.high.z - b.low.z);
	       data.c[data.dim] = 0.5 * (b.high.z + b.low.z);
	       data.pdim[data.dim++] = 2;
	  }
     }
     else if (!empty_z) {
	  data.dir = ez;
	  data.a0 = bb.low.z;
	  data.b0 = bb.high.z;
	  data.w0 = 0.5 * (b.high.z - b.low.z);
	  data.c0 = 0.5 * (b.high.z + b.low.z);
	  if (!empty_x) {
	       xmin[data.dim] = bb.low.x;
	       xmax[data.dim] = bb.high.x;
	       data.winv[data.dim] = 2.0 / (b.high.x - b.low.x);
	       data.c[data.dim] = 0.5 * (b.high.x + b.low.x);
	       data.pdim[data.dim++] = 0;
	  }
	  if (!empty_y) {
	       xmin[data.dim] = bb.low.y;
	       xmax[data.dim] = bb.high.y;
	       data.winv[data.dim] = 2.0 / (b.high.y - b.low.y);
	       data.c[data.dim] = 0.5 * (b.high.y + b.low.y);
	       data.pdim[data.dim++] = 1;
	  }
     }
     else
	  return 1.0;

#if 1
     /* To maintain mirror symmetries through the x/y/z axes, we flip
        the integration range whenever xmax < 0.  (This is in case
        the integration routine is not fully symmetric, which may
        happen(?) due to the upper bound on the #evaluations.)*/
     for (i = 0; i < data.dim; ++i) {
	  if (xmax[i] < 0) {
	       double xm = xmin[i];
	       data.scx[i] = -1;
	       xmin[i] = -xmax[i];
	       xmax[i] = -xm;
	       data.c[i] = -data.c[i];
	  }
	  else
	       data.scx[i] = 1;
     }
#else
     for (i = 0; i < data.dim; ++i) data.scx[i] = 1;
#endif

     if ((data.is_ellipsoid = is_ellipsoid)) { /* data for ellipsoid calc. */
	  if (data.dim == 1)
	       V0 *= K_PI / 4;
	  else if (data.dim == 2)
	       V0 *= K_PI / 6;
     }

     return adaptive_integration(overlap_integrand, xmin, xmax,
				 data.dim, &data,
				 0.0, tol, maxeval,
				 &esterr, &errflag) / V0;
}

number box_overlap_with_object(geom_box b, geometric_object o,
			       number tol, integer maxeval)
{
     return overlap_with_object(b, 0, o, tol, maxeval);
}

number ellipsoid_overlap_with_object(geom_box b, geometric_object o,
				     number tol, integer maxeval)
{
     return overlap_with_object(b, 1, o, tol, maxeval);
}

number CTLIO range_overlap_with_object(vector3 low, vector3 high,
				       geometric_object o, number tol,
				       integer maxeval)
{
     geom_box b;
     b.low = low;
     b.high = high;
     return box_overlap_with_object(b, o, tol, maxeval);
}

/**************************************************************************/

/* geom_box_tree: a tree of boxes and the objects contained within
   them.  The tree recursively partitions the unit cell, allowing us
   to perform binary searches for the object containing a given point. */

void destroy_geom_box_tree(geom_box_tree t)
{
     if (t) {
	  destroy_geom_box_tree(t->t1);
	  destroy_geom_box_tree(t->t2);
	  if (t->nobjects && t->objects)
	       FREE(t->objects);
	  FREE1(t);
     }
}

/* return whether the object o, shifted by the vector shiftby,
   possibly intersects b.  Upon return, obj_b is the bounding
   box for o. */
static int object_in_box(geometric_object o, vector3 shiftby,
			 geom_box *obj_b, const geom_box *b)
{
     geom_get_bounding_box(o, obj_b);
     geom_box_shift(obj_b, shiftby);
     return geom_boxes_intersect(obj_b, b);
}

static geom_box_tree new_geom_box_tree(void)
{
     geom_box_tree t;

     t = MALLOC1(struct geom_box_tree_struct);
     CHECK(t, "out of memory");
     t->t1 = t->t2 = NULL;
     t->nobjects = 0;
     t->objects = NULL;
     return t;
}

/* Divide b into b1 and b2, cutting b in two along the axis
   divide_axis (0 = x, 1 = y, 2 = z) at divide_point. */
static void divide_geom_box(const geom_box *b,
			    int divide_axis, number divide_point,
			    geom_box *b1, geom_box *b2)
{
     *b1 = *b2 = *b;
     switch (divide_axis) {
	 case 0:
	      b1->high.x = b2->low.x = divide_point;
	      break;
	 case 1:
	      b1->high.y = b2->low.y = divide_point;
	      break;
	 case 2:
	      b1->high.z = b2->low.z = divide_point;
	      break;
     }
}

#define VEC_I(v,i) ((i) == 0 ? (v).x : ((i) == 1 ? (v).y : (v).z))
#define SMALL 1.0e-7

/* Find the best place (best_partition) to "cut" along the axis
   divide_axis in order to maximally divide the objects between
   the partitions.  Upon return, n1 and n2 are the number of objects
   below and above the partition, respectively. */
static void find_best_partition(int nobjects, const geom_box_object *objects,
				int divide_axis,
				number *best_partition, int *n1, int *n2)
{
     number cur_partition;
     int i, j, cur_n1, cur_n2;

     *n1 = *n2 = nobjects + 1;
     *best_partition = 0;

     /* Search for the best partition, by checking all possible partitions
	either just above the high end of an object or just below the
	low end of an object. */

     for (i = 0; i < nobjects; ++i) {
         cur_partition = VEC_I(objects[i].box.high, divide_axis) * (1 + SMALL);
	  cur_n1 = cur_n2 = 0;
	  for (j = 0; j < nobjects; ++j) {
	       if (VEC_I(objects[j].box.low, divide_axis) <= cur_partition)
		    ++cur_n1;
	       if (VEC_I(objects[j].box.high, divide_axis) >= cur_partition)
		    ++cur_n2;
	  }
	  CHECK(cur_n1 + cur_n2 >= nobjects, "bug 1 in find_best_partition");
	  if (MAX(cur_n1, cur_n2) < MAX(*n1, *n2)) {
	       *best_partition = cur_partition;
	       *n1 = cur_n1;
	       *n2 = cur_n2;
	  }
     }
     for (i = 0; i < nobjects; ++i) {
         cur_partition = VEC_I(objects[i].box.low, divide_axis) * (1 - SMALL);
	  cur_n1 = cur_n2 = 0;
	  for (j = 0; j < nobjects; ++j) {
	       if (VEC_I(objects[j].box.low, divide_axis) <= cur_partition)
		    ++cur_n1;
	       if (VEC_I(objects[j].box.high, divide_axis) >= cur_partition)
		    ++cur_n2;
	  }
	  CHECK(cur_n1 + cur_n2 >= nobjects, "bug 2 in find_best_partition");
	  if (MAX(cur_n1, cur_n2) < MAX(*n1, *n2)) {
	       *best_partition = cur_partition;
	       *n1 = cur_n1;
	       *n2 = cur_n2;
	  }
     }
}

/* divide_geom_box_tree: recursively divide t in two, each time
   dividing along the axis that maximally partitions the boxes,
   and only stop partitioning when partitioning doesn't help any
   more.  Upon return, t points to the partitioned tree. */
static void divide_geom_box_tree(geom_box_tree t)
{
     int division_nobjects[3][2] = {{0,0},{0,0},{0,0}};
     number division_point[3];
     int best = 0;
     int i, j, n1, n2;

     if (!t)
	  return;
     if (t->t1 || t->t2) {  /* this node has already been divided */
	  divide_geom_box_tree(t->t1);
	  divide_geom_box_tree(t->t2);
	  return;
     }

     if (t->nobjects <= 2)
	  return;  /* no point in partitioning */

     /* Try partitioning along each dimension, counting the
	number of objects in the partitioned boxes and finding
	the best partition. */
     for (i = 0; i < dimensions; ++i) {
	  find_best_partition(t->nobjects, t->objects, i, &division_point[i],
			      &division_nobjects[i][0],
			      &division_nobjects[i][1]);
	  if (MAX(division_nobjects[i][0], division_nobjects[i][1]) <
	      MAX(division_nobjects[best][0], division_nobjects[best][1]))
	       best = i;
     }

     /* don't do anything if division makes the worst case worse or if
	it fails to improve the best case: */
     if (MAX(division_nobjects[best][0], division_nobjects[best][1]) + 1 >
	 t->nobjects ||
	 MIN(division_nobjects[best][0], division_nobjects[best][1]) + 1 >=
         t->nobjects)
	  return;  /* division didn't help us */

     divide_geom_box(&t->b, best, division_point[best], &t->b1, &t->b2);
     t->t1 = new_geom_box_tree();
     t->t2 = new_geom_box_tree();
     t->t1->b = t->b1;
     t->t2->b = t->b2;

     t->t1->nobjects = division_nobjects[best][0];
     t->t1->objects = MALLOC(geom_box_object, t->t1->nobjects);
     CHECK(t->t1->objects, "out of memory");

     t->t2->nobjects = division_nobjects[best][1];
     t->t2->objects = MALLOC(geom_box_object, t->t2->nobjects);
     CHECK(t->t2->objects, "out of memory");

     for (j = n1 = n2 = 0; j < t->nobjects; ++j) {
	  if (geom_boxes_intersect(&t->b1, &t->objects[j].box)) {
	       CHECK(n1 < t->t1->nobjects, "BUG in divide_geom_box_tree");
	       t->t1->objects[n1++] = t->objects[j];
	  }
	  if (geom_boxes_intersect(&t->b2, &t->objects[j].box)) {
	       CHECK(n2 < t->t2->nobjects, "BUG in divide_geom_box_tree");
	       t->t2->objects[n2++] = t->objects[j];
	  }
     }
     CHECK(j == t->nobjects && n1 == t->t1->nobjects && n2 == t->t2->nobjects,
	   "BUG in divide_geom_box_tree: wrong nobjects");

     t->nobjects = 0;
     FREE(t->objects);
     t->objects = NULL;

     divide_geom_box_tree(t->t1);
     divide_geom_box_tree(t->t2);
}

geom_box_tree create_geom_box_tree(void)
{
     geom_box b0;
     b0.low = vector3_plus(geometry_center,
			   vector3_scale(-0.5, geometry_lattice.size));
     b0.high = vector3_plus(geometry_center,
			    vector3_scale(0.5, geometry_lattice.size));
     return create_geom_box_tree0(geometry, b0);
}

static int num_objects_in_box(const geometric_object *o, vector3 shiftby,
			      const geom_box *b)
{
     if (o->which_subclass == GEOM COMPOUND_GEOMETRIC_OBJECT) {
	  int n = o->subclass.compound_geometric_object_data
	       ->component_objects.num_items;
	  geometric_object *os = o->subclass.compound_geometric_object_data
	       ->component_objects.items;
	  int i, sum = 0;
	  shiftby = vector3_plus(shiftby, o->center);
	  for (i = 0; i < n; ++i)
	       sum += num_objects_in_box(os + i, shiftby, b);
	  return sum;
     }
     else {
	  geom_box ob;
	  return object_in_box(*o, shiftby, &ob, b);
     }
}

static int store_objects_in_box(const geometric_object *o, vector3 shiftby,
				const geom_box *b,
				geom_box_object *bo,
				int precedence)
{
     if (o->which_subclass == GEOM COMPOUND_GEOMETRIC_OBJECT) {
	  int n = o->subclass.compound_geometric_object_data
	       ->component_objects.num_items;
	  geometric_object *os = o->subclass.compound_geometric_object_data
	       ->component_objects.items;
	  int i, sum = 0;
	  shiftby = vector3_plus(shiftby, o->center);
	  for (i = 0; i < n; ++i)
	       sum += store_objects_in_box(os + i, shiftby, b, bo + sum,
					   precedence - sum);
	  return sum;
     }
     else {
	  geom_box ob;
	  if (object_in_box(*o, shiftby, &ob, b)) {
	       bo->box = ob;
	       bo->o = o;
	       bo->shiftby = shiftby;
	       bo->precedence = precedence;
	       return 1;
	  }
	  else
	       return 0;
     }
}

geom_box_tree create_geom_box_tree0(geometric_object_list geometry,
				    geom_box b0)
{
     geom_box_tree t = new_geom_box_tree();
     int i, index;

     t->b = b0;

     for (i = geometry.num_items - 1; i >= 0; --i) {
	  vector3 shiftby = {0,0,0};
	  if (ensure_periodicity) {
	       LOOP_PERIODIC(shiftby,
			     t->nobjects += num_objects_in_box(
				  geometry.items + i, shiftby, &t->b));
	  }
	  else
	       t->nobjects += num_objects_in_box(
		    geometry.items + i, shiftby, &t->b);
     }

     t->objects = MALLOC(geom_box_object, t->nobjects);
     CHECK(t->objects || t->nobjects == 0, "out of memory");

     for (i = geometry.num_items - 1, index = 0; i >= 0; --i) {
	  vector3 shiftby = {0,0,0};
	  if (ensure_periodicity) {
	       int precedence = t->nobjects - index;
	       LOOP_PERIODIC(shiftby,
			     index += store_objects_in_box(
				  geometry.items + i, shiftby, &t->b,
				  t->objects + index, precedence));
	  }
	  else
	       index += store_objects_in_box(
		    geometry.items + i, shiftby, &t->b,
		    t->objects + index, t->nobjects - index);
     }
     CHECK(index == t->nobjects, "bug in create_geom_box_tree0");

     divide_geom_box_tree(t);

     return t;
}

/* create a new tree from t, pruning all nodes that don't intersect b */
geom_box_tree restrict_geom_box_tree(geom_box_tree t, const geom_box *b)
{
     geom_box_tree tr;
     int i, j;

     if (!t || !geom_boxes_intersect(&t->b, b))
	  return NULL;

     tr = new_geom_box_tree();

     for (i = 0, j = 0; i < t->nobjects; ++i)
	  if (geom_boxes_intersect(&t->objects[i].box, b))
	       ++j;
     tr->nobjects = j;
     tr->objects = MALLOC(geom_box_object, tr->nobjects);
     CHECK(tr->objects || tr->nobjects == 0, "out of memory");

     for (i = 0, j = 0; i < t->nobjects; ++i)
	  if (geom_boxes_intersect(&t->objects[i].box, b))
	       tr->objects[j++] = t->objects[i];

     tr->t1 = restrict_geom_box_tree(t->t1, b);
     tr->t2 = restrict_geom_box_tree(t->t2, b);

     if (tr->nobjects == 0) {
	  if (tr->t1 && !tr->t2) {
	       geom_box_tree tr0 = tr;
	       tr = tr->t1;
	       FREE1(tr0);
	  }
	  else if (tr->t2 && !tr->t1) {
	       geom_box_tree tr0 = tr;
	       tr = tr->t2;
	       FREE1(tr0);
	  }
     }

     return tr;
}

/**************************************************************************/

/* recursively search the tree for the given point, returning the
   subtree (if any) that contains it and the index oindex of the
   object in that tree.  The input value of oindex indicates the
   starting object to search in t (0 to search all). */
static geom_box_tree tree_search(vector3 p, geom_box_tree t, int *oindex)
{
     int i;
     geom_box_tree gbt;

     if (!t || !geom_box_contains_point(&t->b, p))
	  return NULL;

     for (i = *oindex; i < t->nobjects; ++i)
	  if (geom_box_contains_point(&t->objects[i].box, p) &&
	      point_in_fixed_objectp(vector3_minus(p, t->objects[i].shiftby),
				     *t->objects[i].o)) {
	       *oindex = i;
	       return t;
	  }

     *oindex = 0;
     gbt = tree_search(p, t->t1, oindex);
     if (!gbt)
	  gbt = tree_search(p, t->t2, oindex);
     return gbt;
}

/* shift p to be within the unit cell of the lattice (centered on the
   origin) */
vector3 shift_to_unit_cell(vector3 p)
{
     while (p.x >= 0.5 * geometry_lattice.size.x)
	  p.x -= geometry_lattice.size.x;
     while (p.x < -0.5 * geometry_lattice.size.x)
	  p.x += geometry_lattice.size.x;
     while (p.y >= 0.5 * geometry_lattice.size.y)
	  p.y -= geometry_lattice.size.y;
     while (p.y < -0.5 * geometry_lattice.size.y)
	  p.y += geometry_lattice.size.y;
     while (p.z >= 0.5 * geometry_lattice.size.z)
	  p.z -= geometry_lattice.size.z;
     while (p.z < -0.5 * geometry_lattice.size.z)
	  p.z += geometry_lattice.size.z;
     return p;
}

const geometric_object *object_of_point_in_tree(vector3 p, geom_box_tree t,
						vector3 *shiftby,
						int *precedence)
{
     int oindex = 0;
     t = tree_search(p, t, &oindex);
     if (t) {
	  geom_box_object *gbo = t->objects + oindex;
	  *shiftby = gbo->shiftby;
	  *precedence = gbo->precedence;
	  return gbo->o;
     }
     else {
	  shiftby->x = shiftby->y = shiftby->z = 0;
	  *precedence = 0;
	  return 0;
     }
}

material_type material_of_unshifted_point_in_tree_inobject(
     vector3 p, geom_box_tree t, boolean *inobject)
{
     int oindex = 0;
     t = tree_search(p, t, &oindex);
     if (t) {
	  *inobject = 1;
	  return (t->objects[oindex].o->material);
     }
     else {
	  *inobject = 0;
	  return default_material;
     }
}

material_type material_of_point_in_tree_inobject(vector3 p, geom_box_tree t,
						 boolean *inobject)
{
     /* backwards compatibility */
     return material_of_unshifted_point_in_tree_inobject(
	  shift_to_unit_cell(p), t, inobject);
}

material_type material_of_point_in_tree(vector3 p, geom_box_tree t)
{
     boolean inobject;
     return material_of_point_in_tree_inobject(p, t, &inobject);
}

geom_box_tree geom_tree_search_next(vector3 p, geom_box_tree t, int *oindex)
{
     *oindex += 1; /* search starting at next oindex */
     return tree_search(p, t, oindex);
}

geom_box_tree geom_tree_search(vector3 p, geom_box_tree t, int *oindex)
{
     *oindex = -1; /* search all indices > -1 */
     return geom_tree_search_next(p, t, oindex);
}

/**************************************************************************/

/* convert a vector p in the given object to some coordinate
   in [0,1]^3 that is a more "natural" map of the object interior. */
vector3 to_geom_box_coords(vector3 p, geom_box_object *gbo)
{
     return to_geom_object_coords(vector3_minus(p, gbo->shiftby), *gbo->o);
}

/**************************************************************************/

void display_geom_box_tree(int indentby, geom_box_tree t)
{
     int i;

     if (!t)
	  return;
     printf("%*sbox (%g..%g, %g..%g, %g..%g)\n", indentby, "",
	    t->b.low.x, t->b.high.x,
	    t->b.low.y, t->b.high.y,
	    t->b.low.z, t->b.high.z);
     for (i = 0; i < t->nobjects; ++i) {
	  printf("%*sbounding box (%g..%g, %g..%g, %g..%g)\n", indentby+5, "",
		 t->objects[i].box.low.x, t->objects[i].box.high.x,
		 t->objects[i].box.low.y, t->objects[i].box.high.y,
		 t->objects[i].box.low.z, t->objects[i].box.high.z);
	  printf("%*sshift object by (%g, %g, %g)\n", indentby+5, "",
		 t->objects[i].shiftby.x, t->objects[i].shiftby.y,
		 t->objects[i].shiftby.z);
	  display_geometric_object_info(indentby + 5, *t->objects[i].o);
     }
     display_geom_box_tree(indentby + 5, t->t1);
     display_geom_box_tree(indentby + 5, t->t2);
}

/**************************************************************************/

/* Computing tree statistics (depth and number of nodes): */

/* helper function for geom_box_tree_stats */
static void get_tree_stats(geom_box_tree t, int *depth, int *nobjects)
{
     if (t) {
	  int d1, d2;

	  *nobjects += t->nobjects;
	  d1 = d2 = *depth + 1;
	  get_tree_stats(t->t1, &d1, nobjects);
	  get_tree_stats(t->t2, &d2, nobjects);
	  *depth = MAX(d1, d2);
     }
}

void geom_box_tree_stats(geom_box_tree t, int *depth, int *nobjects)
{
     *depth = *nobjects = 0;
     get_tree_stats(t, depth, nobjects);
}

/**************************************************************************/

#ifndef LIBCTLGEOM

vector3 get_grid_size(void)
{
     return ctl_convert_vector3_to_c(gh_call0(gh_lookup("get-grid-size")));
}

vector3 get_resolution(void)
{
     return ctl_convert_vector3_to_c(gh_call0(gh_lookup("get-resolution")));
}

void get_grid_size_n(int *nx, int *ny, int *nz)
{
     vector3 grid_size;
     grid_size = get_grid_size();
     *nx = (int) grid_size.x;
     *ny = (int) grid_size.y;
     *nz = (int) grid_size.z;
}

#endif

/**************************************************************************/

/* constructors for the geometry types (ugh, wish these
   could be automatically generated from geom.scm) */

geometric_object make_geometric_object(material_type material, vector3 center)
{
     geometric_object o;
     material_type_copy(&material, &o.material);
     o.center = center;
     o.which_subclass = GEOM GEOMETRIC_OBJECT_SELF;
     return o;
}

geometric_object make_cylinder(material_type material, vector3 center,
			       number radius, number height, vector3 axis)
{
     geometric_object o = make_geometric_object(material, center);
     o.which_subclass = GEOM CYLINDER;
     o.subclass.cylinder_data = MALLOC1(cylinder);
     CHECK(o.subclass.cylinder_data, "out of memory");
     o.subclass.cylinder_data->radius = radius;
     o.subclass.cylinder_data->height = height;
     o.subclass.cylinder_data->axis = axis;
     o.subclass.cylinder_data->which_subclass = CYL CYLINDER_SELF;
     geom_fix_object(o);
     return o;
}

geometric_object make_cone(material_type material, vector3 center,
			   number radius, number height, vector3 axis,
			   number radius2)
{
     geometric_object o = make_cylinder(material, center, radius,height, axis);
     o.subclass.cylinder_data->which_subclass = CYL CONE;
     o.subclass.cylinder_data->subclass.cone_data = MALLOC1(cone);
     CHECK(o.subclass.cylinder_data->subclass.cone_data, "out of memory");
     o.subclass.cylinder_data->subclass.cone_data->radius2 = radius2;
     return o;
}

geometric_object make_wedge(material_type material, vector3 center,
			    number radius, number height, vector3 axis,
			    number wedge_angle, vector3 wedge_start)
{
     geometric_object o = make_cylinder(material, center, radius,height, axis);
     o.subclass.cylinder_data->which_subclass = CYL WEDGE;
     o.subclass.cylinder_data->subclass.wedge_data = MALLOC1(wedge);
     CHECK(o.subclass.cylinder_data->subclass.wedge_data, "out of memory");
     o.subclass.cylinder_data->subclass.wedge_data->wedge_angle = wedge_angle;
     o.subclass.cylinder_data->subclass.wedge_data->wedge_start = wedge_start;
     geom_fix_object(o);
     return o;
}

geometric_object make_sphere(material_type material, vector3 center,
			     number radius)
{
     geometric_object o = make_geometric_object(material, center);
     o.which_subclass = GEOM SPHERE;
     o.subclass.sphere_data = MALLOC1(sphere);
     CHECK(o.subclass.sphere_data, "out of memory");
     o.subclass.sphere_data->radius = radius;
     return o;
}

geometric_object make_block(material_type material, vector3 center,
			    vector3 e1, vector3 e2, vector3 e3,
			    vector3 size)
{
     geometric_object o = make_geometric_object(material, center);
     o.which_subclass = GEOM BLOCK;
     o.subclass.block_data = MALLOC1(block);
     CHECK(o.subclass.block_data, "out of memory");
     o.subclass.block_data->e1 = e1;
     o.subclass.block_data->e2 = e2;
     o.subclass.block_data->e3 = e3;
     o.subclass.block_data->size = size;
     o.subclass.block_data->which_subclass = BLK BLOCK_SELF;
     geom_fix_object(o);
     return o;
}

geometric_object make_ellipsoid(material_type material, vector3 center,
				vector3 e1, vector3 e2, vector3 e3,
				vector3 size)
{
     geometric_object o = make_block(material, center, e1,e2,e3, size);
     o.subclass.block_data->which_subclass = BLK ELLIPSOID;
     o.subclass.block_data->subclass.ellipsoid_data = MALLOC1(ellipsoid);
     CHECK(o.subclass.block_data->subclass.ellipsoid_data, "out of memory");
     o.subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes.x
	  = 2.0 / size.x;
     o.subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes.y
	  = 2.0 / size.y;
     o.subclass.block_data->subclass.ellipsoid_data->inverse_semi_axes.z
	  = 2.0 / size.z;
     return o;
}

/***************************************************************
 * the remainder of this file is the content of `prism.c`
 * implementing geometric primitives for prisms
 *
 * prism.c -- geometry routines for prisms.
 * a prism is a planar polygon, consisting of 3 or more user-specified
 * vertices, extruded through a given thickness (the "height") in the
 * direction of a given axis.
 * most calculations are done in the "prism coordinate system",
 * in which the prism floor lies in the XY plane with centroid
 * at the origin and the prism axis is the positive Z-axis.
 * homer reid 4/2018
 *
 ***************************************************************/

/***************************************************************/
/* given coordinates of a point in the prism coordinate system,*/
/* return cartesian coordinates of that point                  */
/***************************************************************/
vector3 prism_coordinate_p2c(prism *prsm, vector3 vp)
{ return vector3_plus(prsm->centroid, matrix3x3_vector3_mult(prsm->m_p2c,vp)); }

vector3 prism_vector_p2c(prism *prsm, vector3 vp)
{ return matrix3x3_vector3_mult(prsm->m_p2c, vp); }

vector3 prism_coordinate_c2p(prism *prsm, vector3 vc)
{ return matrix3x3_vector3_mult(prsm->m_c2p, vector3_minus(vc,prsm->centroid)); }

vector3 prism_vector_c2p(prism *prsm, vector3 vc)
{ return matrix3x3_vector3_mult(prsm->m_c2p, vc); }

/***************************************************************/
/***************************************************************/
/***************************************************************/
void get_prism_bounding_box(prism *prsm, geom_box *box)
{
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;

  box->low = box->high = prism_coordinate_p2c(prsm, vertices[0]);
  int nv, fc;
  for(nv=0; nv<num_vertices; nv++)
   for(fc=0; fc<2; fc++) // 'floor,ceiling'
   { vector3 vp = vertices[nv];
     if (fc==1) vp.z=height;
     vector3 vc = prism_coordinate_p2c(prsm, vp);

     box->low.x  = fmin(box->low.x, vc.x);
     box->low.y  = fmin(box->low.y, vc.y);
     box->low.z  = fmin(box->low.z, vc.z);

     box->high.x  = fmax(box->high.x, vc.x);
     box->high.y  = fmax(box->high.y, vc.y);
     box->high.z  = fmax(box->high.z, vc.z);
   }
}

/***************************************************************/
/* find the value of s at which the line p+s*d intersects the  */
/* line segment connecting v1 to v2 (in 2 dimensions)          */
/* algorithm: solve the 2x2 linear system p+s*d = a+t*b        */
/* where s,t are scalars and p,d,a,b are 2-vectors with        */
/* a=v1, b=v2-v1                                               */
/***************************************************************/
boolean intersect_line_with_segment(double px, double py, double dx, double dy,
                                    vector3 v1, vector3 v2, double *s)
{
  double ax   = v1.x,       ay  = v1.y;
  double bx   = v2.x-v1.x,  by  = v2.y-v1.y;
  double M00  = dx,         M10 = dy;
  double M01  = -1.0*bx,    M11 = -1.0*by;
  double RHSx = ax - px,    RHSy = ay - py;
  double DetM = M00*M11 - M01*M10;
  double L2 = bx*bx + by*by; // squared length of edge
  if ( fabs(DetM) < 1.0e-10*L2 ) // d zero or nearly parallel to edge-->no intersection
   return 0;

  double t = (M00*RHSy-M10*RHSx)/DetM;
  if (s) *s = (M11*RHSx-M01*RHSy)/DetM;

  // the plumb line intersects the segment if 0<=t<=1, with t==0,1
  // corresponding to the endpoints; for our purposes we count
  // the intersection if the plumb line runs through the t==0 vertex, but
  // NOT the t==1 vertex, to avoid double-counting for complete polygons.
#define DELTAT 1.0e-6
#define TMIN (0.0)
#define TMAX (1.0-DELTAT)
  return ( ( t<TMIN || t>TMAX ) ? 0 : 1 );
}

// like the previous routine, but only count intersections if s>=0
boolean intersect_ray_with_segment(double px, double py, double dx, double dy,
                                   vector3 v1, vector3 v2, double *s)
{
  double ss;
  int status=intersect_line_with_segment(px,py,dx,dy,v1,v2,&ss);
  if (status==0 || ss<0.0)
   return 0;
  if (s) *s=ss;
  return 1;
}

/***************************************************************/
/* 2D point-in-polygon test: return 1 if the point lies within */
/* the polygon with the given vertices, 0 otherwise.           */
// method: cast a plumb line in the negative y direction from  */
/* p to infinity and count the number of edges intersected;    */
/* point lies in polygon iff this is number is odd.            */
/***************************************************************/
boolean point_in_polygon(double px, double py, vector3 *vertices, int num_vertices)
{
  double dx=0.0, dy=-1.0;
  int num_side_intersections=0;
  int nv;
  for(nv=0; nv<num_vertices; nv++)
   num_side_intersections
    +=intersect_ray_with_segment(px, py, dx, dy,
                                 vertices[nv], vertices[(nv+1)%num_vertices],
                                 0);
  return num_side_intersections%2;
}

/***************************************************************/
/* return 1 or 0 if xc lies inside or outside the prism        */
/***************************************************************/
boolean point_in_prism(prism *prsm, vector3 xc)
{
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;
  vector3 xp        = prism_coordinate_c2p(prsm, xc);
  if ( xp.z<0.0 || xp.z>prsm->height)
   return 0;
  return point_in_polygon(xp.x, xp.y, vertices, num_vertices);
}

/***************************************************************/
/* insert s into slist, which is sorted in increasing order    */
/* has maximum length 2                                        */
/***************************************************************/
int insert_s_in_list(double s, double slist[2], int length)
{
  if (length==0)
   { slist[0]=s;
     return 1;
   }
  if ( s < slist[0] )
   { slist[1]=slist[0];
     slist[0]=s;
   }
  else if (length==1 || s < slist[1] )
   slist[1]=s;

  return 2;
}

/***************************************************************/
/* compute all values of s at which the line p+s*d intersects  */
/* a prism face, retaining the first two entries in a list     */
/* of s values sorted in increasing order m                    */
/***************************************************************/
int intersect_line_with_prism(prism *prsm, vector3 p, vector3 d, double slist[2])
{
  int num_vertices  = prsm->vertices.num_items;
  vector3 *vertices = prsm->vertices.items;
  double height     = prsm->height;

  // get coords of p (line origin) and components of d (line slope)
  // in prism coordinate system
  vector3 p_prsm  = prism_coordinate_c2p(prsm,p);
  vector3 d_prsm  = prism_vector_c2p(prsm,d);

  // use length of first edge as a length scale for judging
  // lengths to be small or large
  double length_scale = vector3_norm(vector3_minus(vertices[1], vertices[0]));

  // identify intersections with prism side faces
  int num_intersections=0;
  int nv;
  for(nv=0; nv<num_vertices; nv++)
   {
     int nvp1 = nv+1; if (nvp1==num_vertices) nvp1=0;

     // first solve the in-plane portion of the problem: determine the
     // intersection of the XY-plane projection of the line with the
     // polygon edge between vertices (nv,nv+1).
     double s;
     if (!intersect_line_with_segment(p_prsm.x, p_prsm.y, d_prsm.x, d_prsm.y,
                                      vertices[nv], vertices[nvp1], &s)
        ) continue;

     // OK, we know the XY-plane projection of the line intersects the polygon edge;
     // now go back to 3D, ask for the z-coordinate at which the line intersects
     // the z-axis extrusion of the edge, and determine whether this point lies
     // inside or outside the height of the prism.
     double z_intersect = p_prsm.z + s*d_prsm.z;
     double AbsTol = 1.0e-7*length_scale;
     double z_min  = 0.0    - AbsTol;
     double z_max  = height + AbsTol;
     if ( (z_intersect<z_min) || (z_intersect>z_max) )
      continue;

     num_intersections=insert_s_in_list(s, slist, num_intersections);
   }

  // identify intersections with prism ceiling and floor faces
  double dz = d_prsm.z;
  int LowerUpper;
  if ( fabs(dz) > 1.0e-7*vector3_norm(d_prsm))
   for(LowerUpper=0; LowerUpper<2; LowerUpper++)
    { double z0    = LowerUpper ? height : 0.0;
      double s     = (z0 - p_prsm.z)/dz;
      if (!point_in_polygon(p_prsm.x + s*d_prsm.x, p_prsm.y+s*d_prsm.y, vertices, num_vertices))
       continue;
      num_intersections=insert_s_in_list(s,slist,num_intersections);
    }
  return num_intersections;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
double intersect_line_segment_with_prism(prism *prsm, vector3 p, vector3 d, double a, double b)
{
  double slist[2]={0.0, 0.0};
  if (2!=intersect_line_with_prism(prsm, p, d, slist))
   return 0.0;
  double ds = fmin(slist[1],b) - fmax(slist[0],a);
  return ds > 0.0 ? ds : 0.0;
}

/***************************************************************/
/* compute the minimum distance from p to the plane            */
/* containing o (origin) and spanned by basis vectors v1,v2    */
/* algorithm: solve the 3x3 system p-s*v3 = o + t*v1 + u*v2    */
/* where v3 = v1 x v2 and s,t,u are unknowns.                  */
/* return value is unit normal vector to plane and *s is such  */
/* that p-(*s)*v3 lies in the plane                            */
/***************************************************************/
vector3 normal_to_plane(vector3 o, vector3 v1, vector3 v2, vector3 p, double *s)
{
  vector3 v3 = unit_vector3(vector3_cross(v1, v2));
  CHECK( (vector3_norm(v3)>1.0e-6), "degenerate plane in normal_to_plane" );
  matrix3x3 M;
  M.c0 = v1;
  M.c1 = v2;
  M.c2 = v3;
  vector3 RHS = vector3_minus(p,o);
  vector3 tus = matrix3x3_vector3_mult(matrix3x3_inverse(M),RHS); // "t, u, s"
  *s = tus.z;
  return v3;
}

/***************************************************************/
/* find the face of the prism for which the normal distance    */
/* from x to the plane of that face is the shortest, then      */
/* return the normal vector to that plane.                     */
/***************************************************************/
vector3 normal_to_prism(prism *prsm, vector3 xc)
{
  vector3 centroid  = prsm->centroid;
  double height     = prsm->height;
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;

  vector3 xp   = prism_coordinate_c2p(prsm, xc);
  vector3 axis = {0,0,0}; axis.z=height;

  vector3 retval;
  double min_distance=HUGE_VAL;

  // side walls
  if (height>0.0)
   { int nv;
     for(nv=0; nv<num_vertices; nv++)
      { int nvp1 = ( nv==(num_vertices-1) ? 0 : nv+1 );
        vector3 v1 = vector3_minus(vertices[nvp1],vertices[nv]);
        vector3 v2 = axis;
        double s;
        vector3 v3 = normal_to_plane(vertices[nv], v1, v2, xp, &s);
        if (fabs(s) < min_distance)
         { min_distance = fabs(s);
           retval = v3;
         }
      }
   }

  // floor and ceiling
  int UpperLower;
  for(UpperLower=0; UpperLower<2; UpperLower++)
   { vector3 zhat={0,0,1.0};
     vector3 v1 = vector3_minus(vertices[1],vertices[0]);
     vector3 v2 = vector3_cross(zhat,v1);
     vector3 o  = {0,0,0};
     if (UpperLower) o.z = height;
     double s;
     vector3 v3 = normal_to_plane(o, v1, v2, xp, &s);
     if (fabs(s) < min_distance)
      { min_distance = fabs(s);
        retval = v3;
      }
   }

  return prism_vector_p2c(prsm, retval);
}


/***************************************************************/
/***************************************************************/
/***************************************************************/
void display_prism_info(int indentby, prism *prsm)
{
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;
  vector3 z0        = {0.0, 0.0, 1.0};
  vector3 axis      = matrix3x3_vector3_mult(prsm->m_c2p,z0);

  printf("%*s     height %g, axis (%g,%g,%g), %i vertices:\n", indentby, "",
          height, axis.x, axis.y, axis.z, num_vertices);
  matrix3x3 m_p2c = matrix3x3_inverse(prsm->m_c2p);
  int nv;
  for(nv=0; nv<num_vertices; nv++)
   { vector3 v = matrix3x3_vector3_mult(m_p2c, vertices[nv]);
     printf("%*s     {%e,%e,%e}\n",indentby,"",v.x,v.y,v.z);
   };

}

/***************************************************************/
// like vector3_equal but tolerant of small floating-point discrepancies
/***************************************************************/
int vector3_nearly_equal(vector3 v1, vector3 v2, double tolerance)
{
  return (vector3_norm( vector3_minus(v1,v2) ) <= tolerance*vector3_norm(v1));
}

/***************************************************************/
/* return the unit normal vector to the triangle with the given*/
/* vertices                                                    */
/***************************************************************/
vector3 triangle_normal(vector3 v1, vector3 v2, vector3 v3)
{
  vector3 nv=vector3_cross( vector3_minus(v2,v1), vector3_minus(v3,v1) );
  double nvnorm=vector3_norm(nv);
 // if (area) *area += 0.5*nvnorm;
  return unit_vector3(nv);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
geometric_object make_prism(material_type material,
			    const vector3 *vertices, int num_vertices,
			    double height, vector3 axis)
{
  CHECK(num_vertices>=3, "fewer than 3 vertices in make_prism");

  // compute centroid of vertices
  vector3 centroid = {0.0, 0.0, 0.0};
  int nv;
  for(nv=0; nv<num_vertices; nv++)
   centroid = vector3_plus(centroid, vertices[nv]);
  centroid = vector3_scale(1.0/((double)num_vertices), centroid);

  // make sure all vertices lie in a plane normal to the given axis
  vector3 zhat = unit_vector3(axis);
  double tolerance=1.0e-6;
  for(nv=0; nv<num_vertices; nv++)
   { int nvp1 = (nv+1) % num_vertices;
     vector3 zhatp = triangle_normal(centroid,vertices[nv],vertices[nvp1]);
     boolean axis_normal
      = (    vector3_nearly_equal(zhat, zhatp, tolerance)
          || vector3_nearly_equal(zhat, vector3_scale(-1.0,zhatp), tolerance)
        );
     CHECK(axis_normal, "axis not normal to vertex plane in make_prism");
   }

  // compute rotation matrix that operates on a vector of cartesian coordinates
  // to yield the coordinates of the same point in the prism coordinate system.
  // the prism coordinate system is a right-handed coordinate system
  // in which the prism lies in the xy plane (extrusion axis is the positive z-axis)
  // with centroid at the origin and the first edge lying on the positive x-axis.
  // note: the prism *centroid* is the center of mass of the planar vertex polygon,
  //       i.e. it is a point lying on the bottom surface of the prism.
  //       This is the origin of coordinates in the prism system.
  //       The *center* of the geometric object is the center of mass of the
  //       3D prism. So center = centroid + 0.5*zHat.
  vector3 xhat    = unit_vector3(vector3_minus(vertices[1],vertices[0]));
  vector3 yhat    = unit_vector3(vector3_cross(zhat,xhat));
  matrix3x3 m_p2c = {xhat, yhat, zhat };
  matrix3x3 m_c2p = matrix3x3_inverse(m_p2c);

  prism *prsm = MALLOC1(prism);
  CHECK(prsm, "out of memory");
  prsm->centroid  = centroid;
  prsm->height    = height;
  prsm->m_p2c     = m_p2c;
  prsm->m_c2p     = m_c2p;

  // note that the vertices stored in the prism_data structure
  // are the vertices in the *prism* coordinate system, which means
  // their z-coordinates are all zero and in principle need not be stored
  prsm->vertices.num_items = num_vertices;
  prsm->vertices.items     = (vector3 *)malloc(num_vertices*sizeof(vector3));
  for(nv=0; nv<num_vertices; nv++)
   prsm->vertices.items[nv] = prism_coordinate_c2p(prsm,vertices[nv]);

  // note the center and centroid are different!
  vector3 center = vector3_plus(centroid, vector3_scale(0.5*height,zhat) );
  geometric_object o=make_geometric_object(material, center);
  o.which_subclass=GEOM PRISM;
  o.subclass.prism_data = prsm;

  return o;
}
