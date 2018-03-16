/************************************************************************/
/* test-prism.c: unit test for prisms in libctlgeom                     */
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

static vector3 make_vector3(double x, double y, double z)
{
  vector3 v;
  v.x = x; v.y = y; v.z = z;
  return v;
}

boolean point_in_polygon(double px, double py, vector3 *vertices, int num_vertices);

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
  for(int nv=0; nv<num_vertices; nv++)
   {  
     vector3 vap = vertices[nv]; vap.z = -0.5*height;
     vector3 vbp = vertices[nv]; vbp.z = +0.5*height;
     vector3 vcp = vertices[(nv+1)%num_vertices]; vcp.z = +0.5*height;
     vector3 vdp = vertices[(nv+1)%num_vertices]; vdp.z = -0.5*height;
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
   };
  fclose(f);
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
void prism2gmsh(prism *prsm, char *filename)
{ 
  vector3 *vertices = prsm->vertices.items;
  int num_vertices  = prsm->vertices.num_items;
  double height     = prsm->height;
  vector3 zhat      = prsm->m_p2c.c2;
  vector3 axis      = vector3_scale(height, zhat);

  FILE *f=fopen(filename,"w");
  for(int nv=0; nv<num_vertices; nv++)
   { vector3 vp=vertices[nv];
     vector3 vc=prism_coordinate_p2c(prsm, vp);
     fprintf(f,"Point(%i)={%e, %e, %e};\n",nv,vc.x,vc.y,vc.z);
   }
  for(int nv=0; nv<num_vertices; nv++)
   fprintf(f,"Line(%i)={%i, %i};\n",nv,nv,(nv+1)%num_vertices);
  fprintf(f,"Line Loop(0)={0");
  for(int nv=1; nv<num_vertices; nv++)
   fprintf(f,",%i",nv);
  fprintf(f,"};\n");
  fprintf(f,"Plane Surface(0)={0};\n");
  fprintf(f,"Extrude { %e,%e,%e } { Surface{0}; }\n",height*zhat.x,height*zhat.y,height*zhat.z);
  fclose(f); 
  
}

/************************************************************************/
/* first unit test: create the same parallelepiped in two ways, as a    */
/*            block and as a prism, then check inclusion of 10000       */
/*            randomly generated points and confirm the results agree.  */
/************************************************************************/
int unit_test1()
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

  int num_failed=0;
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
  
  printf("%i/%i points failed\n",num_failed,NUMPTS);
  return num_failed;
}

