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

/**************************************************************************/

/* Example routines, callable from Guile as defined in the example.scm
   specifications file. Read in the input variables and writes some
   data into the output variables. */

/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <guile/gh.h>

#include "ctl-io.h"
#include <ctlgeom.h>

/**************************************************************************/

/* function to display a little information about a geometric object to
   prove that we've read it in correctly. */
static void display_object_info(geometric_object obj)
{
  printf("     center = (%g,%g,%g), epsilon = %g\n",
	 obj.center.x, obj.center.y,
	 obj.center.z, obj.material.epsilon);
  
  switch (obj.which_subclass) {
  case CYLINDER:
    printf("          cylinder with height %g, axis (%g, %g, %g)\n",
	   obj.subclass.cylinder_data->height,
	   obj.subclass.cylinder_data->axis.x,
	   obj.subclass.cylinder_data->axis.y,
	   obj.subclass.cylinder_data->axis.z);
    break;
  case SPHERE:
    printf("          sphere with radius %g\n",
	   obj.subclass.sphere_data->radius);
    break;
  case BLOCK:
    printf("          block with size (%g,%g,%g)\n",
	   obj.subclass.block_data->size.x,
	   obj.subclass.block_data->size.y,
	   obj.subclass.block_data->size.z);
    printf("          projection matrix: %10.6f%10.6f%10.6f\n"
	   "                             %10.6f%10.6f%10.6f\n"
	   "                             %10.6f%10.6f%10.6f\n",
	   obj.subclass.block_data->projection_matrix.c0.x,
	   obj.subclass.block_data->projection_matrix.c1.x,
	   obj.subclass.block_data->projection_matrix.c2.x,
	   obj.subclass.block_data->projection_matrix.c0.y,
	   obj.subclass.block_data->projection_matrix.c1.y,
	   obj.subclass.block_data->projection_matrix.c2.y,
	   obj.subclass.block_data->projection_matrix.c0.z,
	   obj.subclass.block_data->projection_matrix.c1.z,
	   obj.subclass.block_data->projection_matrix.c2.z);
    break;
  case GEOMETRIC_OBJECT_SELF:
    printf("          generic geometric object\n");
    break;
  default:
    printf("          UNKNOWN OBJECT TYPE!\n");
  }
}

/* run function.  This function is callable from Scheme.  When it
   is called, the input variables are already assigned.   After
   it is called, the values assigned to the output variables are
   automatically exported to scheme. */
void run_program(void)
{
  int i, depth, nobjects;
  vector3 p;
  geom_box_tree t;

  /* Just print out some data to prove that we have read the
     input variables: */

  printf("Working in %d dimensions.\n", dimensions);

  printf("\nk-points are:\n");
  for (i = 0; i < k_points.num_items; ++i)
    printf("     (%g,%g,%g)\n",
	   k_points.items[i].x, k_points.items[i].y, k_points.items[i].z);
  
  printf("\nsome geometry info:\n");
  for (i = 0; i < geometry.num_items; ++i)
    display_object_info(geometry.items[i]);

  t = create_geom_box_tree();
  printf("\ngeometry box tree:\n");
  display_geom_box_tree(2, t);
  geom_box_tree_stats(t, &depth, &nobjects);
  printf("\ntree has depth %d and %d object nodes (vs. %d objects)\n",
	 depth, nobjects, geometry.num_items);

  p.x = 1; p.y = 0; p.z = 0;
  printf("Epsilon of (%g, %g) is %g (tree) or %g (non-tree)\n",
	 p.x, p.y,
	 material_of_point_in_tree(p, t).epsilon,
	 material_of_point(p).epsilon);

  destroy_geom_box_tree(t);

  printf("\nDone writing input.  Sending data to output vars.\n");

  /* Write out some data to the output variables.  Note that we
     MUST do this.  If we leave any output variables uninitialized,
     the result is undefined. */

  if (num_write_output_vars > 1)
    destroy_output_vars(); /* we are responsible for calling this */
  
  printf("dummy = (%g+%gi, %g+%gi, %g+%gi)\n", dummy.x.re, dummy.x.im,
	 dummy.y.re, dummy.y.im, dummy.z.re, dummy.z.im);
  dummy = make_cvector3(vector3_scale(2, cvector3_re(dummy)),
			vector3_scale(3, cvector3_im(dummy)));
  mean_dielectric = 1.23456789;
  gaps.num_items = 2;
  gaps.items = (number *) malloc(gaps.num_items * sizeof(number));
  gaps.items[0] = 3.14159;
  gaps.items[1] = 1.41421;
}

/* Another function callable from Scheme. This function does not
   use the input/output variables, but passes information in
   explicitely through its parameter and out through its return
   value.

   In a real program, this might return the fraction of the field
   energy in the given object. */
number energy_in_object(geometric_object obj)
{
  printf("Computing power in object.\n");
  display_object_info(obj);
  printf("Returning 0.123456.\n");
  return 0.123456;
}

/* A function to test passing and returning list parameters
   to/from Scheme: */
vector3_list list_func_test(number x, integer_list s, vector3 v)
{
     vector3_list vout;
     int i;

     vout.num_items = s.num_items;
     vout.items = (vector3*) malloc(sizeof(vector3) * vout.num_items);
     for (i = 0; i < vout.num_items; ++i)
	  vout.items[i] = vector3_scale(s.items[i] * x, v);
     return vout;
}

/* return func(arg), where func is a Scheme function returning a number. */
number function_func(function func, number arg)
{
     return 
	  ctl_convert_number_to_c(
	       gh_call1(func, ctl_convert_number_to_scm(arg)));
}
