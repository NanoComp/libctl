; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998, 1999, 2000, 2001, Steven G. Johnson
;
; This library is free software; you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation; either
; version 2 of the License, or (at your option) any later version.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Lesser General Public License for more details.
; 
; You should have received a copy of the GNU Lesser General Public
; License along with this library; if not, write to the
; Free Software Foundation, Inc., 59 Temple Place - Suite 330,
; Boston, MA  02111-1307, USA.
;
; Steven G. Johnson can be contacted at stevenj@alum.mit.edu.

; ****************************************************************
; Functions for creating and manipulating classes, objects, types,
; and properties.

(define class-list '())

(define (make-property-value-pair name value)
  (cons name value))

(define (make-object type-names property-values)
  (cons type-names property-values))
(define (object-type-names object) (car object))
(define (object-property-values object) (cdr object))
(define (object-member? type-name object)
  (and (pair? object) (member type-name (object-type-names object))))
(define (object-property-value object property-name)
  (assoc-ref (object-property-values object) property-name))
(define (object-memberp type-name)
  (lambda (object) (object-member? type-name object)))
; (I wish Scheme had implicit currying like ML.)
(define (extend-object object type-name property-values)
  (make-object (cons type-name (object-type-names object))
	       (combine-alists property-values
			       (object-property-values object))))
(define (modify-object object . property-values)
  (make-object (object-type-names object)
	       (combine-alists property-values
			       (object-property-values object))))
(define null-object (make-object '() '()))

(define (make-default default-value)
  (cons true default-value))
(define no-default (cons false '()))
(define (has-default? default) (car default))
(define (default-value default) (cdr default))

(define (make-derived derive-func)
  (cons true derive-func))
(define not-derived (cons false '()))
(define (derived? derived) (car derived))
(define (derive-func derived) (cdr derived))

(define (make-property name type-name default derived . constraints)
  (list name type-name default constraints derived))

(define no-constraints '())
(define (property-name property) (first property))
(define (property-type-name property) (second property))
(define (property-default property) (third property))
(define (property-constraints property) (fourth property))
(define (property-derived property) (fifth property))
(define (property-has-default? property)
  (has-default? (property-default property)))
(define (property-default-value property)
  (default-value (property-default property)))
(define (property-derived? property)
  (derived? (property-derived property)))
(define (derive-property property object)
  (make-property-value-pair
   (property-name property)
   ((derive-func (property-derived property)) object)))

(define (check-constraints constraints value)
  (for-all? constraints (lambda (c) (c value))))

(define (make-list-type el-type-name)
  (cons 'list el-type-name))
(define (list-type-name? type-name)
  (and (pair? type-name) (eq? (car type-name) 'list)))
(define (list-el-type-name type-name) (cdr type-name))

(define (make-type-descriptor kind name name-str predicate)
  (list kind name name-str predicate))
(define type-descriptor-kind first)
(define type-descriptor-name second)
(define type-descriptor-name-str third)
(define type-descriptor-predicate fourth)
(define (make-simple-type-descriptor name predicate)
  (make-type-descriptor 'simple name (symbol->string name) predicate))
(define (make-object-type-descriptor name)
  (make-type-descriptor 'object name (symbol->string name)
			(object-memberp name)))
(define (make-list-type-descriptor name)
  (make-type-descriptor 'uniform-list name "list" list?))
(define (get-type-descriptor type-name)
  (cond
   ((eq? type-name 'number) (make-simple-type-descriptor 'number number?))
   ((eq? type-name 'integer) (make-simple-type-descriptor 'integer integer?))
   ((eq? type-name 'boolean) (make-simple-type-descriptor 'boolean boolean?))
   ((eq? type-name 'string) (make-simple-type-descriptor 'string string?))
   ((eq? type-name 'function)
    (make-simple-type-descriptor 'function procedure?))
   ((eq? type-name 'vector3) (make-simple-type-descriptor 'vector3 vector3?))
   ((eq? type-name 'matrix3x3)
    (make-simple-type-descriptor 'matrix3x3 matrix3x3?))
   ((eq? type-name 'list) (make-simple-type-descriptor 'list list?))
   ((symbol? type-name) (make-object-type-descriptor type-name))
   ((list-type-name? type-name) (make-list-type-descriptor type-name))
   (else (error "unknown type" type-name))))

(define (type-string type-name)
  (let ((desc (get-type-descriptor type-name)))
    (cond
     ((or (eq? (type-descriptor-kind desc) 'simple)
	  (eq? (type-descriptor-kind desc) 'object))
      (type-descriptor-name-str desc))
     ((eq? (type-descriptor-kind desc) 'uniform-list)
      (string-append (type-string (list-el-type-name type-name)) " list"))
     (else (error "unknown type" type-name)))))
(define (type-predicate type-name)
  (let ((desc (get-type-descriptor type-name)))
    (cond
     ((or (eq? (type-descriptor-kind desc) 'simple)
	  (eq? (type-descriptor-kind desc) 'object))
      (type-descriptor-predicate desc))
     ((eq? (type-descriptor-kind desc) 'uniform-list)
      (lambda (val)
	(and ((type-descriptor-predicate desc) val)
	     (for-all? val (type-predicate (list-el-type-name type-name))))))
     (else (error "unknown type" type-name)))))
(define (check-type type-name value)
  ((type-predicate type-name) value))

(define (get-property-value property property-values)
  (let ((val (assoc (property-name property)
		    property-values)))
    (let ((newval (if (pair? val) val
		      (if (property-has-default? property)
			  (make-property-value-pair
			   (property-name property)
			   (property-default-value property))
			  (error "no value for property"
				 (property-name property))))))
      (if (check-constraints (property-constraints property) (cdr newval))
	  (if (check-type (property-type-name property) (cdr newval))
	      newval
	      (error "wrong type for property" (property-name property) 'type
		     (property-type-name property)))
	  (error "invalid value for property" (property-name property))))))

(define (make-class type-name parent . properties)
  (let ((new-class (list type-name parent properties)))
    (set! class-list (cons new-class class-list))
    new-class))
(define (class-type-name class) (first class))
(define (class-parent class) (second class))
(define (class-properties class) (third class))
(define (class-member? type-name class)
  (if (list? class)
      (or (eq? type-name (class-type-name class))
	  (class-member? type-name (class-parent class)))
      false))

(defmacro-public define-class (class-name parent . properties)
  `(define ,class-name (make-class (quote ,class-name)
				   ,parent
				   ,@properties)))

(define no-parent false)

(define (make class . property-values)
  (if (list? class)
      (let ((o
	     (extend-object
	      (apply make (cons (class-parent class) property-values))
	      (class-type-name class)
	      (map (lambda (property)
		     (get-property-value property property-values))
		   (list-transform-negative
		     (class-properties class) property-derived?)))))
	(fold-left (lambda (o p)
		     (modify-object o (derive-property p o)))
		   o
		   (list-transform-positive 
		       (class-properties class) property-derived?)))
      null-object))

; ****************************************************************
; Defining property values.

(define (property-value-constructor name)
  (lambda (x) (make-property-value-pair name x)))

(define (vector3-property-value-constructor name)
  (lambda x (make-property-value-pair name (if (and (= (length x) 1)
						    (vector3? (car x)))
					       (car x)
					       (apply vector3 x)))))

(define (list-property-value-constructor name)
  (lambda x (make-property-value-pair name x)))

(define (type-property-value-constructor type-name name)
  (cond
   ((eq? type-name 'vector3)
    (vector3-property-value-constructor name))
   ((list-type-name? type-name)
    (list-property-value-constructor name))
   (else (property-value-constructor name))))

(define (post-processing-constructor post-process-func constructor)
  (lambda x
    (let ((value-pair (apply constructor x)))
      (make-property-value-pair (car value-pair)
				(post-process-func (cdr value-pair))))))

(defmacro-public define-property (name type-name default . constraints)
  `(begin
     (define ,name
       (type-property-value-constructor ,type-name (quote ,name)))
     (make-property (quote ,name) ,type-name ,default
		    not-derived ,@constraints)))

(defmacro-public define-post-processed-property
  (name type-name post-process-func default . constraints)
  `(begin
     (define ,name (post-processing-constructor
		    ,post-process-func
		    (type-property-value-constructor ,type-name
						     (quote ,name))))
     (make-property (quote ,name) ,type-name ,default
		    not-derived ,@constraints)))

(defmacro-public define-derived-property (name type-name derive-func)
  `(make-property (quote ,name) ,type-name no-default
		  (make-derived ,derive-func)))

; ****************************************************************