/************************************************************************/
/* second unit test: create the same parallelepiped in two ways, as a   */
/*            block and as a prism, then check intersections of 1000    */
/*            randomly generated lines with the objects and confirm the */
/*            results agree.                                            */
/************************************************************************/
int unit_test2()
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

  int num_failed=0;
  vector3 min_corner = vector3_scale(-1.0, size);
  vector3 max_corner = vector3_scale(+1.0, size);
  FILE *f=fopen("/tmp/test-prism.out","w");
  for(int n=0; n<NUMPTS; n++)
   { 
     // randomly generated base point within enlarged bounding box
     vector3 p = random_point(min_corner, max_corner);
     // randomly generated 3D unit vector 
     double CosTheta = myurand(-1.0, 1.0);
     double SinTheta = sqrt(1.0-CosTheta*CosTheta);
     double Phi      = myurand(0.0, 2.0*K_PI);
     vector3 d;
     d.x = SinTheta*cos(Phi);
     d.y = SinTheta*sin(Phi);
     d.z = CosTheta;

     double s_block[2];
     int ns_block=intersect_line_with_object(p, d, the_block, s_block);

     double s_prism[2];
     int ns_prism=intersect_line_with_object(p, d, the_prism, s_prism);

     int match = (ns_block==ns_prism) ? 1 : 0;
     for(int n=0; match==1 && n<ns_block; n++)
      if ( fabs(s_block[n] - s_prism[n]) > 1.0e-7 )
       match=0;

     if (match==0)
      num_failed++;
   }
  fclose(f);
  
  printf("%i/%i lines failed\n",num_failed,NUMLINES);
  return num_failed;
}

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
     printf(" --pointfile  MyPointFile\n");
   };
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
   { 
     int num_failed_1 = unit_test1();
     int num_failed_2 = unit_test2();
     return num_failed_1 + num_failed_2;
   }

  /***************************************************************/
  /* process arguments *******************************************/
  /***************************************************************/
  vector3 *vertices=0;
  int num_vertices=0;
  char *vertexfile=0, *pointfile=0;
  vector3 zhat={0,0,1};
  double height=1.0;
  vector3 test_point={0,0,0}; int have_test_point=0;
  for(int narg=1; narg<argc-1; narg++)
   { if (!strcmp(argv[narg],"--vertexfile"))
      vertexfile=argv[++narg];
     else if (!strcmp(argv[narg],"--axis"))
      { if (narg+3>=argc) usage("too few arguments to --axis");
        sscanf(argv[narg+1],"%le",&(zhat.x));
        sscanf(argv[narg+2],"%le",&(zhat.y));
        sscanf(argv[narg+3],"%le",&(zhat.z));
        narg+=3;
      }
     else if (!strcmp(argv[narg],"--point"))
      { if (narg+3>=argc) usage("too few arguments to --point");
        sscanf(argv[narg+1],"%le",&(test_point.x));
        sscanf(argv[narg+2],"%le",&(test_point.y));
        sscanf(argv[narg+3],"%le",&(test_point.z));
        have_test_point=1;
        narg+=3;
      }
     else if (!strcmp(argv[narg],"--height"))
      sscanf(argv[++narg],"%le",&height);
     else if (!strcmp(argv[narg],"--pointfile"))
      pointfile=argv[++narg];
     else 
      usage("unknown argument");
   }

  /***************************************************************/
  /* read vertices from file and create prism ********************/
  /***************************************************************/
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
   };
  fclose(f);

  geometric_object the_prism=make_prism(NULL, vertices, num_vertices, height, zhat);
  prism *prsm=the_prism.subclass.prism_data;
  prism2gmsh(prsm, "test-prism.geo");
  prism2gnuplot(prsm, "test-prism.gp");


  /***************************************************************/
  /* read points from file or generate random points and check   */
  /* inclusion                                                   */
  /***************************************************************/
  geom_box box;
  get_prism_bounding_box(prsm, &box);
  f=fopen("prism-box.gp","w");
  fprintf(f,"%e %e %e \n",box.low.x,box.low.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.low.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.high.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.high.y,box.low.z);
  fprintf(f,"\n\n");

  fprintf(f,"%e %e %e \n",box.low.x,box.low.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.low.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.high.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.high.y,box.high.z);
  fprintf(f,"\n\n");

  fprintf(f,"%e %e %e \n",box.low.x,box.low.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.low.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.low.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.low.y,box.high.z);
  fprintf(f,"\n\n");

  fprintf(f,"%e %e %e \n",box.low.x,box.high.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.high.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.high.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.high.y,box.high.z);
  fprintf(f,"\n\n");

  fprintf(f,"%e %e %e \n",box.low.x,box.low.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.high.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.high.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.low.x,box.low.y,box.high.z);
  fprintf(f,"\n\n");

  fprintf(f,"%e %e %e \n",box.high.x,box.low.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.high.y,box.low.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.high.y,box.high.z);
  fprintf(f,"%e %e %e \n",box.high.x,box.low.y,box.high.z);
  fprintf(f,"\n\n");
  fclose(f);

  vector3 delta = vector3_minus(box.high, box.low);
  box.high = vector3_plus(box.high,delta);
  box.low  = vector3_minus(box.low,delta);

  f = (pointfile ? fopen(pointfile,"r") : 0);

  FILE *gpfile1  = fopen("test-prism.in.gp","w");
  FILE *gpfile2  = fopen("test-prism.out.gp","w");
  FILE *ppfile1  = fopen("test-prism.in.pp","w");
  FILE *ppfile2  = fopen("test-prism.out.pp","w");
  fprintf(ppfile1,"View \"Points inside prism\" {\n");
  fprintf(ppfile2,"View \"Points outside prism\" {\n");
  int Done=0;
  int NumPoints=0;
  while(!Done)
   {  
     vector3 v;
     if (f)
      { char Line[100];
        NumPoints++;
        if (fgets(Line,100,f))
         sscanf(Line,"%le %le %le\n",&(v.x),&(v.y),&(v.z));
        else
         Done=1;
      }
     else if (have_test_point)
      { v=test_point;
        Done=1;
      }
     else
      { v.x = myurand(box.low.x, box.high.x);
        v.y = myurand(box.low.y, box.high.y);
        v.z = myurand(box.low.z, box.high.z);
        if (++NumPoints == NUMPTS ) Done=1;
      }

     // vector3 vp=prism_coordinate_c2p(prsm,v);
     // vp.z=0;
     // if (point_in_polygon(vp.x,vp.y,prsm->vertices.items,num_vertices))
     if (point_in_objectp(v,the_prism))
      { fprintf(gpfile1,"%e %e %e\n",v.x,v.y,v.z);
        fprintf(ppfile1,"SP(%e,%e,%e) {0};\n",v.x,v.y,v.z);
        if (have_test_point) printf("Point in object.\n");
      }
     else
      { fprintf(gpfile2,"%e %e %e\n",v.x,v.y,v.z);
        fprintf(ppfile2,"SP(%e,%e,%e) {0};\n",v.x,v.y,v.z);
        if (have_test_point) printf("Point not in object.\n");
      }
   }
  fprintf(ppfile1,"};\n");
  fprintf(ppfile2,"};\n");
  fclose(gpfile1);
  fclose(gpfile2);
  fclose(ppfile1);
  fclose(ppfile2);
}
