/* THIS FILE WAS AUTOMATICALLY GENERATED.  DO NOT MODIFY! */
/* generated from the file: ./geom.scm */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ctlgeom-types.h"

#ifdef CXX_CTL_IO
using namespace	ctlio;
#endif


#include "geom-ctl-io-defaults.c"
#if 0

integer		dimensions;
SCM		default_material;
vector3		geometry_center;
lattice		geometry_lattice;
geometric_object_list geometry;
boolean		ensure_periodicity;

#endif


/******* class copy functions *******/

void 
lattice_copy(const lattice * o0, lattice * o)
{
	o->basis1 = o0->basis1;
	o->basis2 = o0->basis2;
	o->basis3 = o0->basis3;
	o->size = o0->size;
	o->basis_size = o0->basis_size;
	o->b1 = o0->b1;
	o->b2 = o0->b2;
	o->b3 = o0->b3;
	o->basis = o0->basis;
	o->metric = o0->metric;
}

void 
ellipsoid_copy(const ellipsoid * o0, ellipsoid * o)
{
	o->inverse_semi_axes = o0->inverse_semi_axes;
}

void
mesh_copy(const mesh * o0, mesh * o)
{
	{
		int		i_t;
		o->vertices.num_items = o0->vertices.num_items;
		o->vertices.items = ((vector3 *) malloc(sizeof(vector3) * (o->vertices.num_items)));
		for (i_t = 0; i_t < o->vertices.num_items; i_t++) {
			o->vertices.items[i_t] = o0->vertices.items[i_t];
		}
	}
	o->num_faces = o0->num_faces;
	o->face_indices = (int *) malloc(sizeof(int) * 3 * o->num_faces);
	memcpy(o->face_indices, o0->face_indices, sizeof(int) * 3 * o->num_faces);
	o->face_normals = (vector3 *) malloc(sizeof(vector3) * o->num_faces);
	memcpy(o->face_normals, o0->face_normals, sizeof(vector3) * o->num_faces);
	o->face_areas = (number *) malloc(sizeof(number) * o->num_faces);
	memcpy(o->face_areas, o0->face_areas, sizeof(number) * o->num_faces);
	o->num_bvh_nodes = o0->num_bvh_nodes;
	o->bvh = (mesh_bvh_node *) malloc(sizeof(mesh_bvh_node) * o->num_bvh_nodes);
	memcpy(o->bvh, o0->bvh, sizeof(mesh_bvh_node) * o->num_bvh_nodes);
	o->bvh_face_ids = (int *) malloc(sizeof(int) * o->num_faces);
	memcpy(o->bvh_face_ids, o0->bvh_face_ids, sizeof(int) * o->num_faces);
	o->is_closed = o0->is_closed;
	o->centroid = o0->centroid;
	o->lengthscale = o0->lengthscale;
}

void
prism_copy(const prism * o0, prism * o)
{
	{
		int		i_t;
		o->vertices.num_items = o0->vertices.num_items;
		o->vertices.items = ((vector3 *) malloc(sizeof(vector3) * (o->vertices.num_items)));
		for (i_t = 0; i_t < o->vertices.num_items; i_t++) {
			o->vertices.items[i_t] = o0->vertices.items[i_t];
		}
	}
	o->height = o0->height;
	o->axis = o0->axis;
	o->sidewall_angle = o0->sidewall_angle;
	{
		int		i_t;
		o->vertices_p.num_items = o0->vertices_p.num_items;
		o->vertices_p.items = ((vector3 *) malloc(sizeof(vector3) * (o->vertices_p.num_items)));
		for (i_t = 0; i_t < o->vertices_p.num_items; i_t++) {
			o->vertices_p.items[i_t] = o0->vertices_p.items[i_t];
		}
	}
	{
		int		i_t;
		o->top_polygon_diff_vectors_p.num_items = o0->top_polygon_diff_vectors_p.num_items;
		o->top_polygon_diff_vectors_p.items = ((vector3 *) malloc(sizeof(vector3) * (o->top_polygon_diff_vectors_p.num_items)));
		for (i_t = 0; i_t < o->top_polygon_diff_vectors_p.num_items; i_t++) {
			o->top_polygon_diff_vectors_p.items[i_t] = o0->top_polygon_diff_vectors_p.items[i_t];
		}
	}
	{
		int		i_t;
		o->top_polygon_diff_vectors_scaled_p.num_items = o0->top_polygon_diff_vectors_scaled_p.num_items;
		o->top_polygon_diff_vectors_scaled_p.items = ((vector3 *) malloc(sizeof(vector3) * (o->top_polygon_diff_vectors_scaled_p.num_items)));
		for (i_t = 0; i_t < o->top_polygon_diff_vectors_scaled_p.num_items; i_t++) {
			o->top_polygon_diff_vectors_scaled_p.items[i_t] = o0->top_polygon_diff_vectors_scaled_p.items[i_t];
		}
	}
	{
		int		i_t;
		o->vertices_top_p.num_items = o0->vertices_top_p.num_items;
		o->vertices_top_p.items = ((vector3 *) malloc(sizeof(vector3) * (o->vertices_top_p.num_items)));
		for (i_t = 0; i_t < o->vertices_top_p.num_items; i_t++) {
			o->vertices_top_p.items[i_t] = o0->vertices_top_p.items[i_t];
		}
	}
	{
		int		i_t;
		o->vertices_top.num_items = o0->vertices_top.num_items;
		o->vertices_top.items = ((vector3 *) malloc(sizeof(vector3) * (o->vertices_top.num_items)));
		for (i_t = 0; i_t < o->vertices_top.num_items; i_t++) {
			o->vertices_top.items[i_t] = o0->vertices_top.items[i_t];
		}
	}
	o->centroid = o0->centroid;
	{
		int		i_t;
		o->workspace.num_items = o0->workspace.num_items;
		o->workspace.items = ((number *) malloc(sizeof(number) * (o->workspace.num_items)));
		for (i_t = 0; i_t < o->workspace.num_items; i_t++) {
			o->workspace.items[i_t] = o0->workspace.items[i_t];
		}
	}
	o->m_c2p = o0->m_c2p;
	o->m_p2c = o0->m_p2c;
}

