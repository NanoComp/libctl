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
/* Test: open mesh detection                                            */
/************************************************************************/
static void test_open_mesh(void) {
  printf("test_open_mesh... ");
  /* A tetrahedron with one face duplicated and one face missing — not closed.
     4 faces total (passes the >= 4 check) but has boundary edges. */
  vector3 verts[4] = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}
  };
  int tris[4 * 3] = {
    0, 1, 2,
    0, 1, 3,
    0, 2, 3,
    0, 2, 3   /* duplicate of face 2 instead of the missing face (1,2,3) */
  };
  geometric_object open = make_mesh(NULL, verts, 4, tris, 4);
  mesh *m = open.subclass.mesh_data;
  ASSERT_TRUE("open mesh detected as not closed", !m->is_closed);

  /* point_in_mesh should return false for open meshes. */
  vector3 p = {0.1, 0.1, 0.1};
  ASSERT_TRUE("open mesh: point_in returns false", !point_in_fixed_pobjectp(p, &open));

  geometric_object_destroy(open);
  printf("done\n");
}

/************************************************************************/
/* Test: closed mesh correctly detected                                 */
/************************************************************************/
static void test_closed_detection(void) {
  printf("test_closed_detection... ");
  geometric_object cube = make_cube_mesh(NULL);
  mesh *m = cube.subclass.mesh_data;
  ASSERT_TRUE("cube mesh detected as closed", m->is_closed);
  geometric_object_destroy(cube);

  geometric_object tetra = make_tetra_mesh(NULL);
  m = tetra.subclass.mesh_data;
  ASSERT_TRUE("tetra mesh detected as closed", m->is_closed);
  geometric_object_destroy(tetra);
  printf("done\n");
}

/************************************************************************/
/* Test: boundary edge (1 face on an edge)                              */
/************************************************************************/
static void test_boundary_edge(void) {
  printf("test_boundary_edge... ");
  /* Take a valid tetrahedron but remove the last face, replace with
     a face that shares no edge with it — leaving boundary edges
     where the removed face was. */
  vector3 verts[5] = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {2, 2, 2}
  };
  int tris[4 * 3] = {
    0, 1, 2,   /* face 0 */
    0, 1, 3,   /* face 1 */
    0, 2, 3,   /* face 2 */
    /* missing: 1, 2, 3 — replaced with unconnected face */
    0, 1, 4    /* face 3: edge (1,2) and (2,3) now have only 1 face */
  };
  geometric_object obj = make_mesh(NULL, verts, 5, tris, 4);
  mesh *m = obj.subclass.mesh_data;
  ASSERT_TRUE("boundary edge mesh detected as not closed", !m->is_closed);
  geometric_object_destroy(obj);
  printf("done\n");
}

/************************************************************************/
/* Test: non-manifold edge (3+ faces sharing an edge)                   */
/************************************************************************/
static void test_nonmanifold_edge(void) {
  printf("test_nonmanifold_edge... ");
  /* A tetrahedron (4 faces) plus 2 extra faces sharing edge (0,1),
     giving that edge 4 adjacent faces instead of 2. */
  vector3 verts[6] = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
    {0.5, -1, 0.5}, {0.5, -1, -0.5}
  };
  int tris[6 * 3] = {
    0, 1, 2,   /* tetra face 0 — uses edge (0,1) */
    0, 1, 3,   /* tetra face 1 — uses edge (0,1) */
    0, 2, 3,   /* tetra face 2 */
    1, 2, 3,   /* tetra face 3 */
    0, 1, 4,   /* extra face — edge (0,1) now has 3 faces */
    0, 1, 5,   /* extra face — edge (0,1) now has 4 faces */
  };
  geometric_object obj = make_mesh(NULL, verts, 6, tris, 6);
  mesh *m = obj.subclass.mesh_data;
  ASSERT_TRUE("non-manifold edge mesh detected as not closed", !m->is_closed);
  geometric_object_destroy(obj);
  printf("done\n");
}

/************************************************************************/
/* Test: isolated vertex (vertex not referenced by any face)            */
/************************************************************************/
static void test_isolated_vertex(void) {
  printf("test_isolated_vertex... ");
  /* A valid tetrahedron plus an extra vertex that no face references.
     The mesh should still be detected as closed (isolated vertices
     don't affect edge manifold-ness). */
  vector3 verts[5] = {
    {1, 1, 1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, 1},
    {99, 99, 99}  /* isolated vertex */
  };
  int tris[4 * 3] = {
    0, 1, 2,
    0, 3, 1,
    0, 2, 3,
    1, 3, 2
  };
  geometric_object obj = make_mesh(NULL, verts, 5, tris, 4);
  mesh *m = obj.subclass.mesh_data;
  ASSERT_TRUE("mesh with isolated vertex still closed", m->is_closed);

  /* Point inside tetrahedron should still work. */
  vector3 p = {0, 0, 0};
  ASSERT_TRUE("isolated vertex: centroid inside", point_in_fixed_pobjectp(p, &obj));

  geometric_object_destroy(obj);
  printf("done\n");
}

