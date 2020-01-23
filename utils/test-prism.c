/* libctl: flexible Guile-based control files for scientific software
 * Copyright (C) 1998-2019 Massachusetts Institute of Technology and Steven G. Johnson
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

/************************************************************************/
/* test-prism.c: unit test for prisms in libctlgeom                     */
/* homer reid 5/2018                                                    */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "ctlgeom.h"

vector3 normal_to_plane(vector3 o, vector3 v1, vector3 v2, vector3 p, double *min_distance);
double min_distance_to_line_segment(vector3 p, vector3 v1, vector3 v2);
boolean point_in_or_on_prism(prism *prsm, vector3 xc, boolean include_boundaries);

#define K_PI 3.141592653589793238462643383279502884197

// routine from geom.c that rotates the coordinate of a point
// from the prism coordinate system to the cartesian coordinate system
vector3 prism_coordinate_p2c(prism *prsm, vector3 vp);
vector3 prism_coordinate_c2p(prism *prsm, vector3 vc);
vector3 prism_vector_p2c(prism *prsm, vector3 vp);
vector3 prism_vector_c2p(prism *prsm, vector3 vc);

/***************************************************************/
/* utility routines for writing points, lines, quadrilaterals  */
/*  to text files for viewing in e.g. gnuplot                  */
/***************************************************************/
void GPPoint(FILE *f, vector3 v, prism *prsm) {
  if (prsm) v = prism_coordinate_p2c(prsm, v);
  fprintf(f, "%e %e %e \n\n\n", v.x, v.y, v.z);
}

void GPLine(FILE *f, vector3 v, vector3 l, prism *prsm) {
  if (prsm) {
    v = prism_coordinate_p2c(prsm, v);
    l = prism_vector_p2c(prsm, l);
  }
  fprintf(f, "%e %e %e \n", v.x, v.y, v.z);
  fprintf(f, "%e %e %e \n\n\n", v.x + l.x, v.y + l.y, v.z + l.z);
}

