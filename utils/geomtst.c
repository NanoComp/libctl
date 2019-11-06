#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "ctlgeom.h"

/************************************************************************/

/* return a random number in [0,1]: */
static double mydrand(void)
{
     double d = rand();
     return (d / (double) RAND_MAX);
}

/* return a uniform random number in [a,b] */
static double myurand(double a, double b)
{
     return ((b - a) * mydrand() + a);
}

#define K_PI 3.141592653589793238462643383279502884197

/* return a random unit vector, uniformly distributed over a sphere */
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

double find_edge(geometric_object o, vector3 dir, double max, double tol)
{
     double min = 0;
     if (!(point_in_fixed_objectp(vector3_scale(min, dir), o) &&
	   !point_in_fixed_objectp(vector3_scale(max, dir), o))) {
	  fprintf(stderr, "object out of bounds in find_edge");
	  exit(1);
     }
     do {
	  double d = (min + max) / 2;
          if (point_in_fixed_objectp(vector3_scale(d, dir), o))
	       min = d;
	  else
	       max = d;
     } while (max - min > tol);
     return (min + max) / 2;
}

static vector3 make_vector3(double x, double y, double z)
{
     vector3 v;
     v.x = x; v.y = y; v.z = z;
     return v;
}

/* return a random geometric object, centered at the origin, with
   diameter roughly 1 */
geometric_object random_object(void)
{
     void* m = NULL;
     vector3 c = { 0, 0, 0 };
     geometric_object o;
     switch (rand() % 5) {
	 case 0:
	      o = make_sphere(m, c, myurand(0.5,1.5));
	      break;
	 case 1:
	      o = make_cylinder(m, c, myurand(0.5,1.5), myurand(0.5,1.5),
				random_unit_vector3());
	      break;
	 case 2:
	      o = make_cone(m, c, myurand(0.5,1.5), myurand(0.5,1.5),
			    random_unit_vector3(), myurand(0.5,1.5));
	      break;
	 case 3:
	      o = make_block(m, c,
#if 1
			     random_unit_vector3(),
			     random_unit_vector3(),
			     random_unit_vector3(),
#else
			     make_vector3(1,0,0),
			     make_vector3(0,1,0),
			     make_vector3(0,0,1),
#endif
			     make_vector3(myurand(0.5,1.5),
					  myurand(0.5,1.5),
					  myurand(0.5,1.5)));
	      break;
	 case 4:
	      o = make_ellipsoid(m, c,
				 random_unit_vector3(),
				 random_unit_vector3(),
				 random_unit_vector3(),
				 make_vector3(myurand(0.5,1.5),
					      myurand(0.5,1.5),
					      myurand(0.5,1.5)));
	      break;
     }
     return o;
}

/************************************************************************/

static double z1(double x) { return (x == 0 ? 1.0 : x); }

static double simple_overlap(geom_box b, geometric_object o, double tol)
{
     double d1,d2,d3, x1,x2,x3, olap0 = 0;
     double itol = 1.0 / ((int) (1/tol + 0.5));

     d1 = (b.high.x - b.low.x) * itol;
     d2 = (b.high.y - b.low.y) * itol;
     d3 = (b.high.z - b.low.z) * itol;
     for (x1 = b.low.x + d1*0.5; x1 <= b.high.x; x1 += d1+(b.high.x==b.low.x))
     for (x2 = b.low.y + d2*0.5; x2 <= b.high.y; x2 += d2+(b.high.y==b.low.y))
     for (x3 = b.low.z + d3*0.5; x3 <= b.high.z; x3 += d3+(b.high.z==b.low.z)){
	  vector3 v;
	  v.x = x1; v.y = x2; v.z = x3;
	  olap0 += z1(d1)*z1(d2)*z1(d3) * point_in_fixed_objectp(v, o);
     }
     olap0 /= z1(b.high.x-b.low.x) * z1(b.high.y-b.low.y) * z1(b.high.z-b.low.z);
     return olap0;
}

static double sqr(double x) { return x * x; }

static double simple_ellip_overlap(geom_box b, geometric_object o, double tol)
{
     double d1,d2,d3, x1,x2,x3, c1,c2,c3, w1,w2,w3, olap0 = 0;
     double itol = 1.0 / ((int) (1/tol + 0.5));
     int dim;

     d1 = (b.high.x - b.low.x) * itol;
     d2 = (b.high.y - b.low.y) * itol;
     d3 = (b.high.z - b.low.z) * itol;
     c1 = (b.high.x + b.low.x) * 0.5;
     c2 = (b.high.y + b.low.y) * 0.5;
     c3 = (b.high.z + b.low.z) * 0.5;
     w1 = 2.0 / z1(b.high.x - b.low.x);
     w2 = 2.0 / z1(b.high.y - b.low.y);
     w3 = 2.0 / z1(b.high.z - b.low.z);
     for (x1 = b.low.x + d1*0.5; x1 <= b.high.x; x1 += d1+(b.high.x==b.low.x))
     for (x2 = b.low.y + d2*0.5; x2 <= b.high.y; x2 += d2+(b.high.y==b.low.y))
     for (x3 = b.low.z + d3*0.5; x3 <= b.high.z; x3 += d3+(b.high.z==b.low.z))
	  if (sqr((x1 - c1) * w1) + sqr((x2 - c2) * w2) + sqr((x3 - c3) * w3)
	      < 1.0) {
	       vector3 v;
	       v.x = x1; v.y = x2; v.z = x3;
	       olap0 += z1(d1)*z1(d2)*z1(d3) * point_in_fixed_objectp(v, o);
	  }
     olap0 /= z1(b.high.x-b.low.x) * z1(b.high.y-b.low.y) * z1(b.high.z-b.low.z);
     dim = (b.high.x!=b.low.x) + (b.high.y!=b.low.y) + (b.high.z!=b.low.z);
     olap0 /= dim == 3 ? 3.14159265358979323846 / 6 :
	  (dim == 2 ? 3.14159265358979323846 / 4 : 1);
     return olap0;
}

