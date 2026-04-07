/* libctl: flexible Guile-based control files for scientific software
 * Copyright (C) 1998-2020 Massachusetts Institute of Technology and Steven G. Johnson
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
/* test-mesh.c: unit test for mesh geometry in libctlgeom               */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ctlgeom.h"

#define K_PI 3.141592653589793238462643383279502884197
#define TOLERANCE 1e-6

static int test_failures = 0;

#define ASSERT_TRUE(msg, cond)                                                \
  do {                                                                        \
    if (!(cond)) {                                                            \
      fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);                 \
      test_failures++;                                                        \
    }                                                                         \
  } while (0)

#define ASSERT_NEAR(msg, val, expected, tol)                                  \
  do {                                                                        \
    double _v = (val), _e = (expected);                                       \
    if (fabs(_v - _e) > (tol)) {                                             \
      fprintf(stderr, "FAIL: %s: got %g, expected %g (line %d)\n",            \
              msg, _v, _e, __LINE__);                                         \
      test_failures++;                                                        \
    }                                                                         \
  } while (0)

/************************************************************************/
/* Helper: create a unit cube mesh centered at origin.                  */
/* 8 vertices, 12 triangles.                                            */
/************************************************************************/
static geometric_object make_cube_mesh(void *material) {
  vector3 verts[8] = {
    {-0.5, -0.5, -0.5}, { 0.5, -0.5, -0.5}, { 0.5,  0.5, -0.5}, {-0.5,  0.5, -0.5},
    {-0.5, -0.5,  0.5}, { 0.5, -0.5,  0.5}, { 0.5,  0.5,  0.5}, {-0.5,  0.5,  0.5}
  };
  /* Outward-facing triangles (right-hand rule). */
  int tris[12 * 3] = {
    /* -Z face */ 0,2,1,  0,3,2,
    /* +Z face */ 4,5,6,  4,6,7,
    /* -Y face */ 0,1,5,  0,5,4,
    /* +Y face */ 2,3,7,  2,7,6,
    /* -X face */ 0,4,7,  0,7,3,
    /* +X face */ 1,2,6,  1,6,5
  };
  return make_mesh(material, verts, 8, tris, 12);
}

/************************************************************************/
/* Helper: create a regular tetrahedron mesh.                           */
/************************************************************************/
static geometric_object make_tetra_mesh(void *material) {
  double a = 1.0;
  vector3 verts[4] = {
    { a,  a,  a},
    { a, -a, -a},
    {-a,  a, -a},
    {-a, -a,  a}
  };
  /* Outward-facing triangles. */
  int tris[4 * 3] = {
    0, 1, 2,
    0, 3, 1,
    0, 2, 3,
    1, 3, 2
  };
  return make_mesh(material, verts, 4, tris, 4);
}