void 
block_copy(const block * o0, block * o)
{
	o->e1 = o0->e1;
	o->e2 = o0->e2;
	o->e3 = o0->e3;
	o->size = o0->size;
	o->projection_matrix = o0->projection_matrix;
	if (o0->which_subclass == ELLIPSOID) {
		o->which_subclass = ELLIPSOID;
		o->subclass.ellipsoid_data = ((ellipsoid *) malloc(sizeof(ellipsoid)));
		ellipsoid_copy(o0->subclass.ellipsoid_data, o->subclass.ellipsoid_data);
	} else
		o->which_subclass = BLOCK_SELF;
}

void 
sphere_copy(const sphere * o0, sphere * o)
{
	o->radius = o0->radius;
}

void 
wedge_copy(const wedge * o0, wedge * o)
{
	o->wedge_angle = o0->wedge_angle;
	o->wedge_start = o0->wedge_start;
	o->e1 = o0->e1;
	o->e2 = o0->e2;
}

void 
cone_copy(const cone * o0, cone * o)
{
	o->radius2 = o0->radius2;
}

void 
cylinder_copy(const cylinder * o0, cylinder * o)
{
	o->axis = o0->axis;
	o->radius = o0->radius;
	o->height = o0->height;
	if (o0->which_subclass == WEDGE) {
		o->which_subclass = WEDGE;
		o->subclass.wedge_data = ((wedge *) malloc(sizeof(wedge)));
		wedge_copy(o0->subclass.wedge_data, o->subclass.wedge_data);
	} else if (o0->which_subclass == CONE) {
		o->which_subclass = CONE;
		o->subclass.cone_data = ((cone *) malloc(sizeof(cone)));
		cone_copy(o0->subclass.cone_data, o->subclass.cone_data);
	} else
		o->which_subclass = CYLINDER_SELF;
}

void 
compound_geometric_object_copy(const compound_geometric_object * o0, compound_geometric_object * o)
{
	{
		int		i_t;
		o->component_objects.num_items = o0->component_objects.num_items;
		o->component_objects.items = ((geometric_object *) malloc(sizeof(geometric_object) * (o->component_objects.num_items)));
		for (i_t = 0; i_t < o->component_objects.num_items; i_t++) {
			geometric_object_copy(&o0->component_objects.items[i_t], &o->component_objects.items[i_t]);
		}
	}
}

