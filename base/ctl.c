/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, Steven G. Johnson
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ctl.h"

/**************************************************************************/

/* Functions missing from Guile 1.2: */

#ifndef HAVE_GH_BOOL2SCM
/* Guile 1.2 is missing gh_bool2scm for some reason; redefine: */
SCM bool2scm(boolean b) { return (b ? SCM_BOOL_T : SCM_BOOL_F); }
#endif

#ifndef HAVE_GH_LENGTH
#define gh_length gh_list_length
#endif

#ifndef HAVE_GH_LIST_REF
/* Guile 1.2 doesn't have the gh_list_ref function.  Sigh. */
/* Note: index must be in [0,list_length(l) - 1].  We don't check! */
static SCM list_ref(list l, int index)
{
  SCM cur = SCM_UNSPECIFIED, rest = l;

  while (index >= 0) {
    cur = gh_car(rest);
    rest = gh_cdr(rest);
    --index;
  }
  return cur;
}

#else /* HAVE_GH_LIST_REF */
#define list_ref(l,index) gh_list_ref(l,gh_int2scm(index))
#endif

#ifndef HAVE_GH_VECTOR_REF
#define gh_vector_ref gh_vref
#endif

/**************************************************************************/

/* Scheme file loading (don't use gh_load directly because subsequent
   loads won't use the correct path name).  Uses our "include" function
   from include.scm, or defaults to gh_load if this function isn't
   defined. */

void ctl_include(char *filename)
{
  SCM include_proc = gh_lookup("include");
  if (include_proc == SCM_UNDEFINED)
    gh_load(filename);
  else
    gh_call1(include_proc, gh_str02scm(filename));
}

/* convert a pathname into one relative to the current include dir */
char *ctl_fix_path(const char *path)
{
     char *newpath;
     if (path[0] != '/') {
	  SCM include_dir = gh_lookup("include-dir");
	  if (include_dir != SCM_UNDEFINED) {
	       char *dir = gh_scm2newstr(include_dir, NULL);
	       newpath = (char *) malloc(sizeof(char) * (strlen(dir) + 
							 strlen(path) + 2));
	       strcpy(newpath, dir);
	       free(dir);
	       if (newpath[0] && newpath[strlen(newpath)-1] != '/')
		    strcat(newpath, "/");
	       strcat(newpath, path);
	       return newpath;
	  }
     }
     newpath = (char *) malloc(sizeof(char) * (strlen(path) + 1));
     strcpy(newpath, path);
     return newpath;
}

/**************************************************************************/

/* vector3 and matrix3x3 utilities: */

