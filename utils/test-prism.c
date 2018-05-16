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

#define K_PI 3.141592653589793238462643383279502884197

#define NUMPTS   10000
#define NUMLINES 1000

#define LX 0.5
#define LY 1.0
#define LZ 1.5

static vector3 make_vector3(double x, double y, double z)
{
  vector3 v;
  v.x = x; v.y = y; v.z = z;
  return v;
}

/************************************************************************/
/* return a uniform random number in [a,b] */
/************************************************************************/
static double urand(double a, double b)
{ return a + (b-a)*(rand()/((double)RAND_MAX)); }

static double drand() 
{ return urand(0.0,1.0); }

/************************************************************************/
/* random point uniformly distributed over a parallelepiped             */
/************************************************************************/
vector3 random_point(vector3 min_corner, vector3 max_corner)
{ return make_vector3( urand(min_corner.x, max_corner.x),
                       urand(min_corner.y, max_corner.y),
                       urand(min_corner.z, max_corner.z)
                     );
}

/************************************************************************/
/* random unit vector uniformly distributed over a sphere               */
/************************************************************************/
vector3 random_unit_vector3(void)
{
  double z   = urand(-1.0,1.0);
  double rho = sqrt(1 - z*z);
  double phi = urand(0.0, 2.0*K_PI);
  return make_vector3( rho*cos(phi), rho*sin(phi), z );
}

/***************************************************************/
/* write prism vertices and edges to text file.                */
/* after running this routine to produce a file named MyFile,  */
/* the prism may be plotted in gnuplot like this:              */
/* gnuplot> splot 'MyFile' u 1:2:3 w lp pt 7 ps 1              */
/***************************************************************/
void prism2gnuplot(prism *prsm, char *filename)
{ 
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;

  FILE *f=fopen(filename,"w");
  int nv;
  for(nv=0; nv<num_vertices; nv++)
   {  
     vector3 vap = vertices[nv]; vap.z = 0.0;
     vector3 vbp = vertices[nv]; vbp.z = height;
     vector3 vcp = vertices[(nv+1)%num_vertices]; vcp.z = height;
     vector3 vdp = vertices[(nv+1)%num_vertices]; vdp.z = 0.0;
     vector3 vac = prism_coordinate_p2c(prsm,vap);
     vector3 vbc = prism_coordinate_p2c(prsm,vbp);
     vector3 vcc = prism_coordinate_p2c(prsm,vcp);
     vector3 vdc = prism_coordinate_p2c(prsm,vdp);

     fprintf(f,"%e %e %e %e %e %e \n",vac.x,vac.y,vac.z,vap.x,vap.y,vap.z);
     fprintf(f,"%e %e %e %e %e %e \n",vbc.x,vbc.y,vbc.z,vbp.x,vbp.y,vbp.z);
     fprintf(f,"%e %e %e %e %e %e \n",vcc.x,vcc.y,vcc.z,vcp.x,vcp.y,vcp.z);
     fprintf(f,"%e %e %e %e %e %e \n",vdc.x,vdc.y,vdc.z,vdp.x,vdp.y,vdp.z);
     fprintf(f,"%e %e %e %e %e %e \n",vac.x,vac.y,vac.z,vap.x,vap.y,vap.z);
     fprintf(f,"\n\n");
   }
  fclose(f);
}

/***************************************************************/
/* write prism vertices and edges to GMSH geometry (.geo) file */
/***************************************************************/
void prism2gmsh(prism *prsm, char *filename)
{ 
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;
  vector3 zhat      = prsm->m_p2c.c2;
  vector3 axis      = vector3_scale(height, zhat);

  FILE *f=fopen(filename,"w");
  int nv;
  for(nv=0; nv<num_vertices; nv++)
   { vector3 vp=vertices[nv];
     vector3 vc=prism_coordinate_p2c(prsm, vp);
     fprintf(f,"Point(%i)={%e, %e, %e};\n",nv,vc.x,vc.y,vc.z);
   }
  for(nv=0; nv<num_vertices; nv++)
   fprintf(f,"Line(%i)={%i, %i};\n",nv,nv,(nv+1)%num_vertices);
  fprintf(f,"Line Loop(0)={0");
  for(nv=1; nv<num_vertices; nv++)
   fprintf(f,",%i",nv);
  fprintf(f,"};\n");
  fprintf(f,"Plane Surface(0)={0};\n");
  fprintf(f,"Extrude { %e,%e,%e } { Surface{0}; }\n",height*zhat.x,height*zhat.y,height*zhat.z);
  fclose(f); 
  
}

/* "standardize" a vector for vector comparisons up to normalization and sign flip */
double sgn(double x) { return x>=0.0 ? 1.0 : -1.0; }

vector3 standardize(vector3 v)
{ vector3 sv=unit_vector3(v);
  double sign = (sv.z!=0.0 ? sgn(sv.z) : sv.y!=0.0 ? sgn(sv.y) : sgn(sv.x));
  return vector3_scale(sign,sv);
}

