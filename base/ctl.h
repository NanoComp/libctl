#ifndef CTL_H
#define CTL_H

#include <guile/gh.h>

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/**************************************************************************/

  /* Basic types: */

typedef int integer;
typedef double number;
typedef short boolean;
typedef SCM list;
typedef SCM object;

  /* define vector3 as a structure, not an array, so that it can
     be a function return value and so that simple assignment works. */
typedef struct {
  number x,y,z;
} vector3;

/**************************************************************************/

  /* vector3 utilities: */

#define vector3_dot(v1,v2) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
#define vector3_norm(v) sqrt(vector3_dot(v,v))

extern vector3 vector3_scale(number s, vector3 v);
extern vector3 vector3_cross(vector3 v1,vector3 v2);
extern vector3 vector3_plus(vector3 v1,vector3 v2);
extern vector3 vector3_minus(vector3 v1,vector3 v2);

/**************************************************************************/

  /* variable get/set functions */

extern integer ctl_get_integer(char *identifier);
extern number ctl_get_number(char *identifier);
extern boolean ctl_get_boolean(char *identifier);
extern char* ctl_get_string(char *identifier);
extern vector3 ctl_get_vector3(char *identifier);
extern list ctl_get_list(char *identifier);
extern object ctl_get_object(char *identifier);

extern void ctl_set_integer(char *identifier, integer value);
extern void ctl_set_number(char *identifier, number value);
extern void ctl_set_boolean(char *identifier, boolean value);
extern void ctl_set_string(char *identifier, char *value);
extern void ctl_set_vector3(char *identifier, vector3 value);
extern void ctl_set_list(char *identifier, list value);
extern void ctl_set_object(char *identifier, object value);

/**************************************************************************/

  /* list traversal */

extern int list_length(list l);
extern integer integer_list_ref(list l, int index);
extern number number_list_ref(list l, int index);
extern boolean boolean_list_ref(list l, int index);
extern char* string_list_ref(list l, int index);
extern vector3 vector3_list_ref(list l, int index);
extern list list_list_ref(list l, int index);
extern object object_list_ref(list l, int index);

/**************************************************************************/

  /* list creation */

extern list make_integer_list(int num_items, integer *items);
extern list make_number_list(int num_items, number *items);
extern list make_boolean_list(int num_items, boolean *items);
extern list make_string_list(int num_items, char **items);
extern list make_vector3_list(int num_items, vector3 *items);
extern list make_list_list(int num_items, list *items);
extern list make_object_list(int num_items, object *items);

/**************************************************************************/

  /* object properties */

SCM object_is_member(char *type_name, object o);

extern integer integer_object_property(object o, char *property_name);
extern number number_object_property(object o, char *property_name);
extern boolean boolean_object_property(object o, char *property_name);
extern char* string_object_property(object o, char *property_name);
extern vector3 vector3_object_property(object o, char *property_name);
extern list list_object_property(object o, char *property_name);
extern object object_object_property(object o, char *property_name);

/**************************************************************************/

#ifdef __cplusplus
	   }                               /* extern "C" */
#endif                          /* __cplusplus */

#endif /* CTL_H */
