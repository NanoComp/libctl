/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998, 1999, 2000, 2001, Steven G. Johnson
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

#include <math.h>

#include <ctlgeom.h>

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
	 case CYLINDER:
	      lattice_normalize(&o.subclass.cylinder_data->axis);
	      break;
	 case BLOCK:
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
	 case GEOMETRIC_OBJECT_SELF: case SPHERE:
	      break; /* these objects are fine */
     }
}

/* fix all objects in the geometry list as described in
   geom_fix_object, above */
void geom_fix_objects(void)
{
     int index;

     for (index = 0; index < geometry.num_items; ++index)
	  geom_fix_object(geometry.items[index]);
}

/**************************************************************************/

/* Return whether or not the point p (in the lattice basis) is inside
   the object o.

   Requires that the global input var geometry_lattice already be
   initialized.

   point_in_fixed_objectp additionally requires that geom_fix_object
   has been called on o (if the lattice basis is non-orthogonal).  */

boolean point_in_objectp(vector3 p, geometric_object o)
{
     geom_fix_object(o);
     return point_in_fixed_objectp(p, o);
}

boolean point_in_fixed_objectp(vector3 p, geometric_object o)
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
      number height = o.subclass.cylinder_data->height;
      if (fabs(proj) <= 0.5 * height) {
	number radius = o.subclass.cylinder_data->radius;
	if (o.subclass.cylinder_data->which_subclass == CONE)
	     radius += (proj/height + 0.5) *
		  (o.subclass.cylinder_data->subclass.cone_data->radius2
		   - radius);
	return(radius != 0.0 && vector3_dot(r,rm) - proj*proj <= radius*radius);
      }
      else
	return 0;
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

boolean point_in_periodic_objectp(vector3 p, geometric_object o)
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

/**************************************************************************/

/* Return the material type corresponding to the point p (in the lattice
   basis).  Returns default_material if p is not in any object.

   Requires that the global input vars geometry_lattice, geometry,
   dimensions, default_material and ensure_periodicity already be
   initialized. 

   Also requires that geom_fix_objects() has been called! 

   material_of_point_inobject is a variant that also returns whether
   or not the point was in any object.  */

material_type material_of_point_inobject(vector3 p, boolean *inobject)
{
     int index;
     
     *inobject = 1;
     /* loop in reverse order so that later items are given precedence: */
     for (index = geometry.num_items - 1; index >= 0; --index) {
	  if ((ensure_periodicity
	       && point_in_periodic_fixed_objectp(p, geometry.items[index]))
	      || point_in_fixed_objectp(p, geometry.items[index]))
	       return(geometry.items[index].material);
     }
     *inobject = 0;
     return default_material;
}

material_type material_of_point(vector3 p)
{
     boolean inobject;
     return material_of_point_inobject(p, &inobject);
}

/**************************************************************************/

/* Given a geometric object o, display some information about it,
   indented by indentby spaces. */

void display_geometric_object_info(int indentby, geometric_object o)
{
     geom_fix_object(o);
     printf("%*s", indentby, "");
     switch (o.which_subclass) {
	 case CYLINDER:
	      switch (o.subclass.cylinder_data->which_subclass) {
		  case CONE:
		       printf("cone");
		       break;
		  case CYLINDER_SELF:
		       printf("cylinder");
		       break;
	      }
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
	      if (o.subclass.cylinder_data->which_subclass == CONE)
		   printf("%*s     radius2 %g\n", indentby, "",
		        o.subclass.cylinder_data->subclass.cone_data->radius2);
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

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
static void geom_box_union(geom_box *bu, geom_box *b1, geom_box *b2)
{
     bu->low.x = MIN(b1->low.x, b2->low.x);
     bu->low.y = MIN(b1->low.y, b2->low.y);
     bu->low.z = MIN(b1->low.z, b2->low.z);
     bu->high.x = MAX(b1->high.x, b2->high.x);
     bu->high.y = MAX(b1->high.y, b2->high.y);
     bu->high.z = MAX(b1->high.z, b2->high.z);
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
static void get_bounding_box(geometric_object o, geom_box *box)
{
     geom_fix_object(o);

     /* initialize to empty box at the center of the object: */
     box->low = box->high = o.center;

     switch (o.which_subclass) {
	 case GEOMETRIC_OBJECT_SELF:
	      break;
	 case SPHERE:
	 {
	      /* Find the parallelepiped that the sphere inscribes.
		 The math comes out surpisingly simple--try it! */

	      number radius = o.subclass.sphere_data->radius;
	      number r1 = compute_dot_cross(geometry_lattice.basis1,
					    geometry_lattice.basis2,
					    geometry_lattice.basis3) * radius;
	      number r2 = compute_dot_cross(geometry_lattice.basis2,
					    geometry_lattice.basis3,
					    geometry_lattice.basis1) * radius;
	      number r3 = compute_dot_cross(geometry_lattice.basis3,
					    geometry_lattice.basis1,
					    geometry_lattice.basis2) * radius;
	      box->low.x -= r1;
	      box->low.y -= r2;
	      box->low.z -= r3;
	      box->high.x += r1;
	      box->high.y += r2;
	      box->high.z += r3;
	      break;
	 }
	 case CYLINDER:
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
			vector3_dot(e23, geometry_lattice.basis1));
	      
	      elen2 = vector3_dot(e31, e31);
	      eproj = vector3_dot(e31, axis);
	      r2 = fabs(sqrt(fabs(elen2 - eproj*eproj)) /
			vector3_dot(e31, geometry_lattice.basis2));

	      elen2 = vector3_dot(e12, e12);
	      eproj = vector3_dot(e12, axis);
	      r3 = fabs(sqrt(fabs(elen2 - eproj*eproj)) /
			vector3_dot(e12, geometry_lattice.basis3));

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

	      if (o.subclass.cylinder_data->which_subclass == CONE)
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
	 case BLOCK:
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
	 }
     }
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
	       free(t->objects);
	  free(t);
     }
}

