#include "ctl.h"

/* Guile 1.2 is missing gh_bool2scm for some reason; redefine: */
#define gh_bool2scm(b) ((b) ? SCM_BOOL_T : SCM_BOOL_F)

/**************************************************************************/

/* vector3 utilities: */

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

/* This will change to use gh_vector_ref in Guile 1.3: */
static vector3 scm2vector3(SCM sv)
{
  vector3 v;

  v.x = gh_scm2double(gh_vref(sv,gh_int2scm(0)));
  v.y = gh_scm2double(gh_vref(sv,gh_int2scm(1)));
  v.z = gh_scm2double(gh_vref(sv,gh_int2scm(2)));
  return v;
}

vector3 ctl_get_vector3(char *identifier)
{
  return(scm2vector3(gh_lookup(identifier)));
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

static set_value(char *identifier, SCM value)
{
  gh_call2(gh_lookup("set!"), gh_lookup(identifier), value);
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

/* in Guile 1.3, gh_vset will become gh_vector_set */
void ctl_set_vector3(char *identifier, vector3 value)
{
  SCM sv = gh_lookup(identifier);

  gh_vset(sv, gh_int2scm(0), gh_double2scm(value.x));
  gh_vset(sv, gh_int2scm(1), gh_double2scm(value.y));
  gh_vset(sv, gh_int2scm(2), gh_double2scm(value.z));
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
  return(gh_list_length(l));
}

/* Guile 1.2 doesn't have the gh_list_ref function.  Sigh. */
/* Note: index must be in [0,list_length(l) - 1].  We don't check! */
static SCM list_ref(list l, int index)
{
  SCM cur, rest = l;

  while (index >= 0) {
    cur = gh_car(rest);
    rest = gh_cdr(l);
    --index;
  }
  return cur;
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
  for (i = num_items - 1; i >= 0; ++i) \
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

SCM vector32scm(vector3 v)
{
  return(gh_call3(gh_lookup("vector3"),
		  gh_double2scm(v.x),
		  gh_double2scm(v.y),
		  gh_double2scm(v.z)));
}

list make_vector3_list(int num_items, vector3 *items)
MAKE_LIST(vector32scm)

#define NO_CONVERSION  

list make_list_list(int num_items, list *items)
MAKE_LIST(NO_CONVERSION)

list make_object_list(int num_items, object *items)
MAKE_LIST(NO_CONVERSION)


/**************************************************************************/

/* object properties */

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

list list_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}

object object_object_property(object o, char *property_name)
{
  return(object_property_value(o,property_name));
}