/************************************************************************/
/* first unit test: check inclusion of randomly-generated points        */
/************************************************************************/
int test_point_inclusion(geometric_object the_block, geometric_object the_prism,
                         int num_tests, int write_log)
{
  vector3 size = the_block.subclass.block_data->size;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f = write_log ? fopen("/tmp/test-prism.points","w") : 0;
  int num_failed=0;
  int n;
  for(n=0; n<num_tests; n++)
   { 
     vector3 p = random_point(min_corner, max_corner);
     boolean in_block=point_in_objectp(p,the_block);
     boolean in_prism=point_in_objectp(p,the_prism);
     if (in_block!=in_prism)
      num_failed++;
     if (f) fprintf(f,"%i %i %e %e %e \n",in_block,in_prism,p.x,p.y,p.z);
   }
  if (f) fclose(f);
  
  printf("point inclusion: %i/%i points failed\n",num_failed,num_tests);
  return num_failed;
}

/************************************************************************/
/* second unit test: check calculation of normals to objects            */
/************************************************************************/
int test_normal_to_object(geometric_object the_block, geometric_object the_prism,
                          int num_tests, int write_log)
{
  vector3 size = the_block.subclass.block_data->size;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f = write_log ? fopen("/tmp/test-prism.normals","w") : 0;

  int num_failed=0;
  int n;
  for(n=0; n<num_tests; n++)
   { 
     // randomly generated base point within enlarged bounding box
     vector3 p = random_point(min_corner, max_corner);

     vector3 nhat_block=standardize(normal_to_object(p, the_block));
     vector3 nhat_prism=standardize(normal_to_object(p, the_prism));
     if (!vector3_nearly_equal(nhat_block, nhat_prism))
      num_failed++;

     if (f)
      fprintf(f,"%e %e %e %e %e %e %e %e %e %i\n\n\n",p.x,p.y,p.z,
                 nhat_block.x,nhat_block.y,nhat_block.z,
                 nhat_prism.x,nhat_prism.y,nhat_prism.z,
                 vector3_nearly_equal(nhat_block,nhat_prism));
   }
  if (f) fclose(f);
  
  printf("%i/%i normals failed\n",num_failed,num_tests);
  return num_failed;
}

/************************************************************************/
/* third unit test: check-line segment intersections                   */
/************************************************************************/
int test_line_segment_intersection(geometric_object the_block, geometric_object the_prism,
                                   int num_tests, int write_log)
{
  vector3 size = the_block.subclass.block_data->size;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f = write_log ? fopen("/tmp/test-prism.segments","w") : 0;

  int num_failed=0;
  int n;
  for(n=0; n<num_tests; n++)
   { 
     // randomly generated base point within enlarged bounding box
     vector3 p = random_point(min_corner, max_corner);
     vector3 d = random_unit_vector3();
     double a  = urand(0.0,1.0);
     double b  = urand(0.0,1.0);

     double sblock = intersect_line_segment_with_object(p, d, the_block, a, b);
     double sprism = intersect_line_segment_with_object(p, d, the_prism, a, b);
     if ( fabs(sblock-sprism) > 1.0e-6*fmax(fabs(sblock),fabs(sprism)) )
      num_failed++;

     if (f)
      fprintf(f," %e %e %s\n",sblock,sprism,fabs(sblock-sprism) > 1.0e-6*fmax(fabs(sblock),fabs(sprism)) ? "fail" : "success");
   }
  if (f) fclose(f);
  
  printf("%i/%i segments failed\n",num_failed,num_tests);
  return num_failed;
}