void 
geometric_object_copy(const geometric_object * o0, geometric_object * o)
{
	o->material = o0->material;
	o->center = o0->center;
	if (o0->which_subclass == PRISM) {
		o->which_subclass = PRISM;
		o->subclass.prism_data = ((prism *) malloc(sizeof(prism)));
		prism_copy(o0->subclass.prism_data, o->subclass.prism_data);
	} else if (o0->which_subclass == BLOCK) {
		o->which_subclass = BLOCK;
		o->subclass.block_data = ((block *) malloc(sizeof(block)));
		block_copy(o0->subclass.block_data, o->subclass.block_data);
	} else if (o0->which_subclass == SPHERE) {
		o->which_subclass = SPHERE;
		o->subclass.sphere_data = ((sphere *) malloc(sizeof(sphere)));
		sphere_copy(o0->subclass.sphere_data, o->subclass.sphere_data);
	} else if (o0->which_subclass == CYLINDER) {
		o->which_subclass = CYLINDER;
		o->subclass.cylinder_data = ((cylinder *) malloc(sizeof(cylinder)));
		cylinder_copy(o0->subclass.cylinder_data, o->subclass.cylinder_data);
	} else if (o0->which_subclass == COMPOUND_GEOMETRIC_OBJECT) {
		o->which_subclass = COMPOUND_GEOMETRIC_OBJECT;
		o->subclass.compound_geometric_object_data = ((compound_geometric_object *) malloc(sizeof(compound_geometric_object)));
		compound_geometric_object_copy(o0->subclass.compound_geometric_object_data, o->subclass.compound_geometric_object_data);
	} else if (o0->which_subclass == MESH) {
		o->which_subclass = MESH;
		o->subclass.mesh_data = ((mesh *) malloc(sizeof(mesh)));
		mesh_copy(o0->subclass.mesh_data, o->subclass.mesh_data);
	} else
		o->which_subclass = GEOMETRIC_OBJECT_SELF;
}

/******* class equal functions *******/

boolean 
lattice_equal(const lattice * o0, const lattice * o)
{
	if (!vector3_equal(o->basis1, o0->basis1))
		return 0;
	if (!vector3_equal(o->basis2, o0->basis2))
		return 0;
	if (!vector3_equal(o->basis3, o0->basis3))
		return 0;
	if (!vector3_equal(o->size, o0->size))
		return 0;
	if (!vector3_equal(o->basis_size, o0->basis_size))
		return 0;
	if (!vector3_equal(o->b1, o0->b1))
		return 0;
	if (!vector3_equal(o->b2, o0->b2))
		return 0;
	if (!vector3_equal(o->b3, o0->b3))
		return 0;
	if (!matrix3x3_equal(o->basis, o0->basis))
		return 0;
	if (!matrix3x3_equal(o->metric, o0->metric))
		return 0;
	;
	return 1;
}

boolean 
ellipsoid_equal(const ellipsoid * o0, const ellipsoid * o)
{
	if (!vector3_equal(o->inverse_semi_axes, o0->inverse_semi_axes))
		return 0;
	;
	return 1;
}

boolean
mesh_equal(const mesh * o0, const mesh * o)
{
	{
		int		i_t;
		if (o->vertices.num_items != o0->vertices.num_items)
			return 0;
		for (i_t = 0; i_t < o->vertices.num_items; i_t++) {
			if (!vector3_equal(o->vertices.items[i_t], o0->vertices.items[i_t]))
				return 0;
		}
	}
	if (o->num_faces != o0->num_faces)
		return 0;
	{
		int		i_t;
		for (i_t = 0; i_t < 3 * o->num_faces; i_t++) {
			if (o->face_indices[i_t] != o0->face_indices[i_t])
				return 0;
		}
	}
	;
	return 1;
}