void GPQuad(FILE *f, vector3 v, vector3 l1, vector3 l2, prism *prsm) {
  if (prsm) {
    v = prism_coordinate_p2c(prsm, v);
    l1 = prism_vector_p2c(prsm, l1);
    l2 = prism_vector_p2c(prsm, l2);
  }
  fprintf(f, "%e %e %e \n", v.x, v.y, v.z);
  fprintf(f, "%e %e %e \n", v.x + l1.x, v.y + l1.y, v.z + l1.z);
  fprintf(f, "%e %e %e \n", v.x + l1.x + l2.x, v.y + l1.y + l2.y, v.z + l1.z + l2.z);
  fprintf(f, "%e %e %e \n", v.x + l2.x, v.y + l2.y, v.z + l2.z);
  fprintf(f, "%e %e %e \n\n\n", v.x, v.y, v.z);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void my_get_prism_bounding_box(prism *prsm, geom_box *box) {
  vector3 *vertices_bottom = prsm->vertices_bottom_p.items;
  int num_vertices = prsm->vertices_bottom_p.num_items;
  double height = prsm->height;

  box->low = box->high = prism_coordinate_p2c(prsm, vertices_bottom[0]);
  int nv, fc;
  for (nv = 0; nv < num_vertices; nv++)
    for (fc = 0; fc < 2; fc++) // 'floor,ceiling'
    {
      vector3 vp = vertices_bottom[nv];
      if (fc == 1) vp.z = height;
      vector3 vc = prism_coordinate_p2c(prsm, vp);

      box->low.x = fmin(box->low.x, vc.x);
      box->low.y = fmin(box->low.y, vc.y);
      box->low.z = fmin(box->low.z, vc.z);

      box->high.x = fmax(box->high.x, vc.x);
      box->high.y = fmax(box->high.y, vc.y);
      box->high.z = fmax(box->high.z, vc.z);
    }
}

static vector3 make_vector3(double x, double y, double z) {
  vector3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

/************************************************************************/
/* return a uniform random number in [a,b] */
/************************************************************************/
static double urand(double a, double b) { return a + (b - a) * (rand() / ((double)RAND_MAX)); }

static double drand() { return urand(0.0, 1.0); }

/************************************************************************/
/* random point uniformly distributed over a parallelepiped             */
/************************************************************************/
vector3 random_point_in_box(vector3 min_corner, vector3 max_corner) {
  return make_vector3(urand(min_corner.x, max_corner.x), urand(min_corner.y, max_corner.y),
                      urand(min_corner.z, max_corner.z));
}

/************************************************************************/
/* random point uniformly distributed over a planar polygon             */
/*  (all z coordinates are 0)                                           */
/************************************************************************/
vector3 random_point_in_polygon(prism *prsm) {
  // randomly choose a vertex and generate random point within the triangle
  // formed by that vertex, the next vertex, and the centroid
  vector3 *vertices_bottom = prsm->vertices_bottom.items;
  int num_vertices = prsm->vertices_bottom.num_items;
  int which_vertex = rand() % num_vertices;
  vector3 v0 = {0, 0, 0};
  vector3 v1 = vertices_bottom[which_vertex];
  vector3 v2 = vertices_bottom[(which_vertex + 1) % num_vertices];
  double xi = urand(0.0, 1.0), eta = urand(0.0, 1.0 - xi);
  return vector3_plus(vector3_scale(xi, vector3_minus(v1, v0)),
                      vector3_scale(eta, vector3_minus(v2, v0)));
}

/************************************************************************/
/* random point uniformly distributed over the surface of a prism       */
/************************************************************************/
vector3 random_point_on_prism(geometric_object o) {
  prism *prsm = o.subclass.prism_data;
  vector3 *vertices_bottom = prsm->vertices_bottom_p.items;
  int num_vertices = prsm->vertices_bottom_p.num_items;
  double height = prsm->height;

  // choose a face
  int num_faces = num_vertices + 2;
  int which_face = rand() % num_faces;
  if (which_face < num_vertices) // side face
  {
    vector3 min_corner = vertices_bottom[which_face];
    vector3 max_corner = vertices_bottom[(which_face + 1) % num_vertices];
    max_corner.z = height;
    return random_point_in_box(prism_coordinate_p2c(prsm, min_corner),
                               prism_coordinate_p2c(prsm, max_corner));
  }
  else // floor or ceiling
  {
    vector3 p = random_point_in_polygon(prsm);
    if (which_face == num_faces - 1) p.z = height;
    return prism_coordinate_p2c(prsm, p);
  }
}

/************************************************************************/
/* random unit vector with direction uniformly distributed over unit sphere*/
/************************************************************************/
vector3 random_unit_vector3() {
  double cos_theta = urand(0.0, 1.0), sin_theta = sqrt(1.0 - cos_theta * cos_theta);
  double phi = urand(0.0, 2.0 * K_PI);
  return make_vector3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}

/***************************************************************/
/* write prism vertices and edges to text file.                */
/* after running this routine to produce a file named MyFile,  */
/* the prism may be plotted in gnuplot like this:              */
/* gnuplot> splot 'MyFile' u 1:2:3 w lp pt 7 ps 1              */
/***************************************************************/
void prism2gnuplot(prism *prsm, char *filename) {
  int num_vertices = prsm->vertices_bottom_p.num_items;
  double height = prsm->height;
  vector3_list vertices_bottom;
  vertices_bottom.num_items = num_vertices;
  vertices_bottom.items = (vector3 *)malloc(num_vertices * sizeof(vector3));
  memcpy(vertices_bottom.items, prsm->vertices_bottom_p.items, num_vertices * sizeof(vector3));
  vector3_list vertices_top;
  vertices_top.num_items = num_vertices;
  vertices_top.items = (vector3 *)malloc(num_vertices * sizeof(vector3));
  int nv;
  for (nv = 0; nv < num_vertices; nv++) {
    vertices_top.items[nv] = vector3_plus(prsm->vertices_bottom_p.items[nv], prsm->top_polygon_diff_vectors_p.items[nv]);
  }

  FILE *f = fopen(filename, "w");
  for (nv = 0; nv < num_vertices; nv++) {
    vector3 vap = vertices_bottom.items[nv];
    vap.z = 0.0;
    vector3 vbp = vertices_top.items[nv];
    vbp.z = height;
    vector3 vcp = vertices_top.items[(nv + 1) % num_vertices];
    vcp.z = height;
    vector3 vdp = vertices_bottom.items[(nv + 1) % num_vertices];
    vdp.z = 0.0;
    vector3 vac = prism_coordinate_p2c(prsm, vap);
    vector3 vbc = prism_coordinate_p2c(prsm, vbp);
    vector3 vcc = prism_coordinate_p2c(prsm, vcp);
    vector3 vdc = prism_coordinate_p2c(prsm, vdp);

    fprintf(f, "%e %e %e \n", vac.x, vac.y, vac.z);
    fprintf(f, "%e %e %e \n", vbc.x, vbc.y, vbc.z);
    fprintf(f, "%e %e %e \n", vcc.x, vcc.y, vcc.z);
    fprintf(f, "%e %e %e \n", vdc.x, vdc.y, vdc.z);
    fprintf(f, "%e %e %e \n", vac.x, vac.y, vac.z);
    fprintf(f, "\n\n");
  }
  fclose(f);
}

/***************************************************************/
/* write prism vertices and edges to GMSH geometry (.geo) file */
/***************************************************************/
void prism2gmsh(prism *prsm, char *filename) {
  vector3 *vertices_bottom = prsm->vertices_bottom_p.items;
  int num_vertices = prsm->vertices_bottom_p.num_items;
  double height = prsm->height;
  vector3 zhat = prsm->m_p2c.c2;
  vector3 axis = vector3_scale(height, zhat);

  FILE *f = fopen(filename, "w");
  int nv;
  for (nv = 0; nv < num_vertices; nv++) {
    vector3 vp = vertices_bottom[nv];
    vector3 vc = prism_coordinate_p2c(prsm, vp);
    fprintf(f, "Point(%i)={%e, %e, %e};\n", nv, vc.x, vc.y, vc.z);
  }
  for (nv = 0; nv < num_vertices; nv++)
    fprintf(f, "Line(%i)={%i, %i};\n", nv, nv, (nv + 1) % num_vertices);
  fprintf(f, "Line Loop(0)={0");
  for (nv = 1; nv < num_vertices; nv++)
    fprintf(f, ",%i", nv);
  fprintf(f, "};\n");
  fprintf(f, "Plane Surface(0)={0};\n");
  fprintf(f, "Extrude { %e,%e,%e } { Surface{0}; }\n", height * zhat.x, height * zhat.y,
          height * zhat.z);
  fclose(f);
}

/* "standardize" a vector for vector comparisons up to normalization and sign flip */
double sgn(double x) { return x >= 0.0 ? 1.0 : -1.0; }

vector3 standardize(vector3 v) {
  vector3 sv = unit_vector3(v);
  double sign = (sv.z != 0.0 ? sgn(sv.z) : sv.y != 0.0 ? sgn(sv.y) : sgn(sv.x));
  return vector3_scale(sign, sv);
}

/************************************************************************/
/* first unit test: check inclusion of randomly-generated points        */
/************************************************************************/
int test_point_inclusion(geometric_object the_block, geometric_object the_prism, int num_tests,
                         int write_log) {
  vector3 size = the_block.subclass.block_data->size;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f = write_log ? fopen("/tmp/test-prism.points", "w") : 0;
  int num_failed = 0, num_adjusted = 0, n;
  for (n = 0; n < num_tests; n++) {
    vector3 p = random_point_in_box(min_corner, max_corner);
    boolean in_block = point_in_objectp(p, the_block);
    boolean in_prism = point_in_objectp(p, the_prism);

    if (in_block != in_prism) {
      // retry with boundary exclusion/inclusion reversed
      boolean libctl_include_boundaries = 1;
      char *s = getenv("LIBCTL_EXCLUDE_BOUNDARIES");
      if (s && s[0] == '1') libctl_include_boundaries = 0;
      in_prism =
          point_in_or_on_prism(the_prism.subclass.prism_data, p, 1 - libctl_include_boundaries);
      if (in_block == in_prism) num_adjusted++;
    }

    if (in_block != in_prism) num_failed++;
    if (f) fprintf(f, "%i %i %e %e %e \n", in_block, in_prism, p.x, p.y, p.z);
  }
  if (f) fclose(f);

  printf("point inclusion: %i/%i points failed (%i adjusted)\n", num_failed, num_tests,
         num_adjusted);
  return num_failed;
}

/************************************************************************/
/* second unit test: check calculation of normals to objects            */
/************************************************************************/
#define PFACE 0.1
int test_normal_to_object(geometric_object the_block, geometric_object the_prism, int num_tests,
                          int write_log) {
  vector3 size = the_block.subclass.block_data->size;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f = write_log ? fopen("/tmp/test-prism.normals", "w") : 0;

  int num_failed = 0;
  double tolerance = 1.0e-6;

  int n;
  for (n = 0; n < num_tests; n++) {
    // with probability PFACE, generate random base point lying on one
    //  of the 6 faces of the prism.
    // with probability 1-PFACE, generate random base point lying in the
    //  extended volume (2x volume of block)
    vector3 p = (urand(0.0, 1.0) < PFACE) ? random_point_on_prism(the_prism)
                                          : random_point_in_box(min_corner, max_corner);

    vector3 nhat_block = standardize(normal_to_object(p, the_block));
    vector3 nhat_prism = standardize(normal_to_object(p, the_prism));
    if (!vector3_nearly_equal(nhat_block, nhat_prism, tolerance)) num_failed++;

    if (f)
      fprintf(f, "%e %e %e %e %e %e %e %e %e %i\n\n\n", p.x, p.y, p.z, nhat_block.x, nhat_block.y,
              nhat_block.z, nhat_prism.x, nhat_prism.y, nhat_prism.z,
              vector3_nearly_equal(nhat_block, nhat_prism, tolerance));
  }
  if (f) fclose(f);

  printf("%i/%i normals failed\n", num_failed, num_tests);
  return num_failed;
}

/************************************************************************/
/* third unit test: check-line segment intersections                   */
/************************************************************************/
int test_line_segment_intersection(geometric_object the_block, geometric_object the_prism,
                                   int num_tests, int write_log) {
  vector3 size = the_block.subclass.block_data->size;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f = write_log ? fopen("/tmp/test-prism.segments", "w") : 0;

  int num_failed = 0;
  int n;
  for (n = 0; n < num_tests; n++) {
    // randomly generated base point within enlarged bounding box
    vector3 p = random_point_in_box(min_corner, max_corner);
    vector3 d = random_unit_vector3();
    double a = urand(0.0, 1.0);
    double b = urand(0.0, 1.0);

    double sblock = intersect_line_segment_with_object(p, d, the_block, a, b);
    double sprism = intersect_line_segment_with_object(p, d, the_prism, a, b);
    if (fabs(sblock - sprism) > 1.0e-6 * fmax(fabs(sblock), fabs(sprism))) num_failed++;

    if (f) {
      int success = fabs(sblock - sprism) <= 1.0e-6 * fmax(fabs(sblock), fabs(sprism));
      fprintf(f, " %e %e %s\n", sblock, sprism, success ? "success" : "fail");
      if (success == 0) {
        fprintf(f, "#%e %e %e %e %e %e %e %e\n", p.x, p.y, p.z, d.x, d.y, d.z, a, b);
        fprintf(f, "%e %e %e\n%e %e %e\n%e %e %e\n", p.x, p.y, p.z, p.x + a * d.x, p.y + a * d.y,
                p.z + a * d.z, p.x + b * d.x, p.y + b * d.y, p.z + b * d.z);
      }
      fprintf(f, "\n");
    }
  }
  if (f) fclose(f);

  printf("%i/%i segments failed\n", num_failed, num_tests);
  return num_failed;
}

/************************************************************************/
/* fourth unit test: check of point in polygon test with slanted H     */
/************************************************************************/
int test_point_in_polygon(int write_log) {
  // make array of test points that should always pass
  vector3 pass[5];
  pass[0] = make_vector3(0.3, 0.5, 0.0);
  pass[1] = make_vector3(0.4, 0.4, 0.0);
  pass[2] = make_vector3(0.5, 0.7, 0.0);
  pass[3] = make_vector3(0.5, 0.5, 0.0);
  pass[4] = make_vector3(0.5, 0.3, 0.0);
  
  // make array of test points that should always pass
  vector3 fail[5];
  fail[0] = make_vector3(0.2, 0.2, 0.0);
  fail[1] = make_vector3(0.3, 0.3, 0.0);
  fail[2] = make_vector3(0.4, 0.6, 0.0);
  fail[3] = make_vector3(0.6, 0.4, 0.0);
  fail[4] = make_vector3(0.7, 0.7, 0.0);

  // make array of nodes for the test polygon (an H slanted by 45 degrees)
  int num_nodes = 12;
  vector3 nodes[num_nodes];
  nodes[0] = make_vector3(0.5, 0.2, 0.0);
  nodes[1] = make_vector3(0.6, 0.3, 0.0);
  nodes[2] = make_vector3(0.5, 0.4, 0.0);
  nodes[3] = make_vector3(0.6, 0.5, 0.0);
  nodes[4] = make_vector3(0.7, 0.4, 0.0);
  nodes[5] = make_vector3(0.8, 0.5, 0.0);
  nodes[6] = make_vector3(0.5, 0.8, 0.0);
  nodes[7] = make_vector3(0.4, 0.7, 0.0);
  nodes[8] = make_vector3(0.5, 0.6, 0.0);
  nodes[9] = make_vector3(0.4, 0.5, 0.0);
  nodes[10] = make_vector3(0.3, 0.6, 0.0);
  nodes[11] = make_vector3(0.2, 0.5, 0.0);
  
  FILE *f = write_log ? fopen("/tmp/test-prism.point-in-polygon", "w") : 0;
  
  boolean all_points_success = 1;
  boolean include_boundaries = 1;
  int i;
  for (i = 0; i < 5; i++) {
    boolean local_success = node_in_or_on_polygon(pass[i], nodes, num_nodes, include_boundaries);
    if (!local_success) {
    	all_points_success = 0;
    }
    if (f) {
      fprintf(f, "%f %f %i\n", pass[i].x, pass[i].y, local_success);
    }
  }
  for (i = 0; i < 5; i++) {
    boolean local_success = !node_in_or_on_polygon(fail[i], nodes, num_nodes, include_boundaries);
    if (!local_success) {
      all_points_success = 0;
    }
    if (f) {
      fprintf(f, "%f %f %i\n", pass[i].x, pass[i].y, local_success);
    }
  }

  if (f) {
    if (all_points_success) {
      printf("all test points for slanted H pass\n");
    }
    else {
	  printf("one or more test points for slanted H fail\n");
    }
    fclose(f);
  }
  
  int num_failed;
  if (all_points_success) {
	num_failed = 0;
    printf("all test points for slanted H pass\n");
  }
  else {
	num_failed = 1;
	printf("one or more test points for slanted H fail\n");
  }
  
  return num_failed;
}

/************************************************************************/
/* fifth unit test: saves a prism with normal sidewall angle and a      */
/* prism with the same base polygon with non-normal sidewall angle to   */
/* separate GNU plot files                                              */
/************************************************************************/
int test_sidewall_prisms_to_gnuplot() {
  void *m = NULL;

  int num_nodes_square = 4;
  vector3 nodes_square[num_nodes_square];
  nodes_square[0] = make_vector3(-1.0, -1.0, 0.0);
  nodes_square[1] = make_vector3(-1.0, 1.0, 0.0);
  nodes_square[2] = make_vector3(1.0, 1.0, 0.0);
  nodes_square[3] = make_vector3(1.0, -1.0, 0.0);

  double height_square = 10;
  vector3 zhat = make_vector3(0, 0, 1);

  double normal_sidewall = 0;
  geometric_object square_normal_sidewall_geom_object = make_prism(m, nodes_square, num_nodes_square, height_square, zhat, normal_sidewall);
  prism *square_normal_sidewall_prism = square_normal_sidewall_geom_object.subclass.prism_data;

  double one_degree_sidewall = 1.0 * 2 * K_PI / 360.0;
  geometric_object square_one_degree_sidewall_geom_object = make_prism(m, nodes_square, num_nodes_square, height_square, zhat, one_degree_sidewall);
  prism *square_one_degree_sidewall_prism = square_one_degree_sidewall_geom_object.subclass.prism_data;

  prism2gnuplot(square_normal_sidewall_prism, "square_normal_sidewall_gnu_plot.dat");
  prism2gnuplot(square_one_degree_sidewall_prism, "square_one_degree_sidewall_gnu_plot.dat");

  ctl_printf("The volume of the square-based prism with a normal sidewall angle is %e\n", geom_object_volume(square_normal_sidewall_geom_object));
  ctl_printf("The volume of the square-based prism with a 1-degree sidewall angle is %e\n", geom_object_volume(square_one_degree_sidewall_geom_object));

  int num_nodes_octagon_c = 16;
  vector3 nodes_octagon_c[num_nodes_octagon_c];
  nodes_octagon_c[0]  = make_vector3(114.905, 88.7434, 0.0);
  nodes_octagon_c[1]  = make_vector3(88.7434, 114.905, 0.0);
  nodes_octagon_c[2]  = make_vector3(51.7447, 114.905, 0.0);
  nodes_octagon_c[3]  = make_vector3(25.5827, 88.7434, 0.0);
  nodes_octagon_c[4]  = make_vector3(25.5827, 51.7447, 0.0);
  nodes_octagon_c[5]  = make_vector3(51.7447, 25.5827, 0.0);
  nodes_octagon_c[6]  = make_vector3(88.7434, 25.5827, 0.0);
  nodes_octagon_c[7]  = make_vector3(114.905, 51.7447, 0.0);
  nodes_octagon_c[8]  = make_vector3(140.488, 41.1477, 0.0);
  nodes_octagon_c[9]  = make_vector3(99.3401, 0.0, 0.0);
  nodes_octagon_c[10] = make_vector3(41.1477, 0.0, 0.0);
  nodes_octagon_c[11] = make_vector3(0.0, 41.1477, 0.0);
  nodes_octagon_c[12] = make_vector3(0.0, 99.3401, 0.0);
  nodes_octagon_c[13] = make_vector3(41.1477, 140.488, 0.0);
  nodes_octagon_c[14] = make_vector3(99.3401, 140.488, 0.0);
  nodes_octagon_c[15] = make_vector3(140.488, 99.3401, 0.0);

  double height_octagon_c = 127;

  geometric_object octagon_c_normal_sidewall_geom_object = make_prism(m, nodes_octagon_c, num_nodes_octagon_c, height_octagon_c, zhat, normal_sidewall);
  prism *octagon_c_normal_sidewall_prism = octagon_c_normal_sidewall_geom_object.subclass.prism_data;

  double two_half_degree_sidewall = 2.5 * 2 * K_PI / 360.0;
  geometric_object octagon_c_two_half_degree_sidewall_geom_object = make_prism(m, nodes_octagon_c, num_nodes_octagon_c, height_octagon_c, zhat, two_half_degree_sidewall);
  prism *octagon_c_two_half_degree_sidewall_prism = octagon_c_two_half_degree_sidewall_geom_object.subclass.prism_data;

  prism2gnuplot(octagon_c_normal_sidewall_prism, "octagon_c_normal_sidewall_gnu_plot.dat");
  prism2gnuplot(octagon_c_two_half_degree_sidewall_prism, "octagon_c_two_half_degree_sidewall_gnu_plot.dat");

  ctl_printf("The volume of the prism with the concave octagonal c shape base with a normal sidewall angle is %e\n", geom_object_volume(octagon_c_normal_sidewall_geom_object));
  ctl_printf("The volume of the prism with the concave octagonal c shape base with a 2.5-degree sidewall angle is %e\n", geom_object_volume(octagon_c_two_half_degree_sidewall_geom_object));


  return 0;
}

/***************************************************************/
/* unit tests: create the same parallelepiped two ways (as a   */
/* block and as a prism) and verify that geometric primitives  */
/* give identical results                                      */
/***************************************************************/
#define NUMPTS 10000
#define NUMLINES 1000

#define LX 0.5
#define LY 1.0
#define LZ 1.5

int run_unit_tests() {
  void *m = NULL;
  vector3 c = {0, 0, 0};
  vector3 xhat = make_vector3(1, 0, 0);
  vector3 yhat = make_vector3(0, 1, 0);
  vector3 zhat = make_vector3(0, 0, 1);
  vector3 size = make_vector3(LX, LY, LZ);

  vector3 v[4];
  v[0].x = -0.5 * LX;
  v[0].y = -0.5 * LY;
  v[0].z = -0.5 * LZ;
  v[1].x = +0.5 * LX;
  v[1].y = -0.5 * LY;
  v[1].z = -0.5 * LZ;
  v[2].x = +0.5 * LX;
  v[2].y = +0.5 * LY;
  v[2].z = -0.5 * LZ;
  v[3].x = -0.5 * LX;
  v[3].y = +0.5 * LY;
  v[3].z = -0.5 * LZ;

  geometric_object the_block = make_block(m, c, xhat, yhat, zhat, size);
  geometric_object the_prism = make_prism(m, v, 4, LZ, zhat, 0.0);

  /***************************************************************/
  /* with probability P_SHIFT, shift the centers of both block   */
  /* and prism by a random displacement vector                   */
  /***************************************************************/
#define P_SHIFT 0.75
  if (urand(0.0, 1.0) < P_SHIFT) {
    vector3 shift = vector3_scale(urand(0.0, 1.0), random_unit_vector3());
    the_block.center = vector3_plus(the_block.center, shift);
    the_prism.center = vector3_plus(the_prism.center, shift);
  }

  char *s = getenv("LIBCTL_TEST_PRISM_LOG");
  int write_log = (s && s[0] == '1') ? 1 : 0;

  if (write_log) prism2gnuplot(the_prism.subclass.prism_data, "/tmp/test-prism.prism");

  int num_failed_1 = test_point_inclusion(the_block, the_prism, NUMPTS, write_log);
  // 20180712 disabling this test because the new implementation of normal_to_object
  //          for prisms is actually more accurate than the implementation for blocks,
  //          although the distinction is only significant in cases where it is irrelevant
  int num_failed_2 = 0; // test_normal_to_object(the_block, the_prism, NUMLINES, write_log);
  int num_failed_3 = test_line_segment_intersection(the_block, the_prism, NUMLINES, write_log);
  int num_failed_4 = test_point_in_polygon(write_log);
  int num_failed_5 = test_sidewall_prisms_to_gnuplot();

  return num_failed_1 + num_failed_2 + num_failed_3 + num_failed_4 + num_failed_5;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void print_usage(char *msg, int print_usage) {
  if (!msg) fprintf(stderr, "%s\n", msg);
  if (print_usage) {
    printf("usage: \n");
    printf(" --vertexfile MyVertices\n");
    printf(" --height     height\n");
    printf(" --axis       x y z\n");
    printf("\n");
    printf(" --point      x y z\n");
    printf(" --dir        x y z\n");
    printf(" --a          a\n");
    printf(" --b          b\n");
  }
  exit(1);
}

void quit(char *msg) { print_usage(msg, 0); }

void usage(char *msg) { print_usage(msg, 1); }

/************************************************************************/
/************************************************************************/
/************************************************************************/
int main(int argc, char *argv[]) {
  srand(time(NULL));
  geom_initialize();

  if (argc <= 1) // if no arguments, run unit tests
    return run_unit_tests();

  /***************************************************************/
  /* process arguments *******************************************/
  /***************************************************************/
  char *vertexfile = 0;
  vector3 axis = {0, 0, 1};
  double height = 1.5;
  vector3 test_point = {0, 0, 0};
  vector3 test_dir = {0, 0, 1};
  double a = 0.2, b = 0.3;
  int narg;
  for (narg = 1; narg < argc - 1; narg++) {
    if (!strcmp(argv[narg], "--vertexfile"))
      vertexfile = argv[++narg];
    else if (!strcmp(argv[narg], "--axis")) {
      if (narg + 3 >= argc) usage("too few arguments to --axis");
      sscanf(argv[narg + 1], "%le", &(axis.x));
      sscanf(argv[narg + 2], "%le", &(axis.y));
      sscanf(argv[narg + 3], "%le", &(axis.z));
      narg += 3;
    }
    else if (!strcmp(argv[narg], "--point")) {
      if (narg + 3 >= argc) usage("too few arguments to --point");
      sscanf(argv[narg + 1], "%le", &(test_point.x));
      sscanf(argv[narg + 2], "%le", &(test_point.y));
      sscanf(argv[narg + 3], "%le", &(test_point.z));
      narg += 3;
    }
    else if (!strcmp(argv[narg], "--line")) {
      if (narg + 6 >= argc) usage("too few arguments to --line");
      vector3 v1, v2;
      sscanf(argv[narg + 1], "%le", &(v1.x));
      sscanf(argv[narg + 2], "%le", &(v1.y));
      sscanf(argv[narg + 3], "%le", &(v1.z));
      sscanf(argv[narg + 4], "%le", &(v2.x));
      sscanf(argv[narg + 5], "%le", &(v2.y));
      sscanf(argv[narg + 6], "%le", &(v2.z));
      printf("Min distance=%e\n", min_distance_to_line_segment(test_point, v1, v2));
      narg += 6;
    }
    else if (!strcmp(argv[narg], "--dir")) {
      if (narg + 3 >= argc) usage("too few arguments to --lineseg");
      sscanf(argv[narg + 1], "%le", &(test_dir.x));
      sscanf(argv[narg + 2], "%le", &(test_dir.y));
      sscanf(argv[narg + 3], "%le", &(test_dir.z));
      narg += 3;
    }
    else if (!strcmp(argv[narg], "--height"))
      sscanf(argv[++narg], "%le", &height);
    else if (!strcmp(argv[narg], "--a"))
      sscanf(argv[++narg], "%le", &a);
    else if (!strcmp(argv[narg], "--b"))
      sscanf(argv[++narg], "%le", &b);
    else
      usage("unknown argument");
  }
  if (!vertexfile) usage("no --vertexfile specified");

  /***************************************************************/
  /* read vertices from vertex file and create prism *************/
  /***************************************************************/
  vector3 *vertices_bottom = 0;
  int num_vertices = 0;
  FILE *f = fopen(vertexfile, "r");
  if (!f) usage("could not open vertexfile");
  char Line[100];
  int LineNum = 0;
  while (fgets(Line, 100, f)) {
    if (Line[0] == '\n' || Line[0] == '#') continue;
    num_vertices++;
    vector3 v;
    if (3 != sscanf(Line, "%le %le %le\n", &(v.x), &(v.y), &(v.z))) {
      fprintf(stderr, "bad vertex on line %i of %s", num_vertices, vertexfile);
      exit(1);
    }
    vertices_bottom = (vector3 *)realloc(vertices_bottom, num_vertices * sizeof(vector3));
    vertices_bottom[num_vertices - 1] = v;
  }
  fclose(f);

  geometric_object the_prism = make_prism(NULL, vertices_bottom, num_vertices, height, axis, 0.0);
  prism *prsm = the_prism.subclass.prism_data;
  prism2gmsh(prsm, "test-prism.pp");
  prism2gnuplot(prsm, "test-prism.gp");
  f = fopen("test-point.gp", "w");
  fprintf(f, "%e %e %e\n", test_point.x, test_point.y, test_point.z);
  fclose(f);
  printf("Wrote prism description to GNUPLOT file test-prism.gp.\n");
  printf("Wrote prism description to GMSH file test-prism.geo.\n");

  geom_box prism_box;
  my_get_prism_bounding_box(prsm, &prism_box);
  f = fopen("test-prism-bb.gp", "w");
  fprintf(f, "%e %e %e\n", prism_box.low.x, prism_box.low.y, prism_box.low.z);
  fprintf(f, "%e %e %e\n", prism_box.high.x, prism_box.low.y, prism_box.low.z);
  fprintf(f, "%e %e %e\n", prism_box.high.x, prism_box.high.y, prism_box.low.z);
  fprintf(f, "%e %e %e\n", prism_box.low.x, prism_box.high.y, prism_box.low.z);
  fprintf(f, "%e %e %e\n\n\n", prism_box.low.x, prism_box.low.y, prism_box.low.z);

  fprintf(f, "%e %e %e\n", prism_box.low.x, prism_box.low.y, prism_box.high.z);
  fprintf(f, "%e %e %e\n", prism_box.high.x, prism_box.low.y, prism_box.high.z);
  fprintf(f, "%e %e %e\n", prism_box.high.x, prism_box.high.y, prism_box.high.z);
  fprintf(f, "%e %e %e\n", prism_box.low.x, prism_box.high.y, prism_box.high.z);
  fprintf(f, "%e %e %e\n\n\n", prism_box.low.x, prism_box.low.y, prism_box.high.z);
  printf("Wrote bounding box to GNUPLOT file test-prism-bb.gp.\n");

  /***************************************************************/
  /* test point inclusion, normal to object, and line-segment    */
  /* intersection with specified data                            */
  /***************************************************************/
  boolean in_prism = point_in_objectp(test_point, the_prism);
  vector3 nhat = normal_to_object(test_point, the_prism);
  double s = intersect_line_segment_with_object(test_point, test_dir, the_prism, a, b);
  printf("point {%e,%e,%e}: \n", test_point.x, test_point.y, test_point.z);
  printf(" %s prism\n", in_prism ? "in" : "not in");
  printf(" normal to prism: {%e,%e,%e}\n", nhat.x, nhat.y, nhat.z);
  printf(" intersection with line segment {%e,%e,%e} + (%e,%e)*{%e,%e,%e}: %e\n", test_point.x,
         test_point.y, test_point.z, a, b, test_dir.x, test_dir.y, test_dir.z, s);
}
