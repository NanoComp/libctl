/* mesh-internal.h: private C-side cache for the mesh geometric object.
 *
 * The public mesh struct (in ctlgeom-types.h, generated from geom.scm) holds
 * only what's representable in Scheme: the user-supplied vertices + triangle
 * indices and an opaque void *internal pointer. Everything else — face
 * normals, areas, BVH acceleration structure, closure flag, centroid,
 * characteristic lengthscale — lives in mesh_internal, allocated by
 * init_mesh and reached via mesh_priv(m).
 *
 * This header is NOT installed and NOT in geom.scm; it is included only by
 * geom.c and the mesh tests.
 */

#ifndef MESH_INTERNAL_H
#define MESH_INTERNAL_H

#include "ctlgeom-types.h"

typedef struct mesh_bvh_node {
  vector3 bbox_low;
  vector3 bbox_high;
  int     left_child;
  int     right_child;
  int     face_start;
  int     face_count;
} mesh_bvh_node;

typedef struct mesh_internal {
  int            num_faces;
  int           *face_indices;    /* unpacked flat: 3 ints per triangle */
  vector3       *face_normals;
  number        *face_areas;
  int            num_bvh_nodes;
  mesh_bvh_node *bvh;
  int           *bvh_face_ids;
  boolean        is_closed;
  vector3        centroid;
  number         lengthscale;
} mesh_internal;

static inline mesh_internal *mesh_priv(const mesh *m) {
  return (mesh_internal *)m->internal;
}

#endif /* MESH_INTERNAL_H */