/***************************************************************/
/* unit tests: create the same parallelepiped two ways (as a   */
/* block and as a prism) and verify that geometric primitives  */
/* give identical results                                      */
/***************************************************************/
int run_unit_tests()
{
  void* m = NULL;
  vector3 c = { 0, 0, 0 };
  vector3 xhat = make_vector3(1,0,0);
  vector3 yhat = make_vector3(0,1,0);
  vector3 zhat = make_vector3(0,0,1);
  vector3 size = make_vector3(LX,LY,LZ);
  geometric_object the_block = make_block(m, c, xhat, yhat, zhat, size);

  vector3 v[4]; 
  v[0].x=-0.5*LX; v[0].y=-0.5*LY; v[0].z=-0.5*LZ;
  v[1].x=+0.5*LX; v[1].y=-0.5*LY; v[1].z=-0.5*LZ;
  v[2].x=+0.5*LX; v[2].y=+0.5*LY; v[2].z=-0.5*LZ;
  v[3].x=-0.5*LX; v[3].y=+0.5*LY; v[3].z=-0.5*LZ;
  geometric_object the_prism=make_prism(m, v, 4, LZ, zhat);

  int write_log=0;

  if (write_log)
   prism2gnuplot(the_prism.subclass.prism_data, "/tmp/test-prism.prism");

  int num_failed_1 = test_point_inclusion(the_block, the_prism, NUMPTS, write_log);
  int num_failed_2 = test_normal_to_object(the_block, the_prism, NUMLINES, write_log);
  int num_failed_3 = test_line_segment_intersection(the_block, the_prism, NUMLINES, write_log);

  return num_failed_1 + num_failed_2 + num_failed_3;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void print_usage(char *msg, int print_usage)
{ 
  if (!msg)
   fprintf(stderr,"%s\n",msg);
  if (print_usage)
   { printf("usage: \n");
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

void quit(char *msg)
{ print_usage(msg, 0); }

void usage(char *msg)
{ print_usage(msg, 1); }

/************************************************************************/
/************************************************************************/
/************************************************************************/
int main(int argc, char *argv[])
{
  srand(time(NULL));
  geom_initialize();

  if (argc<=1) // if no arguments, run unit tests
   return run_unit_tests();

  /***************************************************************/
  /* process arguments *******************************************/
  /***************************************************************/
  char *vertexfile=0;
  vector3 axis={0,0,1};
  double height=1.5;
  vector3 test_point={0,0,0};
  vector3 test_dir={0,0,1};
  double a = 0.2, b=0.3; 
  int narg;
  for(narg=1; narg<argc-1; narg++)
   { if (!strcmp(argv[narg],"--vertexfile"))
      vertexfile=argv[++narg];
     else if (!strcmp(argv[narg],"--axis"))
      { if (narg+3>=argc) usage("too few arguments to --axis");
        sscanf(argv[narg+1],"%le",&(axis.x));
        sscanf(argv[narg+2],"%le",&(axis.y));
        sscanf(argv[narg+3],"%le",&(axis.z));
        narg+=3;
      }
     else if (!strcmp(argv[narg],"--point"))
      { if (narg+3>=argc) usage("too few arguments to --point");
        sscanf(argv[narg+1],"%le",&(test_point.x));
        sscanf(argv[narg+2],"%le",&(test_point.y));
        sscanf(argv[narg+3],"%le",&(test_point.z));
        narg+=3;
      }
     else if (!strcmp(argv[narg],"--dir"))
      { if (narg+5>=argc) usage("too few arguments to --lineseg");
        sscanf(argv[narg+1],"%le",&(test_dir.x));
        sscanf(argv[narg+2],"%le",&(test_dir.y));
        sscanf(argv[narg+3],"%le",&(test_dir.z));
        narg+=3;
      }
     else if (!strcmp(argv[narg],"--height"))
      sscanf(argv[++narg],"%le",&height);
     else if (!strcmp(argv[narg],"--a"))
      sscanf(argv[++narg],"%le",&a);
     else if (!strcmp(argv[narg],"--b"))
      sscanf(argv[++narg],"%le",&b);
     else 
      usage("unknown argument");
   }
  if (!vertexfile) usage("no --vertexfile specified");

  /***************************************************************/
  /* read vertices from vertex file and create prism *************/
  /***************************************************************/
  vector3 *vertices=0;
  int num_vertices=0;
  FILE *f=fopen(vertexfile,"r");
  if (!f) usage("could not open vertexfile");
  char Line[100];
  int LineNum=0;
  while( fgets(Line,100,f) )
   { if (Line[0]=='\n' || Line[0]=='#') continue;
      num_vertices++;
     vector3 v;
     if ( 3!=sscanf(Line,"%le %le %le\n",&(v.x),&(v.y),&(v.z)) )
      { fprintf(stderr,"bad vertex on line %i of %s",num_vertices,vertexfile);
        exit(1);
      }
     vertices = (vector3 *)realloc(vertices, num_vertices*sizeof(vector3));
     vertices[num_vertices-1]=v;
   }
  fclose(f);

  geometric_object the_prism=make_prism(NULL, vertices, num_vertices, height, axis);
  prism *prsm=the_prism.subclass.prism_data;
  prism2gmsh(prsm, "test-prism.pp");
  prism2gnuplot(prsm, "test-prism.gp");
  printf("Wrote prism description to GNUPLOT file test-prism.gp.\n");
  printf("Wrote prism description to GMSH file test-prism.geo.\n");

  /***************************************************************/
  /* test point inclusion, normal to object, and line-segment    */
  /* intersection with specified data                            */
  /***************************************************************/
  boolean in_prism=point_in_objectp(test_point,the_prism);
  vector3 nhat=normal_to_object(test_point, the_prism);
  double s= intersect_line_segment_with_object(test_point, test_dir, the_prism, a, b);
  printf("point {%e,%e,%e}: \n",test_point.x,test_point.y,test_point.z);
  printf(" %s prism\n", in_prism ? "in" : "not in");
  printf(" normal to prism: {%e,%e,%e}\n",nhat.x,nhat.y,nhat.z);
  printf(" intersection with line segment {%e,%e,%e} + (%e,%e)*{%e,%e,%e}: %s\n",
           test_point.x, test_point.y, test_point.z, 
           a,b,test_dir.x, test_dir.y, test_dir.z,s);
}