boolean
prism_equal(const prism * o0, const prism * o)
{
	{
		int		i_t;
		if (o->vertices.num_items != o0->vertices.num_items)
			return 0;
		for (i_t = 0; i_t < o->vertices.num_items; i_t++) {
			if (!vector3_equal(o->vertices.items[i_t], o0->vertices.items[i_t]))
				return 0;
		}
	}
	if (o->height != o0->height)
		return 0;
	if (!vector3_equal(o->axis, o0->axis))
		return 0;
	if (o->sidewall_angle != o0->sidewall_angle)
		return 0;
	{
		int		i_t;
		if (o->vertices_p.num_items != o0->vertices_p.num_items)
			return 0;
		for (i_t = 0; i_t < o->vertices_p.num_items; i_t++) {
			if (!vector3_equal(o->vertices_p.items[i_t], o0->vertices_p.items[i_t]))
				return 0;
		}
	}
	{
		int		i_t;
		if (o->top_polygon_diff_vectors_p.num_items != o0->top_polygon_diff_vectors_p.num_items)
			return 0;
		for (i_t = 0; i_t < o->top_polygon_diff_vectors_p.num_items; i_t++) {
			if (!vector3_equal(o->top_polygon_diff_vectors_p.items[i_t], o0->top_polygon_diff_vectors_p.items[i_t]))
				return 0;
		}
	}
	{
		int		i_t;
		if (o->top_polygon_diff_vectors_scaled_p.num_items != o0->top_polygon_diff_vectors_scaled_p.num_items)
			return 0;
		for (i_t = 0; i_t < o->top_polygon_diff_vectors_scaled_p.num_items; i_t++) {
			if (!vector3_equal(o->top_polygon_diff_vectors_scaled_p.items[i_t], o0->top_polygon_diff_vectors_scaled_p.items[i_t]))
				return 0;
		}
	}
	{
		int		i_t;
		if (o->vertices_top_p.num_items != o0->vertices_top_p.num_items)
			return 0;
		for (i_t = 0; i_t < o->vertices_top_p.num_items; i_t++) {
			if (!vector3_equal(o->vertices_top_p.items[i_t], o0->vertices_top_p.items[i_t]))
				return 0;
		}
	}
	{
		int		i_t;
		if (o->vertices_top.num_items != o0->vertices_top.num_items)
			return 0;
		for (i_t = 0; i_t < o->vertices_top.num_items; i_t++) {
			if (!vector3_equal(o->vertices_top.items[i_t], o0->vertices_top.items[i_t]))
				return 0;
		}
	}
	if (!vector3_equal(o->centroid, o0->centroid))
		return 0;
	{
		int		i_t;
		if (o->workspace.num_items != o0->workspace.num_items)
			return 0;
		for (i_t = 0; i_t < o->workspace.num_items; i_t++) {
			if (o->workspace.items[i_t] != o0->workspace.items[i_t])
				return 0;
		}
	}
	if (!matrix3x3_equal(o->m_c2p, o0->m_c2p))
		return 0;
	if (!matrix3x3_equal(o->m_p2c, o0->m_p2c))
		return 0;
	;
	return 1;
}

boolean 
block_equal(const block * o0, const block * o)
{
	if (!vector3_equal(o->e1, o0->e1))
		return 0;
	if (!vector3_equal(o->e2, o0->e2))
		return 0;
	if (!vector3_equal(o->e3, o0->e3))
		return 0;
	if (!vector3_equal(o->size, o0->size))
		return 0;
	if (!matrix3x3_equal(o->projection_matrix, o0->projection_matrix))
		return 0;
	if (o0->which_subclass != o->which_subclass)
		return 0;
	if (o0->which_subclass == ELLIPSOID) {
		if (!ellipsoid_equal(o0->subclass.ellipsoid_data, o->subclass.ellipsoid_data))
			return 0;
	} else;
	return 1;
}

boolean 
sphere_equal(const sphere * o0, const sphere * o)
{
	if (o->radius != o0->radius)
		return 0;
	;
	return 1;
}

boolean 
wedge_equal(const wedge * o0, const wedge * o)
{
	if (o->wedge_angle != o0->wedge_angle)
		return 0;
	if (!vector3_equal(o->wedge_start, o0->wedge_start))
		return 0;
	if (!vector3_equal(o->e1, o0->e1))
		return 0;
	if (!vector3_equal(o->e2, o0->e2))
		return 0;
	;
	return 1;
}

boolean 
cone_equal(const cone * o0, const cone * o)
{
	if (o->radius2 != o0->radius2)
		return 0;
	;
	return 1;
}

boolean 
cylinder_equal(const cylinder * o0, const cylinder * o)
{
	if (!vector3_equal(o->axis, o0->axis))
		return 0;
	if (o->radius != o0->radius)
		return 0;
	if (o->height != o0->height)
		return 0;
	if (o0->which_subclass != o->which_subclass)
		return 0;
	if (o0->which_subclass == WEDGE) {
		if (!wedge_equal(o0->subclass.wedge_data, o->subclass.wedge_data))
			return 0;
	} else if (o0->which_subclass == CONE) {
		if (!cone_equal(o0->subclass.cone_data, o->subclass.cone_data))
			return 0;
	} else;
	return 1;
}

boolean 
compound_geometric_object_equal(const compound_geometric_object * o0, const compound_geometric_object * o)
{
	{
		int		i_t;
		if (o->component_objects.num_items != o0->component_objects.num_items)
			return 0;
		for (i_t = 0; i_t < o->component_objects.num_items; i_t++) {
			if (!geometric_object_equal(&o0->component_objects.items[i_t], &o->component_objects.items[i_t]))
				return 0;
		}
	}
	;
	return 1;
}