geometric_object random_object_and_lattice(void)
{
     geometric_object o = random_object();
#if 1
     geometry_lattice.basis1 = random_unit_vector3();
     geometry_lattice.basis2 = random_unit_vector3();
     geometry_lattice.basis3 = random_unit_vector3();
     geom_fix_lattice();
     geom_fix_object_ptr(&o);
#endif
     return o;
}

static const char *object_name(geometric_object o)
{
     switch (o.which_subclass) {
	 case CYLINDER:
	      switch (o.subclass.cylinder_data->which_subclass) {
		  case WEDGE: return "wedge";
		  case CONE: return "cone";
		  case CYLINDER_SELF: return "cylinder";
	      }
	 case SPHERE: return "sphere";
	 case BLOCK:
	      switch (o.subclass.block_data->which_subclass) {
		  case ELLIPSOID: return "ellipsoid";
		  case BLOCK_SELF: return "block";
	      }
	 case PRISM: return "prism";
	 case COMPOUND_GEOMETRIC_OBJECT: return "compound object";
	 default: return "geometric object";
     }
}

void check_overlap(double tol, double olap0, double olap, int dim, geometric_object o, geom_box b)
{
     if (fabs(olap0 - olap) > 2 * tol * fabs(olap)) {
	  fprintf(stderr, "Large error %e in overlap (%g vs. %g) for:\n"
		  "  lattice = (%g,%g,%g), (%g,%g,%g), (%g,%g,%g)\n"
		  "  box = (%g,%g,%g) - (%g,%g,%g)\n",
		  fabs(olap0 - olap) / fabs(olap),
		  olap, olap0,
		  geometry_lattice.basis1.x,
		  geometry_lattice.basis1.y,
		  geometry_lattice.basis1.z,
		  geometry_lattice.basis2.x,
		  geometry_lattice.basis2.y,
		  geometry_lattice.basis2.z,
		  geometry_lattice.basis3.x,
		  geometry_lattice.basis3.y,
		  geometry_lattice.basis3.z,
		  b.low.x, b.low.y, b.low.z,
		  b.high.x, b.high.y, b.high.z);
	  display_geometric_object_info(2, o);
	  /* exit(1); */
     }
     else
	  printf("Got %s %dd overlap %g vs. %g with tol = %e\n",
		 object_name(o), dim,olap,olap0,tol);
}

static void test_overlap(double tol,
			 number (*box_overlap_with_object)
			 (geom_box b, geometric_object o,
			  number tol, integer maxeval),
			 double (*simple_overlap)
			 (geom_box b, geometric_object o, double tol))
{
     geometric_object o = random_object_and_lattice();
     vector3 dir = random_unit_vector3();
     geom_box b;
     double d, olap, olap0;
     int dim;

     b.low = make_vector3(myurand(-1,0), myurand(-1,0), myurand(-1,0));
     b.high = make_vector3(myurand(0,1), myurand(0,1), myurand(0,1));
     d = find_edge(o, dir, 10, tol);
     b.low = vector3_plus(b.low, vector3_scale(d, dir));
     b.high = vector3_plus(b.high, vector3_scale(d, dir));

     dim = rand() % 3 + 1;
     if (dim < 3)
	  b.low.z = b.high.z = 0;
     if (dim < 2)
	  b.low.y = b.high.y = 0;

     olap = box_overlap_with_object(b, o, tol/100, 10000/tol);
     olap0 = simple_overlap(b, o, tol/2);
     check_overlap(tol, olap0, olap, dim, o, b);
     geometric_object_destroy(o);
}

static void test_volume(double tol)
{
     geometric_object o = random_object_and_lattice();
     geom_box b;
     double olap1, olap2;

     geom_get_bounding_box(o, &b);
     olap1 = box_overlap_with_object(b, o, tol/100, 10000/tol);
     b.low.x += 1e-7 * (b.high.x - b.low.x); /* b no longer contains o */
     olap2 = box_overlap_with_object(b, o, tol/100, 10000/tol);
     check_overlap(tol, olap1, olap2, 3, o, b);
     geometric_object_destroy(o);
}


/************************************************************************/

int main(void)
{
     const int ntest = 100;
     const double tol = 1e-2;
     int i;

     srand(time(NULL));

     geom_initialize();

     printf("**** whole box overlap: ****\n");
     for (i = 0; i < ntest; ++i)
          test_volume(tol);
     for (i = 0; i < ntest; ++i) {
          printf("**** box overlap: ****\n");
          test_overlap(tol,
                    box_overlap_with_object,
                    simple_overlap);
          printf("**** ellipsoid overlap: ****\n");
          test_overlap(tol,
                    ellipsoid_overlap_with_object,
                    simple_ellip_overlap);
     }

     return 0;
}

