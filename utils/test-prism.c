/************************************************************************/
/* test-prism.c: unit test for prisms in libctlgeom                     */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "ctlgeom.h"

#define K_PI 3.141592653589793238462643383279502884197

static vector3 make_vector3(double x, double y, double z)
{
  vector3 v;
  v.x = x; v.y = y; v.z = z;
  return v;
}

/************************************************************************/
/* return a uniform random number in [a,b] */
/************************************************************************/
static double myurand(double a, double b)
{
  return a + (b-a)*(rand()/((double)RAND_MAX));
}

static double mydrand() 
{ return myurand(0.0,1.0); }

/************************************************************************/
/* return a random unit vector, uniformly distributed over a parallelepiped */
/************************************************************************/
vector3 random_point(vector3 min_corner, vector3 max_corner)
{ return make_vector3( myurand(min_corner.x, max_corner.x),
                       myurand(min_corner.y, max_corner.y),
                       myurand(min_corner.z, max_corner.z)
                     );
}

/************************************************************************/
/* return a random unit vector, uniformly distributed over a sphere */
/************************************************************************/
vector3 random_unit_vector3(void)
{
     double z, t, r;
     vector3 v;

     z = 2*mydrand() - 1;
     t = 2*K_PI*mydrand();
     r = sqrt(1 - z*z);
     v.x = r * cos(t);
     v.y = r * sin(t);
     v.z = z;
     return v;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void draw_prism(geometric_object o, char *filename)
{ 
  vector3 center    = o.center;
  prism *prsm       = o.subclass.prism_data;
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;
  matrix3x3 m_c2p   = prsm->m_c2p;
  matrix3x3 m_p2c   = matrix3x3_inverse(m_c2p);

  FILE *f=fopen(filename,"w");
  for(int nv=0; nv<num_vertices; nv++)
   {  
     vector3 va = vertices[nv]; va.z = -0.5*height;
     vector3 vb = vertices[nv]; vb.z = +0.5*height;
     vector3 vc = vertices[(nv+1)%num_vertices]; vc.z = +0.5*height;
     vector3 vd = vertices[(nv+1)%num_vertices]; vd.z = -0.5*height;
     va=vector3_plus(center, matrix3x3_vector3_mult(m_p2c,va));
     vb=vector3_plus(center, matrix3x3_vector3_mult(m_p2c,vb));
     vc=vector3_plus(center, matrix3x3_vector3_mult(m_p2c,vc));
     vd=vector3_plus(center, matrix3x3_vector3_mult(m_p2c,vd));
     fprintf(f,"%e %e %e \n",va.x,va.y,va.z);
     fprintf(f,"%e %e %e \n",vb.x,vb.y,vb.z);
     fprintf(f,"%e %e %e \n",vc.x,vc.y,vc.z);
     fprintf(f,"%e %e %e \n",vd.x,vd.y,vd.z);
     fprintf(f,"%e %e %e \n",va.x,va.y,va.z);
     fprintf(f,"\n\n");
   };
  fclose(f);
}

/************************************************************************/
/* first test: create the same parallelepiped in two ways, as a block   */
/*             and as a prism, then check inclusion of 100 randomly     */
/*             generated points                                         */
/************************************************************************/
int test1()
{
  void* m = NULL;
  vector3 c = { 0, 0, 0 };
  vector3 xhat = make_vector3(1,0,0);
  vector3 yhat = make_vector3(0,1,0);
  vector3 zhat = make_vector3(0,0,1);
  vector3 size = make_vector3(0.5,1.0,1.5);
  geometric_object the_block = make_block(m, c, xhat, yhat, zhat, size);

  vector3 v[4];
  v[0].x=-0.25; v[0].y=-0.5; v[0].z=-0.75;
  v[1].x=+0.25; v[1].y=-0.5; v[1].z=-0.75;
  v[2].x=+0.25; v[2].y=+0.5; v[2].z=-0.75;
  v[3].x=-0.25; v[3].y=+0.5; v[3].z=-0.75;
  geometric_object the_prism=make_prism(m, v, 4, 1.5, zhat);
  draw_prism(the_prism,"the_prism.out");

  int num_failed=0;
#define NUMPTS 10000
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f=fopen("/tmp/test-prism.out","w");
  for(int n=0; n<NUMPTS; n++)
   { 
     vector3 p = random_point(min_corner, max_corner);
     boolean in_block=point_in_objectp(p,the_block);
     boolean in_prism=point_in_objectp(p,the_prism);
     if (in_block!=in_prism)
      num_failed++;
     fprintf(f,"%i %i %e %e %e \n",in_block,in_prism,p.x,p.y,p.z);
   };
  fclose(f);
  
  printf("%i/%i points failed",num_failed,NUMPTS);
  return num_failed;
}

/************************************************************************/
int main(void)
{
  srand(time(NULL));
  geom_initialize();
  int num_failed = test1();
  return num_failed;
}