number vector3_dot(vector3 v1,vector3 v2)
{
  return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

number vector3_norm(vector3 v)
{
  return (sqrt(vector3_dot(v,v)));
}

vector3 vector3_scale(number s, vector3 v)
{
  vector3 vnew;

  vnew.x = s * v.x;
  vnew.y = s * v.y;
  vnew.z = s * v.z;
  return vnew;
}

vector3 unit_vector3(vector3 v)
{
  number norm = vector3_norm(v);
  if (norm < 1.0e-15)
    return v;
  else
    return vector3_scale(1.0/norm, v);
}

vector3 vector3_plus(vector3 v1,vector3 v2)
{
  vector3 vnew;

  vnew.x = v1.x + v2.x;
  vnew.y = v1.y + v2.y;
  vnew.z = v1.z + v2.z;
  return vnew;
}

vector3 vector3_minus(vector3 v1,vector3 v2)
{
  vector3 vnew;

  vnew.x = v1.x - v2.x;
  vnew.y = v1.y - v2.y;
  vnew.z = v1.z - v2.z;
  return vnew;
}

vector3 vector3_cross(vector3 v1,vector3 v2)
{
  vector3 vnew;

  vnew.x = v1.y * v2.z - v2.y * v1.z;
  vnew.y = v1.z * v2.x - v2.z * v1.x;
  vnew.z = v1.x * v2.y - v2.x * v1.y;
  return vnew;
}

int vector3_equal(vector3 v1, vector3 v2)
{
     return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
}

vector3 matrix3x3_vector3_mult(matrix3x3 m, vector3 v)
{
  vector3 vnew;

  vnew.x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z;
  vnew.y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z;
  vnew.z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z;
  return vnew;
}

matrix3x3 matrix3x3_mult(matrix3x3 m1, matrix3x3 m2)
{
  matrix3x3 m;

  m.c0.x = m1.c0.x * m2.c0.x + m1.c1.x * m2.c0.y + m1.c2.x * m2.c0.z;
  m.c0.y = m1.c0.y * m2.c0.x + m1.c1.y * m2.c0.y + m1.c2.y * m2.c0.z;
  m.c0.z = m1.c0.z * m2.c0.x + m1.c1.z * m2.c0.y + m1.c2.z * m2.c0.z;

  m.c1.x = m1.c0.x * m2.c1.x + m1.c1.x * m2.c1.y + m1.c2.x * m2.c1.z;
  m.c1.y = m1.c0.y * m2.c1.x + m1.c1.y * m2.c1.y + m1.c2.y * m2.c1.z;
  m.c1.z = m1.c0.z * m2.c1.x + m1.c1.z * m2.c1.y + m1.c2.z * m2.c1.z;

  m.c2.x = m1.c0.x * m2.c2.x + m1.c1.x * m2.c2.y + m1.c2.x * m2.c2.z;
  m.c2.y = m1.c0.y * m2.c2.x + m1.c1.y * m2.c2.y + m1.c2.y * m2.c2.z;
  m.c2.z = m1.c0.z * m2.c2.x + m1.c1.z * m2.c2.y + m1.c2.z * m2.c2.z;

  return m;
}

matrix3x3 matrix3x3_transpose(matrix3x3 m)
{
     matrix3x3 mt;
    
     mt.c0.x = m.c0.x;
     mt.c1.x = m.c0.y;
     mt.c2.x = m.c0.z;
     mt.c0.y = m.c1.x;
     mt.c1.y = m.c1.y;
     mt.c2.y = m.c1.z;
     mt.c0.z = m.c2.x;
     mt.c1.z = m.c2.y;
     mt.c2.z = m.c2.z;
     return mt;
}

number matrix3x3_determinant(matrix3x3 m)
{
     return(m.c0.x*m.c1.y*m.c2.z - m.c2.x*m.c1.y*m.c0.z +
	    m.c1.x*m.c2.y*m.c0.z + m.c0.y*m.c1.z*m.c2.x -
	    m.c1.x*m.c0.y*m.c2.z - m.c2.y*m.c1.z*m.c0.x);
}

matrix3x3 matrix3x3_inverse(matrix3x3 m)
{
     matrix3x3 minv;
     number detinv = matrix3x3_determinant(m);

     if (detinv == 0.0) {
	  fprintf(stderr, "error: singular matrix in matrix3x3_inverse!\n");
	  exit(EXIT_FAILURE);
     }
     detinv = 1.0/detinv;

     minv.c0.x = detinv * (m.c1.y * m.c2.z - m.c2.y * m.c1.z);
     minv.c1.y = detinv * (m.c0.x * m.c2.z - m.c2.x * m.c0.z);
     minv.c2.z = detinv * (m.c1.y * m.c0.x - m.c0.y * m.c1.x);
     
     minv.c0.z = detinv * (m.c0.y * m.c1.z - m.c1.y * m.c0.z);
     minv.c0.y = -detinv * (m.c0.y * m.c2.z - m.c2.y * m.c0.z);
     minv.c1.z = -detinv * (m.c0.x * m.c1.z - m.c1.x * m.c0.z);
     
     minv.c2.x = detinv * (m.c1.x * m.c2.y - m.c1.y * m.c2.x);
     minv.c1.x = -detinv * (m.c1.x * m.c2.z - m.c1.z * m.c2.x);
     minv.c2.y = -detinv * (m.c0.x * m.c2.y - m.c0.y * m.c2.x);

     return minv;
}

int matrix3x3_equal(matrix3x3 m1, matrix3x3 m2)
{
     return (vector3_equal(m1.c0, m2.c0)
	     && vector3_equal(m1.c1, m2.c1)
	     && vector3_equal(m1.c2, m2.c2));
}

/**************************************************************************/

/* complex number utilities */

cnumber make_cnumber(number r, number i)
{
     cnumber c;
     c.re = r; c.im = i;
     return c;
}

cnumber cnumber_conj(cnumber c)
{
     return make_cnumber(c.re, -c.im);
}

int cnumber_equal(cnumber c1, cnumber c2)
{
     return (c1.re == c2.re && c1.im == c2.im);
}

vector3 cvector3_re(cvector3 cv)
{
     vector3 v;
     v.x = cv.x.re; v.y = cv.y.re; v.z = cv.z.re;
     return v;
}

vector3 cvector3_im(cvector3 cv)
{
     vector3 v;
     v.x = cv.x.im; v.y = cv.y.im; v.z = cv.z.im;
     return v;
}

cvector3 make_cvector3(vector3 vr, vector3 vi)
{
     cvector3 cv;
     cv.x = make_cnumber(vr.x, vi.x);
     cv.y = make_cnumber(vr.y, vi.y);
     cv.z = make_cnumber(vr.z, vi.z);
     return cv;
}

int cvector3_equal(cvector3 v1, cvector3 v2)
{
     return (vector3_equal(cvector3_re(v1), cvector3_re(v2)) &&
	     vector3_equal(cvector3_im(v1), cvector3_im(v2)));
}

matrix3x3 cmatrix3x3_re(cmatrix3x3 cm)
{
     matrix3x3 m;
     m.c0 = cvector3_re(cm.c0);
     m.c1 = cvector3_re(cm.c1);
     m.c2 = cvector3_re(cm.c2);
     return m;
}

matrix3x3 cmatrix3x3_im(cmatrix3x3 cm)
{
     matrix3x3 m;
     m.c0 = cvector3_im(cm.c0);
     m.c1 = cvector3_im(cm.c1);
     m.c2 = cvector3_im(cm.c2);
     return m;
}

cmatrix3x3 make_cmatrix3x3(matrix3x3 mr, matrix3x3 mi)
{
     cmatrix3x3 cm;
     cm.c0 = make_cvector3(mr.c0, mi.c0);
     cm.c1 = make_cvector3(mr.c1, mi.c1);
     cm.c2 = make_cvector3(mr.c2, mi.c2);
     return cm;
}

cmatrix3x3 make_hermitian_cmatrix3x3(number m00, number m11, number m22,
				     cnumber m01, cnumber m02, cnumber m12)
{
     cmatrix3x3 cm;
     cm.c0.x = make_cnumber(m00, 0);
     cm.c1.y = make_cnumber(m11, 0);
     cm.c2.z = make_cnumber(m22, 0);
     cm.c1.x = m01; cm.c0.y = cnumber_conj(m01);
     cm.c2.x = m02; cm.c0.z = cnumber_conj(m02);
     cm.c2.y = m12; cm.c1.z = cnumber_conj(m12);
     return cm;
}

int cmatrix3x3_equal(cmatrix3x3 m1, cmatrix3x3 m2)
{
     return (matrix3x3_equal(cmatrix3x3_re(m1), cmatrix3x3_re(m2)) &&
	     matrix3x3_equal(cmatrix3x3_im(m1), cmatrix3x3_im(m2)));
}

/**************************************************************************/

/* type conversion */

vector3 scm2vector3(SCM sv)
{
  vector3 v;

  v.x = gh_scm2double(gh_vector_ref(sv,gh_int2scm(0)));
  v.y = gh_scm2double(gh_vector_ref(sv,gh_int2scm(1)));
  v.z = gh_scm2double(gh_vector_ref(sv,gh_int2scm(2)));
  return v;
}

matrix3x3 scm2matrix3x3(SCM sm)
{
  matrix3x3 m;

  m.c0 = scm2vector3(gh_vector_ref(sm,gh_int2scm(0)));
  m.c1 = scm2vector3(gh_vector_ref(sm,gh_int2scm(1)));
  m.c2 = scm2vector3(gh_vector_ref(sm,gh_int2scm(2)));
  return m;
}

static SCM make_vector3(SCM x, SCM y, SCM z)
{
  SCM vscm, *data;
  vscm = scm_c_make_vector(3, SCM_UNSPECIFIED);
  data = SCM_VELTS(vscm);
  data[0] = x;
  data[1] = y;
  data[2] = z;
  return vscm;
}

SCM vector32scm(vector3 v)
{
  return make_vector3(gh_double2scm(v.x),
		      gh_double2scm(v.y), 
		      gh_double2scm(v.z));
}

SCM matrix3x32scm(matrix3x3 m)
{
  return make_vector3(vector32scm(m.c0),
		      vector32scm(m.c1),
		      vector32scm(m.c2));
}

cnumber scm2cnumber(SCM sx)
{
#ifdef HAVE_SCM_COMPLEXP
     if (scm_real_p(sx) && !(SCM_COMPLEXP(sx)))
	  return make_cnumber(gh_scm2double(sx), 0.0);
     else
	  return make_cnumber(SCM_COMPLEX_REAL(sx), SCM_COMPLEX_IMAG(sx));
#else
     if (scm_real_p(sx) && !(SCM_NIMP(sx) && SCM_INEXP(sx) && SCM_CPLXP(sx)))
	  return make_cnumber(gh_scm2double(sx), 0.0);
     else
	  return make_cnumber(SCM_REALPART(sx), SCM_IMAG(sx));
#endif
}

SCM cnumber2scm(cnumber x)
{
#ifdef HAVE_SCM_MAKE_COMPLEX
     return scm_make_complex(x.re, x.im); /* Guile 1.5 */
#else
     if (x.im == 0.0)
	  return gh_double2scm(x.re);
     else
	  return scm_makdbl(x.re, x.im);
#endif
}

cvector3 scm2cvector3(SCM sv)
{
     cvector3 v;

     v.x = scm2cnumber(gh_vector_ref(sv,gh_int2scm(0)));
     v.y = scm2cnumber(gh_vector_ref(sv,gh_int2scm(1)));
     v.z = scm2cnumber(gh_vector_ref(sv,gh_int2scm(2)));
     return v;
}

cmatrix3x3 scm2cmatrix3x3(SCM sm)
{
     cmatrix3x3 m;

     m.c0 = scm2cvector3(gh_vector_ref(sm,gh_int2scm(0)));
     m.c1 = scm2cvector3(gh_vector_ref(sm,gh_int2scm(1)));
     m.c2 = scm2cvector3(gh_vector_ref(sm,gh_int2scm(2)));
     return m;
}

SCM cvector32scm(cvector3 v)
{
  return make_vector3(cnumber2scm(v.x),
		      cnumber2scm(v.y), 
		      cnumber2scm(v.z));
}

SCM cmatrix3x32scm(cmatrix3x3 m)
{
  return make_vector3(cvector32scm(m.c0),
		      cvector32scm(m.c1),
		      cvector32scm(m.c2));
}

/**************************************************************************/

/* variable get/set functions */

/**** Getters ****/

integer ctl_get_integer(char *identifier)
{
  return(gh_scm2int(gh_lookup(identifier)));
}

number ctl_get_number(char *identifier)
{
  return(gh_scm2double(gh_lookup(identifier)));
}

cnumber ctl_get_cnumber(char *identifier)
{
  return(scm2cnumber(gh_lookup(identifier)));
}

boolean ctl_get_boolean(char *identifier)
{
  return(gh_scm2bool(gh_lookup(identifier)));
}

char* ctl_get_string(char *identifier)
{
  return(gh_scm2newstr(gh_lookup(identifier), NULL));
}

vector3 ctl_get_vector3(char *identifier)
{
  return(scm2vector3(gh_lookup(identifier)));
}

matrix3x3 ctl_get_matrix3x3(char *identifier)
{
  return(scm2matrix3x3(gh_lookup(identifier)));
}

cvector3 ctl_get_cvector3(char *identifier)
{
  return(scm2cvector3(gh_lookup(identifier)));
}

cmatrix3x3 ctl_get_cmatrix3x3(char *identifier)
{
  return(scm2cmatrix3x3(gh_lookup(identifier)));
}

list ctl_get_list(char *identifier)
{
  return(gh_lookup(identifier));
}

object ctl_get_object(char *identifier)
{
  return(gh_lookup(identifier));
}

function ctl_get_function(char *identifier)
{
  return(gh_lookup(identifier));
}

SCM ctl_get_SCM(char *identifier)
{
  return(gh_lookup(identifier));
}

/**** Setters ****/

/* UGLY hack alert!  There doesn't seem to be any clean way of setting
   Scheme variables from C in Guile (e.g. no gh_* interface).

   One option is to use scm_symbol_set_x (symbol-set! in Scheme), but
   I'm not sure how to get this to work in Guile 1.3 because of the
   %&*@^-ing module system (I need to pass some module for the first
   parameter, but I don't know what to pass).

   Instead, I hacked together the following my_symbol_set_x routine,
   using the functions scm_symbol_value0 and scm_symbol_set_x from the
   Guile 1.3 sources. (scm_symbol_value0 has the virtue of looking in
   the correct module somehow; I also used this function to replace
   gh_lookup, which broke in Guile 1.3 as well...sigh.)

   Note that I can't call "set!" because it is really a macro. 

   All the ugliness is confined to the set_value() routine, though.  

   Update: in Guile 1.5, we can call scm_variable_set_x (equivalent
   to variable-set!) to set values of variables, which are looked up
   via scm_c_lookup (which doesn't exist in Guile 1.3.x). */

#if !(defined(HAVE_SCM_VARIABLE_SET_X) && defined(HAVE_SCM_C_LOOKUP))
#  define USE_MY_SYMBOL_SET_X 1   /* use the hack */
#endif

#ifdef USE_MY_SYMBOL_SET_X
static SCM my_symbol_set_x(char *name, SCM v)
{
     /* code swiped from scm_symbol_value0 and scm_symbol_set_x */
     SCM symbol = scm_intern_obarray_soft(name, strlen (name), scm_symhash, 0);
     SCM vcell = scm_sym2vcell (SCM_CAR (symbol),
                                SCM_CDR (scm_top_level_lookup_closure_var),
                                SCM_BOOL_F);
     if (SCM_FALSEP (vcell))
          return SCM_UNDEFINED;
     SCM_SETCDR (vcell, v);
     return SCM_UNSPECIFIED;
}
#endif

static void set_value(char *identifier, SCM value)
{
#if defined(USE_SCM_SYMBOL_SET_X)   /* worked in Guile 1.1, 1.2 */
     scm_symbol_set_x(SCM_BOOL_F, gh_symbol2scm(identifier), value);
#elif defined(HAVE_SCM_VARIABLE_SET_X) && defined(HAVE_SCM_C_LOOKUP)
     scm_variable_set_x(scm_c_lookup(identifier), value);
#elif defined(USE_MY_SYMBOL_SET_X)
     my_symbol_set_x(identifier, value);
#endif
}

void ctl_set_integer(char *identifier, integer value)
{
  set_value(identifier, gh_int2scm(value));
}

void ctl_set_number(char *identifier, number value)
{
  set_value(identifier, gh_double2scm(value));
}

void ctl_set_cnumber(char *identifier, cnumber value)
{
  set_value(identifier, cnumber2scm(value));
}

void ctl_set_boolean(char *identifier, boolean value)
{
  set_value(identifier, gh_bool2scm(value));
}

void ctl_set_string(char *identifier, char *value)
{
  set_value(identifier, gh_str02scm(value));
}

void ctl_set_vector3(char *identifier, vector3 value)
{
  set_value(identifier, vector32scm(value));
}

void ctl_set_matrix3x3(char *identifier, matrix3x3 value)
{
  set_value(identifier, matrix3x32scm(value));
}

void ctl_set_cvector3(char *identifier, cvector3 value)
{
  set_value(identifier, cvector32scm(value));
}

void ctl_set_cmatrix3x3(char *identifier, cmatrix3x3 value)
{
  set_value(identifier, cmatrix3x32scm(value));
}

void ctl_set_list(char *identifier, list value)
{
  set_value(identifier, value);
}

void ctl_set_object(char *identifier, object value)
{
  set_value(identifier, value);
}

void ctl_set_function(char *identifier, function value)
{
  set_value(identifier, value);
}

void ctl_set_SCM(char *identifier, SCM value)
{
  set_value(identifier, value);
}

/**************************************************************************/

/* list traversal */

int list_length(list l)
{
  return(gh_length(l));
}

integer integer_list_ref(list l, int index)
{
  return(gh_scm2int(list_ref(l,index)));
}

number number_list_ref(list l, int index)
{
  return(gh_scm2double(list_ref(l,index)));
}

cnumber cnumber_list_ref(list l, int index)
{
  return(scm2cnumber(list_ref(l,index)));
}

boolean boolean_list_ref(list l, int index)
{
  return(SCM_BOOL_F != list_ref(l,index));
}

char* string_list_ref(list l, int index)
{
  return(gh_scm2newstr(list_ref(l,index),NULL));
}

vector3 vector3_list_ref(list l, int index)
{
  return(scm2vector3(list_ref(l,index)));
}

matrix3x3 matrix3x3_list_ref(list l, int index)
{
  return(scm2matrix3x3(list_ref(l,index)));
}

cvector3 cvector3_list_ref(list l, int index)
{
  return(scm2cvector3(list_ref(l,index)));
}

cmatrix3x3 cmatrix3x3_list_ref(list l, int index)
{
  return(scm2cmatrix3x3(list_ref(l,index)));
}

list list_list_ref(list l, int index)
{
  return(list_ref(l,index));
}

object object_list_ref(list l, int index)
{
  return(list_ref(l,index));
}

function function_list_ref(list l, int index)
{
  return(list_ref(l,index));
}

SCM SCM_list_ref(list l, int index)
{
  return(list_ref(l,index));
}

/**************************************************************************/

/* list creation */

#define MAKE_LIST(conv) \
{ \
  int i; \
  list cur_list = SCM_EOL; \
  for (i = num_items - 1; i >= 0; --i) \
    cur_list = gh_cons(conv (items[i]), cur_list); \
  return(cur_list); \
} \

list make_integer_list(int num_items, integer *items)
MAKE_LIST(gh_int2scm)

list make_number_list(int num_items, number *items)
MAKE_LIST(gh_double2scm)

list make_cnumber_list(int num_items, cnumber *items)
MAKE_LIST(cnumber2scm)

list make_boolean_list(int num_items, boolean *items)
MAKE_LIST(gh_bool2scm)

list make_string_list(int num_items, char **items)
MAKE_LIST(gh_str02scm)

list make_vector3_list(int num_items, vector3 *items)
MAKE_LIST(vector32scm)

list make_matrix3x3_list(int num_items, matrix3x3 *items)
MAKE_LIST(matrix3x32scm)

list make_cvector3_list(int num_items, cvector3 *items)
MAKE_LIST(cvector32scm)

list make_cmatrix3x3_list(int num_items, cmatrix3x3 *items)
MAKE_LIST(cmatrix3x32scm)

#define NO_CONVERSION  

list make_list_list(int num_items, list *items)
MAKE_LIST(NO_CONVERSION)

list make_object_list(int num_items, object *items)
MAKE_LIST(NO_CONVERSION)

list make_function_list(int num_items, object *items)
MAKE_LIST(NO_CONVERSION)

list make_SCM_list(int num_items, object *items)
MAKE_LIST(NO_CONVERSION)


/**************************************************************************/

/* object properties */

boolean object_is_member(char *type_name, object o)
{
  return(SCM_BOOL_F != gh_call2(gh_lookup("object-member?"),
				gh_symbol2scm(type_name),
				o));
}

static SCM object_property_value(object o, char *property_name)
{
  return(gh_call2(gh_lookup("object-property-value"),
		  o, 
		  gh_symbol2scm(property_name)));
}

integer integer_object_property(object o, char *property_name)
{
  return(gh_scm2int(object_property_value(o,property_name)));
}

number number_object_property(object o, char *property_name)
{
  return(gh_scm2double(object_property_value(o,property_name)));
}

cnumber cnumber_object_property(object o, char *property_name)
{
  return(scm2cnumber(object_property_value(o,property_name)));
}

boolean boolean_object_property(object o, char *property_name)
{
  return(SCM_BOOL_F != object_property_value(o,property_name));
}

char* string_object_property(object o, char *property_name)
{
  return(gh_scm2newstr(object_property_value(o,property_name),NULL));
}

vector3 vector3_object_property(object o, char *property_name)
{
  return(scm2vector3(object_property_value(o,property_name)));
}

matrix3x3 matrix3x3_object_property(object o, char *property_name)
{
  return(scm2matrix3x3(object_property_value(o,property_name)));
}

cvector3 cvector3_object_property(object o, char *property_name)
{
  return(scm2cvector3(object_property_value(o,property_name)));
}

cmatrix3x3 cmatrix3x3_object_property(object o, char *property_name)
{
  return(scm2cmatrix3x3(object_property_value(o,property_name)));
}

list list_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}

object object_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}

function function_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}

SCM SCM_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}