/************************************************************************/
/* Test: point_in_mesh for cube                                         */
/************************************************************************/
static void test_cube_point_in(void) {
  printf("test_cube_point_in... ");
  geometric_object cube = make_cube_mesh(NULL);

  /* Points inside. */
  vector3 p_inside = {0, 0, 0};
  ASSERT_TRUE("origin inside cube", point_in_fixed_pobjectp(p_inside, &cube));

  vector3 p_inside2 = {0.4, 0.4, 0.4};
  ASSERT_TRUE("corner-near point inside cube", point_in_fixed_pobjectp(p_inside2, &cube));

  /* Points outside. */
  vector3 p_outside = {1.0, 0, 0};
  ASSERT_TRUE("point outside cube +x", !point_in_fixed_pobjectp(p_outside, &cube));

  vector3 p_outside2 = {0.6, 0, 0};
  ASSERT_TRUE("point outside cube +x close", !point_in_fixed_pobjectp(p_outside2, &cube));

  vector3 p_outside3 = {0, 0.6, 0};
  ASSERT_TRUE("point outside cube +y", !point_in_fixed_pobjectp(p_outside3, &cube));

  vector3 p_outside4 = {0, 0, -0.6};
  ASSERT_TRUE("point outside cube -z", !point_in_fixed_pobjectp(p_outside4, &cube));

  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
/* Test: cube volume                                                    */
/************************************************************************/
static void test_cube_volume(void) {
  printf("test_cube_volume... ");
  geometric_object cube = make_cube_mesh(NULL);

  double vol = geom_object_volume(cube);
  ASSERT_NEAR("cube volume", vol, 1.0, TOLERANCE);

  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
/* Test: tetrahedron volume                                             */
/************************************************************************/
static void test_tetra_volume(void) {
  printf("test_tetra_volume... ");
  geometric_object tetra = make_tetra_mesh(NULL);

  /* Regular tetrahedron with vertices at (±1,±1,±1):
     edge length = 2*sqrt(2), volume = edge^3 / (6*sqrt(2)) = 8/3 */
  double expected_vol = 8.0 / 3.0;
  double vol = geom_object_volume(tetra);
  ASSERT_NEAR("tetra volume", vol, expected_vol, TOLERANCE);

  geometric_object_destroy(tetra);
  printf("done\n");
}

/************************************************************************/
/* Test: bounding box                                                   */
/************************************************************************/
static void test_cube_bounding_box(void) {
  printf("test_cube_bounding_box... ");
  geometric_object cube = make_cube_mesh(NULL);

  geom_box box;
  geom_get_bounding_box(cube, &box);
  ASSERT_NEAR("cube bbox low.x", box.low.x, -0.5, TOLERANCE);
  ASSERT_NEAR("cube bbox low.y", box.low.y, -0.5, TOLERANCE);
  ASSERT_NEAR("cube bbox low.z", box.low.z, -0.5, TOLERANCE);
  ASSERT_NEAR("cube bbox high.x", box.high.x, 0.5, TOLERANCE);
  ASSERT_NEAR("cube bbox high.y", box.high.y, 0.5, TOLERANCE);
  ASSERT_NEAR("cube bbox high.z", box.high.z, 0.5, TOLERANCE);

  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
/* Test: normals                                                        */
/************************************************************************/
static void test_cube_normals(void) {
  printf("test_cube_normals... ");
  geometric_object cube = make_cube_mesh(NULL);

  /* Point near +X face: normal should point in +X direction. */
  vector3 p = {0.49, 0, 0};
  vector3 n = normal_to_fixed_object(p, cube);
  ASSERT_NEAR("normal near +x face: nx", fabs(n.x), 1.0, 0.01);
  ASSERT_NEAR("normal near +x face: ny", fabs(n.y), 0.0, 0.01);
  ASSERT_NEAR("normal near +x face: nz", fabs(n.z), 0.0, 0.01);

  /* Point near +Y face. */
  vector3 p2 = {0, 0.49, 0};
  vector3 n2 = normal_to_fixed_object(p2, cube);
  ASSERT_NEAR("normal near +y face: ny", fabs(n2.y), 1.0, 0.01);

  /* Point near -Z face. */
  vector3 p3 = {0, 0, -0.49};
  vector3 n3 = normal_to_fixed_object(p3, cube);
  ASSERT_NEAR("normal near -z face: nz", fabs(n3.z), 1.0, 0.01);

  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
/* Test: cube mesh vs block (compare point_in results)                  */
/************************************************************************/
static void test_cube_vs_block(void) {
  printf("test_cube_vs_block... ");
  geometric_object cube_mesh = make_cube_mesh(NULL);
  vector3 center = {0, 0, 0};
  vector3 e1 = {1, 0, 0}, e2 = {0, 1, 0}, e3 = {0, 0, 1};
  vector3 size = {1, 1, 1};
  geometric_object cube_block = make_block(NULL, center, e1, e2, e3, size);

  int mismatches = 0;
  srand(42);
  for (int i = 0; i < 10000; i++) {
    vector3 p;
    p.x = (rand() / (double)RAND_MAX) * 2.0 - 1.0;
    p.y = (rand() / (double)RAND_MAX) * 2.0 - 1.0;
    p.z = (rand() / (double)RAND_MAX) * 2.0 - 1.0;

    int in_mesh = point_in_fixed_pobjectp(p, &cube_mesh);
    int in_block = point_in_fixed_pobjectp(p, &cube_block);

    /* Allow mismatches very close to the surface (within 1e-6). */
    if (in_mesh != in_block) {
      double dx = fmin(fabs(fabs(p.x) - 0.5), fmin(fabs(fabs(p.y) - 0.5), fabs(fabs(p.z) - 0.5)));
      if (dx > 1e-6) mismatches++;
    }
  }

  ASSERT_TRUE("cube mesh matches block for random points", mismatches == 0);

  geometric_object_destroy(cube_mesh);
  geometric_object_destroy(cube_block);
  printf("done (%d mismatches)\n", mismatches);
}

/************************************************************************/
/* Test: line_segment intersection                                      */
/************************************************************************/
static void test_cube_line_segment(void) {
  printf("test_cube_line_segment... ");
  geometric_object cube = make_cube_mesh(NULL);

  /* Line along x-axis through center: should intersect for length 1.0. */
  vector3 p = {0, 0, 0};
  vector3 d = {1, 0, 0};
  double len = intersect_line_segment_with_object(p, d, cube, -1.0, 1.0);
  ASSERT_NEAR("x-axis segment length", len, 1.0, 0.01);

  /* Line along y-axis. */
  vector3 d2 = {0, 1, 0};
  double len2 = intersect_line_segment_with_object(p, d2, cube, -1.0, 1.0);
  ASSERT_NEAR("y-axis segment length", len2, 1.0, 0.01);

  /* Line that misses the cube. */
  vector3 p3 = {2, 2, 2};
  double len3 = intersect_line_segment_with_object(p3, d, cube, -1.0, 1.0);
  ASSERT_NEAR("miss segment length", len3, 0.0, 0.01);

  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
/* Test: copy and destroy                                               */
/************************************************************************/
static void test_copy_destroy(void) {
  printf("test_copy_destroy... ");
  geometric_object cube = make_cube_mesh(NULL);
  geometric_object cube_copy;
  geometric_object_copy(&cube, &cube_copy);

  /* Verify copy works (point-in test). */
  vector3 p = {0, 0, 0};
  ASSERT_TRUE("copy: origin inside", point_in_fixed_pobjectp(p, &cube_copy));

  vector3 p2 = {1, 0, 0};
  ASSERT_TRUE("copy: outside", !point_in_fixed_pobjectp(p2, &cube_copy));

  double vol = geom_object_volume(cube_copy);
  ASSERT_NEAR("copy: volume", vol, 1.0, TOLERANCE);

  geometric_object_destroy(cube);
  geometric_object_destroy(cube_copy);
  printf("done\n");
}

/************************************************************************/
/* Test: display info                                                   */
/************************************************************************/
static void test_display_info(void) {
  printf("test_display_info... ");
  geometric_object cube = make_cube_mesh(NULL);
  display_geometric_object_info(0, cube);
  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
/* Test: point_in_mesh for tetrahedron                                  */
/************************************************************************/
static void test_tetra_point_in(void) {
  printf("test_tetra_point_in... ");
  geometric_object tetra = make_tetra_mesh(NULL);

  /* Centroid (0,0,0) should be inside. */
  vector3 p_in = {0, 0, 0};
  ASSERT_TRUE("tetra: centroid inside", point_in_fixed_pobjectp(p_in, &tetra));

  /* Point far outside. */
  vector3 p_out = {3, 3, 3};
  ASSERT_TRUE("tetra: far point outside", !point_in_fixed_pobjectp(p_out, &tetra));

  geometric_object_destroy(tetra);
  printf("done\n");
}

/************************************************************************/
/* Test: make_mesh_with_center                                          */
/************************************************************************/
static void test_mesh_with_center(void) {
  printf("test_mesh_with_center... ");
  vector3 verts[8] = {
    {-0.5, -0.5, -0.5}, { 0.5, -0.5, -0.5}, { 0.5,  0.5, -0.5}, {-0.5,  0.5, -0.5},
    {-0.5, -0.5,  0.5}, { 0.5, -0.5,  0.5}, { 0.5,  0.5,  0.5}, {-0.5,  0.5,  0.5}
  };
  int tris[12 * 3] = {
    0,2,1,  0,3,2,
    4,5,6,  4,6,7,
    0,1,5,  0,5,4,
    2,3,7,  2,7,6,
    0,4,7,  0,7,3,
    1,2,6,  1,6,5
  };
  vector3 center = {5, 5, 5};
  geometric_object cube = make_mesh_with_center(NULL, center, verts, 8, tris, 12);

  /* Point at center (5,5,5) should be inside. */
  vector3 p_in = {5, 5, 5};
  ASSERT_TRUE("centered mesh: center inside", point_in_fixed_pobjectp(p_in, &cube));

  /* Point at origin should be outside. */
  vector3 p_out = {0, 0, 0};
  ASSERT_TRUE("centered mesh: origin outside", !point_in_fixed_pobjectp(p_out, &cube));

  /* Volume should still be 1. */
  double vol = geom_object_volume(cube);
  ASSERT_NEAR("centered mesh volume", vol, 1.0, TOLERANCE);

  geometric_object_destroy(cube);
  printf("done\n");
}

/************************************************************************/
int main(void) {
  geom_initialize();

  test_cube_point_in();
  test_cube_volume();
  test_tetra_volume();
  test_cube_bounding_box();
  test_cube_normals();
  test_cube_vs_block();
  test_cube_line_segment();
  test_copy_destroy();
  test_display_info();
  test_tetra_point_in();
  test_mesh_with_center();

  printf("\n%d test failures\n", test_failures);
  return test_failures > 0 ? 1 : 0;
}
