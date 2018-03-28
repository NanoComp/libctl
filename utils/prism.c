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

/*
 * prism.c -- geometry routines for prisms
 *            a prism is a planar polygon, consisting of any
 *            number of user-specified vertices, extruded
 *            through a given thickness (the "height") in the
 *            direction of a given axis.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef LIBCTLGEOM
#  include "ctl-io.h"
#else
#  define material_type void*
   static void material_type_copy(void **src, void **dest) { *dest = *src; }
#endif
#include <ctlgeom.h>

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

  // set x,y coordinates of low, high to bounding box 
  // of prism base polygon (in prism coordinate system)
  vector3 low      = vertices[0];
  vector3 high     = vertices[0];
  for(int nv=1; nv<num_vertices; nv++)
   { low.x  = fmin(low.x, vertices[nv].x);
     low.y  = fmin(low.y, vertices[nv].y);
     high.x = fmax(high.x, vertices[nv].x);
     high.y = fmax(high.y, vertices[nv].y);
   };

  // set z coordinates of low, high to upper and lower 
  // prism surfaces (in prism coordinate system)
  low.z  = 0.0;
  high.z = height;
  
  // convert from prism coordinates to cartesian coordinates
  box->low  = prism_coordinate_p2c(prsm, low);
  box->high = prism_coordinate_p2c(prsm, high);
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

  if (t<0.0 || t>1.0) // intersection of lines does not lie between vertices
   return 0;

  return 1;
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
  for(int nv=0; nv<num_vertices; nv++)
   num_side_intersections
    +=intersect_ray_with_segment(px, py, dx, dy,
                                 vertices[nv], vertices[(nv+1)%num_vertices],
                                 0);
  return num_side_intersections%2;
}

/***************************************************************/
/* return the number of intersections of prism surfaces with   */
/* the line segment p + s*d , a<s<b.                           */
/***************************************************************/
double intersect_line_segment_with_prism(prism *prsm, vector3 p, vector3 d, double a, double b)
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

  // identify s-values s0, s1, ... of line-segment intersections
  // with all prism side surfaces.
  // measure[0] = (s_2-s_1) + (s_4-s_3) + ...
  // measure[1] = (s_1-s_0) + (s_3-s_2) + ...
  int num_intersections=0;
  double last_s, measure[2]={0.0,0.0};
  for(int nv=0; nv<num_vertices; nv++)
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

     // Now we know the line p+s*d intersects the prism...does it do so
     // within the range of the specified line segment?
     if (s<a || s>b)
      continue;

     if (num_intersections>0)
      measure[num_intersections%2] += s-last_s;
     num_intersections++;
     last_s=s;
   };

  // identify intersections of line with prism top and bottom surfaces
  double dz = d_prsm.z;
  if ( fabs(dz) > 1.0e-7*vector3_norm(d_prsm))
   for(int LowerUpper=0; LowerUpper<2; LowerUpper++)
    { double z0    = LowerUpper ? height : 0.0;
      double s     = (z0 - p_prsm.z)/dz;
      if (s<a || s>b) 
       continue;
      if (!point_in_polygon(p_prsm.x + s*d_prsm.x, p_prsm.y+s*d_prsm.y, vertices, num_vertices))
       continue;

      if (num_intersections>0)
       measure[num_intersections%2] += s-last_s;
      num_intersections++;
      last_s=s;
    }

  //if (num_intersections%2)  // point lies inside object
  return 0.0;
}

