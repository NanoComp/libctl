; libctl: flexible Guile-based control files for scientific software
; Copyright (C) 1998-2019 Massachusetts Institute of Technology and Steven G. Johnson
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
  (and (pair? object)
       (list? (object-type-names object))
       (member type-name (object-type-names object))))
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

(define no-default '(no-default))
(define (has-default? default) (not (eq? default no-default)))

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
(define (property-default-value property) (property-default property))
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

(define exported-type-list '())
(define (export-type type-name)
  (set! exported-type-list (cons type-name exported-type-list)))

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
   ((eq? type-name 'number) (make-simple-type-descriptor 'number real?))
   ((eq? type-name 'cnumber) (make-simple-type-descriptor 'cnumber complex?))
   ((eq? type-name 'integer) (make-simple-type-descriptor 'integer integer?))
   ((eq? type-name 'boolean) (make-simple-type-descriptor 'boolean boolean?))
   ((eq? type-name 'string) (make-simple-type-descriptor 'string string?))
   ((eq? type-name 'SCM)
    (make-simple-type-descriptor 'SCM (lambda (x) true)))
   ((eq? type-name 'function)
    (make-simple-type-descriptor 'function procedure?))
   ((eq? type-name 'vector3)
    (make-simple-type-descriptor 'vector3 real-vector3?))
   ((eq? type-name 'cvector3) (make-simple-type-descriptor 'cvector3 vector3?))
   ((eq? type-name 'matrix3x3)
    (make-simple-type-descriptor 'matrix3x3 real-matrix3x3?))
   ((eq? type-name 'cmatrix3x3)
    (make-simple-type-descriptor 'cmatrix3x3 matrix3x3?))
   ((eq? type-name 'list) (make-simple-type-descriptor 'list list?))
   ((symbol? type-name) (make-object-type-descriptor type-name))
   ((list-type-name? type-name) (make-list-type-descriptor type-name))
   (else (error "unknown type" type-name))))

(define (primitive-type? type-name)
  (or (eq? type-name 'number)
      (eq? type-name 'integer)
      (eq? type-name 'boolean)
      (eq? type-name 'function)
      (eq? type-name 'SCM)))

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
(define (class-properties-all class)
  (append (class-properties class)
	  (let ((parent (class-parent class)))
	    (if parent (class-properties parent) '()))))
(define (class-member? type-name class)
  (if (list? class)
      (or (eq? type-name (class-type-name class))
	  (class-member? type-name (class-parent class)))
      false))

(define no-parent false)

(define (make class . property-values)
  (if (list? class)
      (let* ((newprops
              (map (lambda (property)
                     (get-property-value property property-values))
                   (list-transform-negative
                     (class-properties class) property-derived?)))
             (o
              (extend-object
               (apply make (cons (class-parent class)
                                 (append newprops property-values)))
               (class-type-name class)
               newprops)))
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

(define (list-property-value-constructor name type-name)
  (lambda x
    (make-property-value-pair
     name
     (if (and (= (length x) 1) (check-type type-name (car x)))
	 (car x)
	 x))))

(define (type-property-value-constructor type-name name)
  (cond
   ((or (eq? type-name 'vector3)  (eq? type-name 'cvector3))
    (vector3-property-value-constructor name))
   ((list-type-name? type-name)
    (list-property-value-constructor name type-name))
   (else (property-value-constructor name))))

(define (post-processing-constructor post-process-func constructor)
  (lambda x
    (let ((value-pair (apply constructor x)))
      (make-property-value-pair (car value-pair)
				(post-process-func (cdr value-pair))))))

(defmacro-public define-property (name default type-name . constraints)
  `(begin
     (define ,name
       (type-property-value-constructor ,type-name (quote ,name)))
     (make-property (quote ,name) ,type-name ,default
		    not-derived ,@constraints)))

(defmacro-public define-post-processed-property
  (name default type-name post-process-func . constraints)
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
; Define classes.  A bit ugly since we support (define-property ...)
; in the property list, but Guile 2.x doesn't allow (define ...) to
; be used in the middle of a list expression.  So, we need to extract
; those definitions first and duplicate some of the define-property
; code above.

(defmacro-public define-class (class-name parent . properties)
  (let ((pdefs (map
		(lambda (p)
		  (let ((name (cadr p))
			(type-name (cadddr p)))
		    `(define ,name
		       (type-property-value-constructor
			,type-name (quote ,name)))))
		(list-transform-positive properties
		  (lambda (p) (eq? (car p) 'define-property)))))
	(ppdefs (map
		 (lambda (p)
		   (let ((name (cadr p))
			 (type-name (cadddr p))
			 (post-process-func (list-ref p 4)))
		     `(define ,name (post-processing-constructor
				     ,post-process-func
				     (type-property-value-constructor
				      ,type-name (quote ,name))))))
		 (list-transform-positive properties
		   (lambda (p)
		     (eq? (car p) 'define-post-processed-property)))))
	(props (map
		(lambda (p)
		  (cond
		   ((eq? (car p) 'define-property)
		    (let ((name (cadr p))
			  (default (caddr p))
			  (type-name (cadddr p))
			  (constraints (cddddr p)))
		      `(make-property (quote ,name) ,type-name ,default
				      not-derived ,@constraints)))
		   ((eq? (car p) 'define-post-processed-property)
		    (let ((name (cadr p))
			  (default (caddr p))
			  (type-name (cadddr p))
			  (post-process-func (list-ref p 4))
			  (constraints (cdr (cddddr p))))
		      `(make-property (quote ,name) ,type-name ,default
				      not-derived ,@constraints)))
		   (else p)))
		properties)))
  `(begin
     ,@pdefs
     ,@ppdefs
     (define ,class-name (make-class (quote ,class-name)
				   ,parent
				   ,@props)))))
