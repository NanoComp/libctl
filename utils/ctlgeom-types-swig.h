/* This file is used to define the types for the SWIG bindings */

#ifndef CTL_IO_H
#define CTL_IO_H

#include <ctl-math.h>

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

	/******* Type declarations *******/

	typedef struct geometric_object_struct {
		void*		material;
		vector3		center;
		enum {
			GEOMETRIC_OBJECT_SELF, PRISM, BLOCK, SPHERE, CYLINDER, COMPOUND_GEOMETRIC_OBJECT
		}		which_subclass;
		union {
			struct prism_struct *prism_data;
			struct block_struct *block_data;
			struct sphere_struct *sphere_data;
			struct cylinder_struct *cylinder_data;
			struct compound_geometric_object_struct *compound_geometric_object_data;
		}		subclass;
	}		geometric_object;

	typedef struct {
		int		num_items;
		geometric_object *items;
	} geometric_object_list;

	typedef struct compound_geometric_object_struct {
		geometric_object_list component_objects;
	}		compound_geometric_object;

	typedef struct cylinder_struct {
		vector3		axis;
		number		radius;
		number		height;
		enum {
			CYLINDER_SELF, WEDGE, CONE
		}		which_subclass;
		union {
			struct wedge_struct *wedge_data;
			struct cone_struct *cone_data;
		}		subclass;
	}		cylinder;

	typedef struct cone_struct {
		number		radius2;
	}		cone;

	typedef struct wedge_struct {
		number		wedge_angle;
		vector3		wedge_start;
		vector3		e1;
		vector3		e2;
	}		wedge;

	typedef struct sphere_struct {
		number		radius;
	}		sphere;

	typedef struct block_struct {
		vector3		e1;
		vector3		e2;
		vector3		e3;
		vector3		size;
		matrix3x3	projection_matrix;
		enum {
			BLOCK_SELF, ELLIPSOID
		}		which_subclass;
		union {
			struct ellipsoid_struct *ellipsoid_data;
		}		subclass;
	}		block;

	typedef struct {
		int		num_items;
		vector3	       *items;
	} vector3_list;

	typedef struct {
		int		num_items;
		number	       *items;
	} number_list;

	typedef struct prism_struct {
		vector3_list	vertices;
		number		height;
		vector3		axis;
		number		sidewall_angle;
		vector3_list	vertices_p;
		vector3_list	top_polygon_diff_vectors_p;
		vector3_list	top_polygon_diff_vectors_scaled_p;
		vector3_list	vertices_top_p;
		vector3_list	vertices_top;
		vector3		centroid;
		number_list	workspace;
		matrix3x3	m_c2p;
		matrix3x3	m_p2c;
	}		prism;

	typedef struct ellipsoid_struct {
		vector3		inverse_semi_axes;
	}		ellipsoid;

	typedef struct lattice_struct {
		vector3		basis1;
		vector3		basis2;
		vector3		basis3;
		vector3		size;
		vector3		basis_size;
		vector3		b1;
		vector3		b2;
		vector3		b3;
		matrix3x3	basis;
		matrix3x3	metric;
	}		lattice;

	/******* Input variables *******/
	extern integer dimensions;
	extern void* default_material;
	extern vector3 geometry_center;
	extern lattice geometry_lattice;
	extern geometric_object_list geometry;
	extern boolean ensure_periodicity;

	/******* Output variables *******/

	/******* class copy function prototypes *******/

	extern void	lattice_copy(const lattice * o0, lattice * o);
	extern void	ellipsoid_copy(const ellipsoid * o0, ellipsoid * o);
	extern void	prism_copy(const prism * o0, prism * o);
	extern void	block_copy(const block * o0, block * o);
	extern void	sphere_copy(const sphere * o0, sphere * o);
	extern void	wedge_copy(const wedge * o0, wedge * o);
	extern void	cone_copy(const cone * o0, cone * o);
	extern void	cylinder_copy(const cylinder * o0, cylinder * o);
	extern void	compound_geometric_object_copy(const compound_geometric_object * o0, compound_geometric_object * o);
	extern void	geometric_object_copy(const geometric_object * o0, geometric_object * o);

	/******* class equal function prototypes *******/

	extern boolean lattice_equal(const lattice * o0, const lattice * o);
	extern boolean ellipsoid_equal(const ellipsoid * o0, const ellipsoid * o);
	extern boolean prism_equal(const prism * o0, const prism * o);
	extern boolean block_equal(const block * o0, const block * o);
	extern boolean sphere_equal(const sphere * o0, const sphere * o);
	extern boolean wedge_equal(const wedge * o0, const wedge * o);
	extern boolean cone_equal(const cone * o0, const cone * o);
	extern boolean cylinder_equal(const cylinder * o0, const cylinder * o);
	extern boolean compound_geometric_object_equal(const compound_geometric_object * o0, const compound_geometric_object * o);
	extern boolean geometric_object_equal(const geometric_object * o0, const geometric_object * o);

	/******* class destruction function prototypes *******/

	extern void	lattice_destroy(lattice o);
	extern void	ellipsoid_destroy(ellipsoid o);
	extern void	prism_destroy(prism o);
	extern void	block_destroy(block o);
	extern void	sphere_destroy(sphere o);
	extern void	wedge_destroy(wedge o);
	extern void	cone_destroy(cone o);
	extern void	cylinder_destroy(cylinder o);
	extern void	compound_geometric_object_destroy(compound_geometric_object o);
	extern void	geometric_object_destroy(geometric_object o);


#ifdef __cplusplus
}				/* extern "C" */
#endif				/* __cplusplus */

#endif				/* CTL_IO_H */
#ifndef LIBCTL_MAJOR_VERSION
#  define LIBCTL_MAJOR_VERSION 4
#  define LIBCTL_MINOR_VERSION 5
#  define LIBCTL_BUGFIX_VERSION 1
#endif