boolean 
geometric_object_equal(const geometric_object * o0, const geometric_object * o)
{
	if (o->material != o0->material)
		return 0;
	if (!vector3_equal(o->center, o0->center))
		return 0;
	if (o0->which_subclass != o->which_subclass)
		return 0;
	if (o0->which_subclass == PRISM) {
		if (!prism_equal(o0->subclass.prism_data, o->subclass.prism_data))
			return 0;
	} else if (o0->which_subclass == BLOCK) {
		if (!block_equal(o0->subclass.block_data, o->subclass.block_data))
			return 0;
	} else if (o0->which_subclass == SPHERE) {
		if (!sphere_equal(o0->subclass.sphere_data, o->subclass.sphere_data))
			return 0;
	} else if (o0->which_subclass == CYLINDER) {
		if (!cylinder_equal(o0->subclass.cylinder_data, o->subclass.cylinder_data))
			return 0;
	} else if (o0->which_subclass == COMPOUND_GEOMETRIC_OBJECT) {
		if (!compound_geometric_object_equal(o0->subclass.compound_geometric_object_data, o->subclass.compound_geometric_object_data))
			return 0;
	} else if (o0->which_subclass == MESH) {
		if (!mesh_equal(o0->subclass.mesh_data, o->subclass.mesh_data))
			return 0;
	} else;
	return 1;
}

/******* class destruction functions *******/

void 
lattice_destroy(lattice o)
{
}

void 
ellipsoid_destroy(ellipsoid o)
{
}

void
mesh_destroy(mesh o)
{
	free(o.vertices.items);
	free(o.face_indices);
	free(o.face_normals);
	free(o.face_areas);
	free(o.bvh);
	free(o.bvh_face_ids);
}

void
prism_destroy(prism o)
{
	{
		int		index_t;
		for (index_t = 0; index_t < o.vertices.num_items; index_t++) {
		}
	}
	free(o.vertices.items);
	{
		int		index_t;
		for (index_t = 0; index_t < o.vertices_p.num_items; index_t++) {
		}
	}
	free(o.vertices_p.items);
	{
		int		index_t;
		for (index_t = 0; index_t < o.top_polygon_diff_vectors_p.num_items; index_t++) {
		}
	}
	free(o.top_polygon_diff_vectors_p.items);
	{
		int		index_t;
		for (index_t = 0; index_t < o.top_polygon_diff_vectors_scaled_p.num_items; index_t++) {
		}
	}
	free(o.top_polygon_diff_vectors_scaled_p.items);
	{
		int		index_t;
		for (index_t = 0; index_t < o.vertices_top_p.num_items; index_t++) {
		}
	}
	free(o.vertices_top_p.items);
	{
		int		index_t;
		for (index_t = 0; index_t < o.vertices_top.num_items; index_t++) {
		}
	}
	free(o.vertices_top.items);
	{
		int		index_t;
		for (index_t = 0; index_t < o.workspace.num_items; index_t++) {
		}
	}
	free(o.workspace.items);
}

void 
block_destroy(block o)
{
	if (o.which_subclass == ELLIPSOID) {
		ellipsoid_destroy(*o.subclass.ellipsoid_data);
		free(o.subclass.ellipsoid_data);
	} else {
	}
}

void 
sphere_destroy(sphere o)
{
}

void 
wedge_destroy(wedge o)
{
}

void 
cone_destroy(cone o)
{
}

void 
cylinder_destroy(cylinder o)
{
	if (o.which_subclass == WEDGE) {
		wedge_destroy(*o.subclass.wedge_data);
		free(o.subclass.wedge_data);
	} else if (o.which_subclass == CONE) {
		cone_destroy(*o.subclass.cone_data);
		free(o.subclass.cone_data);
	} else {
	}
}

void 
compound_geometric_object_destroy(compound_geometric_object o)
{
	{
		int		index_t;
		for (index_t = 0; index_t < o.component_objects.num_items; index_t++) {
			geometric_object_destroy(o.component_objects.items[index_t]);
		}
	}
	free(o.component_objects.items);
}

void 
geometric_object_destroy(geometric_object o)
{
	if (o.which_subclass == PRISM) {
		prism_destroy(*o.subclass.prism_data);
		free(o.subclass.prism_data);
	} else if (o.which_subclass == BLOCK) {
		block_destroy(*o.subclass.block_data);
		free(o.subclass.block_data);
	} else if (o.which_subclass == SPHERE) {
		sphere_destroy(*o.subclass.sphere_data);
		free(o.subclass.sphere_data);
	} else if (o.which_subclass == CYLINDER) {
		cylinder_destroy(*o.subclass.cylinder_data);
		free(o.subclass.cylinder_data);
	} else if (o.which_subclass == COMPOUND_GEOMETRIC_OBJECT) {
		compound_geometric_object_destroy(*o.subclass.compound_geometric_object_data);
		free(o.subclass.compound_geometric_object_data);
	} else if (o.which_subclass == MESH) {
		mesh_destroy(*o.subclass.mesh_data);
		free(o.subclass.mesh_data);
	} else {
	}
}
