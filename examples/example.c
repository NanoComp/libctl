/**************************************************************************/

/* Example program, called by run_program in main.c, which reads the
   input variables and writes some data into the output variables. */

/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <guile/gh.h>

#include "ctl-io.h"

/**************************************************************************/

void example_do_stuff(void)
{
  int i;

  /* Just print out some data to prove that we have read the
     input variables: */

  printf("Working in %d dimensions.\n", dimensions);

  printf("\nk-points are:\n");
  for (i = 0; i < k_points.num_items; ++i)
    printf("     (%g,%g,%g)\n",
	   k_points.items[i].x, k_points.items[i].y, k_points.items[i].z);
  
  printf("\nsome geometry info:\n");
  for (i = 0; i < geometry.num_items; ++i) {
    printf("     center = (%g,%g,%g), epsilon = %g\n",
	   geometry.items[i].center.x, geometry.items[i].center.y,
	   geometry.items[i].center.z, geometry.items[i].material.epsilon);

    switch (geometry.items[i].which_subclass) {
    case CYLINDER:
      printf("          cylinder with height %g\n",
	     geometry.items[i].subclass.cylinder_data->height);
      break;
    case SPHERE:
      printf("          sphere with radius %g\n",
	     geometry.items[i].subclass.sphere_data->radius);
      break;
    case BLOCK:
      printf("          block with size (%g,%g,%g)\n",
	     geometry.items[i].subclass.block_data->size.x,
	     geometry.items[i].subclass.block_data->size.y,
	     geometry.items[i].subclass.block_data->size.z);
      break;
    case GEOMETRIC_OBJECT:
      printf("          generic geometric object\n");
      break;
    default:
      printf("          UNKNOWN OBJECT TYPE!\n");
    }
  }

  printf("\nDone writing input.  Sending data to output vars.\n");

  /* Write out some data to the output variables.  Note that we
     MUST do this.  If we leave any output variables uninitialized,
     the result is undefined. */

  if (num_runs > 1)
    destroy_output_vars(); /* we are responsible for calling this */
  
  dummy = vector3_scale(2, dummy);
  mean_dielectric = 1.23456789;
  gaps.num_items = 2;
  gaps.items = (number *) malloc(gaps.num_items * sizeof(number));
  gaps.items[0] = 3.14159;
  gaps.items[1] = 1.41421;
}