/***************************************************************/
/***************************************************************/
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
/* compute the minimum-length vector from p to the plane       */
/* containing o (origin) and spanned by basis vectors v1,v2    */
/* algorithm: solve the 3x3 system p-s*v3 = o + t*v1 + u*v2    */
/* where v3 = v1 x v2 and s,t,u are unknowns                   */
/***************************************************************/
vector3 normal_to_plane(vector3 o, vector3 v1, vector3 v2, vector3 p)
{
  vector3 RHS = vector3_minus(p,o);

  // handle the degenerate-to-2D case
  if ( (fabs(v1.z) + fabs(v2.z)) < 2.0e-7 && fabs(RHS.z)<1.0e-7 )
   { vector3 zhat={0,0,1};
     vector3 v3 = vector3_cross(zhat, v1);
     double M00  = v1.x;     double M10  = v3.x;
     double M01  = v1.y;     double M11  = v3.y;
     double DetM = M00*M11 - M01*M10;
     if ( fabs(DetM) < 1.0e-10 )
      return vector3_scale(0.0, v3);
     // double t = (M00*RHSy-M10*RHSx)/DetM;
     double s= (M11*RHS.x-M01*RHS.y)/DetM;
     return vector3_scale(-1.0*s,v3);
   }

  vector3 v3 = vector3_cross(v1, v2);
  matrix3x3 M;
  M.c0 = v1;
  M.c1 = v2;
  M.c2 = vector3_scale(-1.0, v3);
  vector3 tus = matrix3x3_vector3_mult(matrix3x3_inverse(M),RHS);
  return vector3_scale(-1.0*tus.z, v3);
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
  double min_distance;
  // side walls
  for(int nv=0; nv<num_vertices; nv++)
   { int nvp1 = ( nv==(num_vertices-1) ? 0 : nv+1 );
     vector3 v1 = vector3_minus(vertices[nvp1],vertices[nv]);
     vector3 v2 = axis;
     vector3 v3 = normal_to_plane(vertices[nv], v1, v2, xp);
     double distance = vector3_norm(v3);
     if (nv==0 || distance < min_distance)
      { min_distance = distance;
        retval = v3;
      };
   }

  // roof and ceiling
  for(int UpperLower=0; UpperLower<2; UpperLower++)
   { vector3 zhat={0,0,1.0};
     vector3 v1 = vector3_minus(vertices[1],vertices[0]);
     vector3 v2 = vector3_cross(zhat,v1);
     vector3 o  = centroid;
     if (UpperLower) o.z = height;
     vector3 v3 = normal_to_plane(o, v1, v2, xp);
     double distance = vector3_norm(v3);
     if (distance < min_distance)
      { min_distance = distance;
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
  for(int nv=0; nv<num_vertices; nv++)
   { vector3 v = matrix3x3_vector3_mult(m_p2c, vertices[nv]);
     printf("%*s     {%e,%e,%e}\n",indentby,"",v.x,v.y,v.z);
   };

}

/***************************************************************/
// like vector3_equal but tolerant of small floating-point discrepancies
/***************************************************************/
int vector3_nearly_equal(vector3 v1, vector3 v2)
{
  return (vector3_norm( vector3_minus(v1,v2) ) < 1.0e-7*vector3_norm(v1));
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
GEOMETRIC_OBJECT make_prism(MATERIAL_TYPE material,
			    vector3 *vertices, int num_vertices,
			    double height, vector3 axis)
{
  CHECK(num_vertices>=3, "fewer than 3 vertices in make_prism");
   
  // compute centroid of vertices
  vector3 centroid = {0.0, 0.0, 0.0};
  for(int nv=0; nv<num_vertices; nv++)
   centroid = vector3_plus(centroid, vertices[nv]);
  centroid = vector3_scale(1.0/((double)num_vertices), centroid);

  // make sure all vertices lie in a plane normal to the given axis
  vector3 zhat = unit_vector3(axis);
  for(int nv=0; nv<num_vertices; nv++)
   { int nvp1 = (nv+1) % num_vertices;
     vector3 zhatp = triangle_normal(centroid,vertices[nv],vertices[nvp1]);
     boolean axis_normal 
      = (    vector3_nearly_equal(zhat, zhatp) 
          || vector3_nearly_equal(zhat, vector3_scale(-1.0,zhatp))
        );
     CHECK(axis_normal, "axis not normal to vertex plane in make_prism");
   }

  // compute rotation matrix that operates on a vector of cartesian coordinates
  // to yield the coordinates of the same point in the prism coordinate system.
  // the prism coordinate system is a right-handed coordinate system
  // in which the prism lies in the xy plane (extrusion axis is the z-axis) 
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
  // their z-coordinates are all zero and in principle need not
  // be stored
  prsm->vertices.num_items = num_vertices;
  prsm->vertices.items     = (vector3 *)malloc(num_vertices*sizeof(vector3));
  for(int nv=0; nv<num_vertices; nv++)
   prsm->vertices.items[nv] = prism_coordinate_c2p(prsm,vertices[nv]);
 
  // note the center and centroid are different!
  vector3 center = vector3_plus(centroid, vector3_scale(0.5*height,zhat) );
  geometric_object o=make_geometric_object(material, center);
  o.which_subclass=GEOM PRISM;
  o.subclass.prism_data = prsm;

  return o;
}