/* return whether the object o, shifted by the vector shiftby,
   possibly intersects b.  Upon return, obj_b is the bounding
   box for o. */
static int object_in_box(geometric_object o, vector3 shiftby,
			 geom_box *obj_b, const geom_box *b)
{
     get_bounding_box(o, obj_b);
     geom_box_shift(obj_b, shiftby);
     return geom_boxes_intersect(obj_b, b);
}

#define CHECK(cond, s) if (!(cond)){fprintf(stderr,s "\n");exit(EXIT_FAILURE);}

static geom_box_tree new_geom_box_tree(void)
{
     geom_box_tree t;

     t = (geom_box_tree) malloc(sizeof(struct geom_box_tree_struct));
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
	  cur_partition = VEC_I(objects[i].box.high, divide_axis) + SMALL;
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
	  cur_partition = VEC_I(objects[i].box.low, divide_axis) - SMALL;
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
     t->t1->objects = (geom_box_object *) malloc(t->t1->nobjects *
						 sizeof(geom_box_object));
     CHECK(t->t1->objects, "out of memory");

     t->t2->nobjects = division_nobjects[best][1];
     t->t2->objects = (geom_box_object *) malloc(t->t2->nobjects *
						 sizeof(geom_box_object));
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
     free(t->objects);
     t->objects = NULL;

     divide_geom_box_tree(t->t1);
     divide_geom_box_tree(t->t2);
}

geom_box_tree create_geom_box_tree(void)
{
     geom_box b;
     geom_box_tree t = new_geom_box_tree();
     int i, index;

     t->b.low = vector3_scale(-0.5, geometry_lattice.size);
     t->b.high = vector3_scale(0.5, geometry_lattice.size);

     for (i = geometry.num_items - 1; i >= 0; --i) {
	  vector3 shiftby = {0,0,0};
	  if (ensure_periodicity) {
	       LOOP_PERIODIC(shiftby,
			     if (object_in_box(geometry.items[i], shiftby,
					       &b, &t->b)) ++t->nobjects);
	  }
	  else if (object_in_box(geometry.items[i], shiftby, &b, &t->b))
	       ++t->nobjects;
     }

     t->objects = (geom_box_object *) malloc(t->nobjects *
					     sizeof(geom_box_object));
     CHECK(t->objects || t->nobjects == 0, "out of memory");
	  
     for (i = geometry.num_items - 1, index = 0; i >= 0; --i) {
	  vector3 shiftby = {0,0,0};
	  if (ensure_periodicity) {
	       LOOP_PERIODIC(shiftby,
			     if (object_in_box(geometry.items[i], shiftby,
					       &b, &t->b)) {
				  t->objects[index].box = b;
				  t->objects[index].o = &geometry.items[i];
				  t->objects[index].shiftby = shiftby;
				  index++;
			     });
	  }
	  else if (object_in_box(geometry.items[i], shiftby, &b, &t->b)) {
	       t->objects[index].box = b;
	       t->objects[index].o = &geometry.items[i];
	       t->objects[index].shiftby = shiftby;
	       index++;
	  }
     }

     divide_geom_box_tree(t);
     
     return t;
}

/**************************************************************************/

/* recursively search the tree for the given point. */
static geom_box_object *find_box_object(vector3 p, geom_box_tree t)
{
     int i;
     geom_box_object *gbo;

     if (!t || !geom_box_contains_point(&t->b, p))
	  return NULL;

     for (i = 0; i < t->nobjects; ++i)
	  if (geom_box_contains_point(&t->objects[i].box, p) &&
	      point_in_fixed_objectp(vector3_minus(p, t->objects[i].shiftby),
				     *t->objects[i].o))
	       return(&t->objects[i]);
     
     gbo = find_box_object(p, t->t1);
     if (!gbo)
	  gbo = find_box_object(p, t->t2);
     return gbo;
}

/* shift p to be within the unit cell of the lattice (centered on the
   origin); p is required to be no more than one lattice constant
   away from the unit cell in any direction. */
static void shift_to_unit_cell(vector3 *p)
{
     if (p->x >= 0.5 * geometry_lattice.size.x)
	  p->x -= geometry_lattice.size.x;
     else if (p->x < -0.5 * geometry_lattice.size.x)
	  p->x += geometry_lattice.size.x;
     if (p->y >= 0.5 * geometry_lattice.size.y)
	  p->y -= geometry_lattice.size.y;
     else if (p->y < -0.5 * geometry_lattice.size.y)
	  p->y += geometry_lattice.size.y;
     if (p->z >= 0.5 * geometry_lattice.size.z)
	  p->z -= geometry_lattice.size.z;
     else if (p->z < -0.5 * geometry_lattice.size.z)
	  p->z += geometry_lattice.size.z;
}

material_type material_of_point_in_tree_inobject(vector3 p, geom_box_tree t,
						 boolean *inobject)
{
     geom_box_object *gbo;

     shift_to_unit_cell(&p);
     gbo = find_box_object(p, t);
     if (gbo) {
	  *inobject = 1;
	  return (gbo->o->material);
     }
     else {
	  *inobject = 0;
	  return default_material;
     }
}

material_type material_of_point_in_tree(vector3 p, geom_box_tree t)
{
     boolean inobject;
     return material_of_point_in_tree_inobject(p, t, &inobject);
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
