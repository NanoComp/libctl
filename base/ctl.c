/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998 Steven G. Johnson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 * Steven G. Johnson can be contacted at stevenj@alum.mit.edu.
 */

#include "ctl.h"

/**************************************************************************/

/* Functions missing from Guile 1.2: */

#ifndef HAVE_GUILE_1_3

/* Guile 1.2 is missing gh_bool2scm for some reason; redefine: */
#define gh_bool2scm bool2scm
SCM bool2scm(int b) { return (b ? SCM_BOOL_T : SCM_BOOL_F); }

#define gh_length gh_list_length
#define gh_vector_ref gh_vref

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

#else /* Guile 1.3 */

#define list_ref(l,index) gh_list_ref(l,gh_int2scm(index))

#endif

/**************************************************************************/

/* vector3 and matrix3x3 utilities: */

vector3 vector3_scale(number s, vector3 v)
{
  vector3 vnew;

  vnew.x = s * v.x;
  vnew.y = s * v.y;
  vnew.z = s * v.z;
  return vnew;
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

vector3 matrix3x3_vector3_mult(matrix3x3 m, vector3 v)
{
  vector3 vnew;

  vnew.x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z;
  vnew.y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z;
  vnew.z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z;
  return vnew;
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

boolean ctl_get_boolean(char *identifier)
{
  return(gh_scm2bool(gh_lookup(identifier)));
}

char* ctl_get_string(char *identifier)
{
  return(gh_scm2newstr(gh_lookup(identifier), NULL));
}

static vector3 scm2vector3(SCM sv)
{
  vector3 v;

  v.x = gh_scm2double(gh_vector_ref(sv,gh_int2scm(0)));
  v.y = gh_scm2double(gh_vector_ref(sv,gh_int2scm(1)));
  v.z = gh_scm2double(gh_vector_ref(sv,gh_int2scm(2)));
  return v;
}

vector3 ctl_get_vector3(char *identifier)
{
  return(scm2vector3(gh_lookup(identifier)));
}

static matrix3x3 scm2matrix3x3(SCM sm)
{
  matrix3x3 m;

  m.c0 = scm2vector3(gh_vector_ref(sm,gh_int2scm(0)));
  m.c1 = scm2vector3(gh_vector_ref(sm,gh_int2scm(1)));
  m.c2 = scm2vector3(gh_vector_ref(sm,gh_int2scm(2)));
  return m;
}

matrix3x3 ctl_get_matrix3x3(char *identifier)
{
  return(scm2matrix3x3(gh_lookup(identifier)));
}

list ctl_get_list(char *identifier)
{
  return(gh_lookup(identifier));
}

object ctl_get_object(char *identifier)
{
  return(gh_lookup(identifier));
}

/**** Setters ****/

/* This is ugly, but there is no gh_set routine and set! can't
   be called normally because it is really a macro. */
static void set_value(char *identifier, SCM value)
{
  gh_call3(gh_lookup("symbol-set!"), SCM_BOOL_F,
	   gh_symbol2scm(identifier), value);
}

void ctl_set_integer(char *identifier, integer value)
{
  set_value(identifier, gh_int2scm(value));
}

void ctl_set_number(char *identifier, number value)
{
  set_value(identifier, gh_double2scm(value));
}

void ctl_set_boolean(char *identifier, boolean value)
{
  set_value(identifier, gh_bool2scm(value));
}

void ctl_set_string(char *identifier, char *value)
{
  set_value(identifier, gh_str02scm(value));
}

static SCM vector32scm(vector3 v)
{
  return(gh_call3(gh_lookup("vector3"),
		  gh_double2scm(v.x),
		  gh_double2scm(v.y),
		  gh_double2scm(v.z)));
}

void ctl_set_vector3(char *identifier, vector3 value)
{
  set_value(identifier, vector32scm(value));
}

static SCM matrix3x32scm(matrix3x3 m)
{
  return(gh_call3(gh_lookup("matrix3x3"),
		  vector32scm(m.c0),
		  vector32scm(m.c1),
		  vector32scm(m.c2)));
}

void ctl_set_matrix3x3(char *identifier, matrix3x3 value)
{
  set_value(identifier, matrix3x32scm(value));
}

void ctl_set_list(char *identifier, list value)
{
  set_value(identifier, value);
}

void ctl_set_object(char *identifier, object value)
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

list list_list_ref(list l, int index)
{
  return(list_ref(l,index));
}

object object_list_ref(list l, int index)
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

list make_boolean_list(int num_items, boolean *items)
MAKE_LIST(gh_bool2scm)

list make_string_list(int num_items, char **items)
MAKE_LIST(gh_str02scm)

list make_vector3_list(int num_items, vector3 *items)
MAKE_LIST(vector32scm)

list make_matrix3x3_list(int num_items, matrix3x3 *items)
MAKE_LIST(matrix3x32scm)

#define NO_CONVERSION  

list make_list_list(int num_items, list *items)
MAKE_LIST(NO_CONVERSION)

list make_object_list(int num_items, object *items)
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

list list_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}

object object_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}