/************************************************************************/
/* Test: mixed winding across disconnected components.                  */
/* Two unit cubes: one at (-1.5,0,0) with outward winding, one at      */
/* (1.5,0,0) with inward (reversed) winding. Per-component auto-fix    */
/* should correct the inverted cube independently.                      */
/************************************************************************/
static geometric_object make_two_cube_mesh_mixed_winding(void *material) {
  /* Cube A centered at (-1.5, 0, 0): outward winding (CCW from outside). */
  vector3 verts[16] = {
    /* Cube A vertices 0-7 */
    {-2, -0.5, -0.5}, {-1, -0.5, -0.5}, {-1,  0.5, -0.5}, {-2,  0.5, -0.5},
    {-2, -0.5,  0.5}, {-1, -0.5,  0.5}, {-1,  0.5,  0.5}, {-2,  0.5,  0.5},
    /* Cube B vertices 8-15 */
    { 1, -0.5, -0.5}, { 2, -0.5, -0.5}, { 2,  0.5, -0.5}, { 1,  0.5, -0.5},
    { 1, -0.5,  0.5}, { 2, -0.5,  0.5}, { 2,  0.5,  0.5}, { 1,  0.5,  0.5}
  };
  int tris[24 * 3] = {
    /* Cube A: outward-facing (correct winding). */
    0,2,1,  0,3,2,   /* -Z */
    4,5,6,  4,6,7,   /* +Z */
    0,1,5,  0,5,4,   /* -Y */
    2,3,7,  2,7,6,   /* +Y */
    0,4,7,  0,7,3,   /* -X */
    1,2,6,  1,6,5,   /* +X */
    /* Cube B: inward-facing (REVERSED winding — should be auto-fixed). */
    8,9,10,   8,10,11,   /* -Z reversed */
    12,14,13, 12,15,14,  /* +Z reversed */
    8,13,9,   8,12,13,   /* -Y reversed */
    10,14,11, 10,15,14,  /* +Y reversed — WRONG, fix below */
    8,11,15,  8,15,12,   /* -X reversed */
    9,13,14,  9,14,10    /* +X reversed */
  };
  /* Fix +Y face of cube B: the reversed winding for +Y should be: */
  tris[18*3+0]=10; tris[18*3+1]=14; tris[18*3+2]=15;
  tris[19*3+0]=10; tris[19*3+1]=15; tris[19*3+2]=11;
  return make_mesh(material, verts, 16, tris, 24);
}

static void test_mixed_winding(void) {
  printf("test_mixed_winding... ");
  geometric_object obj = make_two_cube_mesh_mixed_winding(NULL);

  /* Both cubes should report correct volume. Total = 1.0 + 1.0 = 2.0. */
  double vol = geom_object_volume(obj);
  ASSERT_NEAR("mixed winding: total volume", vol, 2.0, TOLERANCE);

  /* Points inside each cube should be detected as inside. */
  vector3 p_a = {-1.5, 0, 0};
  ASSERT_TRUE("mixed winding: center of cube A inside", point_in_fixed_pobjectp(p_a, &obj));
  vector3 p_b = {1.5, 0, 0};
  ASSERT_TRUE("mixed winding: center of cube B inside", point_in_fixed_pobjectp(p_b, &obj));

  /* Point between cubes should be outside. */
  vector3 p_mid = {0, 0, 0};
  ASSERT_TRUE("mixed winding: midpoint outside", !point_in_fixed_pobjectp(p_mid, &obj));

  /* Normals should point outward for both cubes. */
  vector3 p_bx = {1.99, 0, 0};
  vector3 n_bx = normal_to_fixed_object(p_bx, obj);
  ASSERT_NEAR("mixed winding: cube B +X normal", n_bx.x, 1.0, 0.01);

  vector3 p_ax = {-1.99, 0, 0};
  vector3 n_ax = normal_to_fixed_object(p_ax, obj);
  ASSERT_NEAR("mixed winding: cube A -X normal", n_ax.x, -1.0, 0.01);

  geometric_object_destroy(obj);
  printf("done\n");
}

/************************************************************************/
/* Test: high intersection count (>256 ray crossings).                  */
/* Builds 130 disconnected unit cubes along x-axis (= 260 crossings).  */
/* Before the fix, this would silently truncate at 256, corrupting the  */
/* intersection list and producing wrong interior length.               */
/************************************************************************/
static void test_many_intersections(void) {
  printf("test_many_intersections... ");
  int num_cubes = 130;
  int nv = num_cubes * 8;
  int nf = num_cubes * 12;
  vector3 *verts = (vector3 *)malloc(nv * sizeof(vector3));
  int *tris = (int *)malloc(nf * 3 * sizeof(int));

  for (int s = 0; s < num_cubes; s++) {
    double cx = s * 3.0;
    int vi = s * 8;
    verts[vi+0] = (vector3){cx-0.5, -0.5, -0.5};
    verts[vi+1] = (vector3){cx+0.5, -0.5, -0.5};
    verts[vi+2] = (vector3){cx+0.5,  0.5, -0.5};
    verts[vi+3] = (vector3){cx-0.5,  0.5, -0.5};
    verts[vi+4] = (vector3){cx-0.5, -0.5,  0.5};
    verts[vi+5] = (vector3){cx+0.5, -0.5,  0.5};
    verts[vi+6] = (vector3){cx+0.5,  0.5,  0.5};
    verts[vi+7] = (vector3){cx-0.5,  0.5,  0.5};

    int ti = s * 12 * 3;
    int cube_tris[12][3] = {
      {0,2,1}, {0,3,2}, {4,5,6}, {4,6,7},
      {0,1,5}, {0,5,4}, {2,3,7}, {2,7,6},
      {0,4,7}, {0,7,3}, {1,2,6}, {1,6,5}
    };
    for (int f = 0; f < 12; f++) {
      tris[ti + f*3 + 0] = vi + cube_tris[f][0];
      tris[ti + f*3 + 1] = vi + cube_tris[f][1];
      tris[ti + f*3 + 2] = vi + cube_tris[f][2];
    }
  }

  geometric_object obj = make_mesh(NULL, verts, nv, tris, nf);

  /* A line segment along x through all cubes should give total interior
     length = num_cubes * 1.0 (each cube has x-extent of 1.0). */
  vector3 p = {-1, 0, 0};
  vector3 d = {1, 0, 0};
  double len = intersect_line_segment_with_object(p, d, obj, 0, num_cubes * 3.0);
  ASSERT_NEAR("many intersections: total interior length", len, (double)num_cubes, 0.01);

  geometric_object_destroy(obj);
  free(verts);
  free(tris);
  printf("done\n");
}

/************************************************************************/
/* Test: vertex mutation followed by geom_fix_object_ptr.               */
/* Translates all vertices by +10 in x, calls fix_object_ptr, verifies  */
/* that queries use updated geometry (BVH/normals/centroid rebuilt).     */
/************************************************************************/
static void test_vertex_mutation(void) {
  printf("test_vertex_mutation... ");
  geometric_object cube = make_cube_mesh(NULL);

  /* Before mutation: origin is inside. */
  vector3 p0 = {0, 0, 0};
  ASSERT_TRUE("pre-mutate: origin inside", point_in_fixed_pobjectp(p0, &cube));

  /* Mutate: translate all vertices by +10 in x. */
  mesh *m = cube.subclass.mesh_data;
  for (int i = 0; i < m->vertices.num_items; i++)
    m->vertices.items[i].x += 10.0;

  /* Trigger rebuild via geom_fix_object_ptr. */
  geom_fix_object_ptr(&cube);

  /* After mutation: origin should be outside, (10,0,0) should be inside. */
  ASSERT_TRUE("post-mutate: origin outside", !point_in_fixed_pobjectp(p0, &cube));
  vector3 p10 = {10, 0, 0};
  ASSERT_TRUE("post-mutate: (10,0,0) inside", point_in_fixed_pobjectp(p10, &cube));

  /* Volume should still be 1.0. */
  double vol = geom_object_volume(cube);
  ASSERT_NEAR("post-mutate: volume", vol, 1.0, TOLERANCE);

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
  test_open_mesh();
  test_closed_detection();
  test_boundary_edge();
  test_nonmanifold_edge();
  test_isolated_vertex();
  test_mixed_winding();
  test_many_intersections();
  test_vertex_mutation();

  printf("\n%d test failures\n", test_failures);
  return test_failures > 0 ? 1 : 0;
}
