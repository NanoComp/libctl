; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998 Steven G. Johnson
;
; This library is free software; you can redistribute it and/or
; modify it under the terms of the GNU Library General Public
; License as published by the Free Software Foundation; either
; version 2 of the License, or (at your option) any later version.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Library General Public License for more details.
; 
; You should have received a copy of the GNU Library General Public
; License along with this library; if not, write to the
; Free Software Foundation, Inc., 59 Temple Place - Suite 330,
; Boston, MA  02111-1307, USA.
;
; Steven G. Johnson can be contacted at stevenj@alum.mit.edu.

; ****************************************************************
; Replacements for Scheme functions missing from Guile 1.2.

(define true #t)
(define false #f)

(define (string-suffix? suff s)
  (if (> (string-length suff) (string-length s))
      false
      (string=? suff (substring s (- (string-length s)
				     (string-length suff))
				(string-length s)))))

(define (list-transform-positive l pred)
  (if (null? l)
      l
      (if (pred (car l))
	  (cons (car l) (list-transform-positive (cdr l) pred))
	  (list-transform-positive (cdr l) pred))))

(define (list-transform-negative l pred)
  (if (null? l)
      l
      (if (not (pred (car l)))
	  (cons (car l) (list-transform-negative (cdr l) pred))
	  (list-transform-negative (cdr l) pred))))

(define (alist-copy al)
  (if (null? al) '()
      (cons (cons (caar al) (cdar al)) (alist-copy (cdr al)))))

(define (for-all? l pred)
  (if (null? l)
      true
      (if (pred (car l))
	  (for-all? (cdr l) pred)
	  false)))

(define (first list) (list-ref list 0))
(define (second list) (list-ref list 1))
(define (third list) (list-ref list 2))
(define (fourth list) (list-ref list 3))
(define (fifth list) (list-ref list 4))

(define (fold-right op init list)
  (if (null? list)
      init
      (op (car list) (fold-right op init (cdr list)))))

; ****************************************************************
; Miscellaneous utility functions.

(define (compose f g) (lambda args (f (apply g args))))

(define (car-or-x p) (if (pair? p) (car p) p))

; combine 2 alists.  returns a list containing all of the associations
; in a1 and any associations in a2 that are not in a1
(define (combine-alists a1 a2)
  (if (null? a2)
      a1
      (combine-alists
       (if (assoc (caar a2) a1) a1 (cons (car a2) a1))
       (cdr a2))))

(define (vector-for-all? v pred) (for-all? (vector->list v) pred))

(define (vector-fold-right op init v)
  (fold-right op init (vector->list v)))

(define (vector-map func . v)
  (list->vector (apply map (cons func (map vector->list v)))))

(define (indent indentby)
  (display (make-string indentby #\space)))

(define (display-many . items)
  (for-each (lambda (item) (display item)) items))

; ****************************************************************
; vector3 and associated operators.  (a type to represent 3-vectors)

(define (vector3 . args)
  (if (= (length args) 0)
      (vector 0 0 0)
      (if (= (length args) 1)
	  (vector (first args) 0 0)
	  (if (= (length args) 2)
	      (vector (first args) (second args) 0)
	      (vector (first args) (second args) (third args))))))
(define (vector3? v)
  (and (vector? v)
       (= (vector-length v) 3)
       (vector-for-all? v number?)))
(define (vector3+ v1 v2) (vector-map + v1 v2))
(define (vector3- v1 v2) (vector-map - v1 v2))
(define (vector3-dot v1 v2) (vector-fold-right + 0 (vector-map * v1 v2)))
(define (vector3-scale s v) (vector-map (lambda (x) (* s x)) v))
(define (vector3* a b)
  (if (number? a)
      (vector3-scale a b)
      (if (number? b)
	  (vector3-scale b a)
	  (vector3-dot a b))))
(define (vector3-cross v1 v2)
  (vector3 (- (* (vector-ref v1 1) (vector-ref v2 2))
	      (* (vector-ref v1 2) (vector-ref v2 1)))
	   (- (* (vector-ref v1 2) (vector-ref v2 0))
	      (* (vector-ref v1 0) (vector-ref v2 2)))
	   (- (* (vector-ref v1 0) (vector-ref v2 1))
	      (* (vector-ref v1 1) (vector-ref v2 0)))))
(define (vector3-norm v) (sqrt (vector3-dot v v)))

(define (unit-vector3 . args)
  (let ((v (if (and (= (length args) 1) (vector3? (car args)))
	       (car args)
	       (apply vector3 args))))
    (vector3-scale (/ (vector3-norm v)) v)))

; Define polymorphic binary operators (work on both vectors and numbers):

(define (binary+ x y)
  (if (and (vector3? x) (vector3? y)) (vector3+ x y) (+ x y)))
(define (binary- x y)
  (if (and (vector3? x) (vector3? y)) (vector3- x y) (- x y)))
(define (binary* x y)
  (if (or (vector3? x) (vector3? y)) (vector3* x y) (* x y)))
(define (binary/ x y)
  (if (and (vector3? x) (number? y)) (vector3-scale (/ y) x) (/ x y)))

; ****************************************************************
; matrix3x3 and associated functions (a type to represent 3x3 matrices)

; we represent a matrix3x3 by a vector of 3 columns, each of which
; is a 3-vector.

(define (matrix3x3 c1 c2 c3)
  (vector c1 c2 c3))
(define (matrix3x3? m)
  (and (vector? m)
       (= (vector-length m) 3)
       (vector-for-all? m vector3?)))
(define (matrix3x3-col m col)
  (vector-ref m col))
(define (matrix3x3-ref m row col)
  (vector-ref (matrix3x3-col m col) row))
(define (matrix3x3-row m row)
  (vector3 (matrix3x3-ref m row 0)
	   (matrix3x3-ref m row 1)
	   (matrix3x3-ref m row 2)))

(define (matrix3x3-transpose m)
  (matrix3x3
   (matrix3x3-row m 0)
   (matrix3x3-row m 1)
   (matrix3x3-row m 2)))

(define (matrix3x3+ m1 m2)
  (vector-map vector3+ m1 m2))
(define (matrix3x3- m1 m2)
  (vector-map vector3- m1 m2))

(define (matrix3x3-scale s m)
  (vector-map (lambda (v) (vector3-scale s v)) m))
(define (matrix3x3-mv-mult m v)
  (vector3 (vector3-dot (matrix3x3-row m 0) v)
	   (vector3-dot (matrix3x3-row m 1) v)
	   (vector3-dot (matrix3x3-row m 2) v)))
(define (matrix3x3-vm-mult v m)
  (vector3 (vector3-dot (matrix3x3-col m 0) v)
	   (vector3-dot (matrix3x3-col m 1) v)
	   (vector3-dot (matrix3x3-col m 2) v)))
(define (matrix3x3-mm-mult m1 m2)
  (matrix3x3
   (vector3 (vector3-dot (matrix3x3-row m1 0) (matrix3x3-col m2 0))
	    (vector3-dot (matrix3x3-row m1 1) (matrix3x3-col m2 0))
	    (vector3-dot (matrix3x3-row m1 2) (matrix3x3-col m2 0)))
   (vector3 (vector3-dot (matrix3x3-row m1 0) (matrix3x3-col m2 1))
	    (vector3-dot (matrix3x3-row m1 1) (matrix3x3-col m2 1))
	    (vector3-dot (matrix3x3-row m1 2) (matrix3x3-col m2 1)))
   (vector3 (vector3-dot (matrix3x3-row m1 0) (matrix3x3-col m2 2))
	    (vector3-dot (matrix3x3-row m1 1) (matrix3x3-col m2 2))
	    (vector3-dot (matrix3x3-row m1 2) (matrix3x3-col m2 2)))))
(define (matrix3x3* a b)
  (cond
   ((number? a) (matrix3x3-scale a b))
   ((number? b) (matrix3x3-scale b a))
   ((vector3? a) (matrix3x3-vm-mult a b))
   ((vector3? b) (matrix3x3-mv-mult a b))
   (else (matrix3x3-mm-mult a b))))

(define (matrix3x3-determinant m)
  (-
   (+ (* (matrix3x3-ref m 0 0) (matrix3x3-ref m 1 1) (matrix3x3-ref m 2 2))
      (* (matrix3x3-ref m 0 1) (matrix3x3-ref m 1 2) (matrix3x3-ref m 2 0))
      (* (matrix3x3-ref m 1 0) (matrix3x3-ref m 2 1) (matrix3x3-ref m 0 2)))
   (+ (* (matrix3x3-ref m 0 2) (matrix3x3-ref m 1 1) (matrix3x3-ref m 2 0))
      (* (matrix3x3-ref m 0 1) (matrix3x3-ref m 1 0) (matrix3x3-ref m 2 2))
      (* (matrix3x3-ref m 1 2) (matrix3x3-ref m 2 1) (matrix3x3-ref m 0 0)))))

(define (matrix3x3-inverse m)
  (matrix3x3-scale
   (/ (matrix3x3-determinant m))
   (matrix3x3
    (vector3
     (- (* (matrix3x3-ref m 1 1) (matrix3x3-ref m 2 2))
	(* (matrix3x3-ref m 1 2) (matrix3x3-ref m 2 2)))
     (- (* (matrix3x3-ref m 1 2) (matrix3x3-ref m 2 0))
	(* (matrix3x3-ref m 1 0) (matrix3x3-ref m 2 2)))
     (- (* (matrix3x3-ref m 1 0) (matrix3x3-ref m 2 1))
	(* (matrix3x3-ref m 1 1) (matrix3x3-ref m 2 0))))
    (vector3
     (- (* (matrix3x3-ref m 2 1) (matrix3x3-ref m 0 2))
	(* (matrix3x3-ref m 0 1) (matrix3x3-ref m 2 2)))
     (- (* (matrix3x3-ref m 0 0) (matrix3x3-ref m 2 2))
	(* (matrix3x3-ref m 0 2) (matrix3x3-ref m 2 0)))
     (- (* (matrix3x3-ref m 0 1) (matrix3x3-ref m 2 0))
	(* (matrix3x3-ref m 0 0) (matrix3x3-ref m 2 1))))
    (vector3
     (- (* (matrix3x3-ref m 0 1) (matrix3x3-ref m 1 2))
	(* (matrix3x3-ref m 1 1) (matrix3x3-ref m 0 2)))
     (- (* (matrix3x3-ref m 1 0) (matrix3x3-ref m 0 2))
	(* (matrix3x3-ref m 0 0) (matrix3x3-ref m 1 2)))
     (- (* (matrix3x3-ref m 1 1) (matrix3x3-ref m 0 0))
	(* (matrix3x3-ref m 1 0) (matrix3x3-ref m 0 1)))))))

; ****************************************************************
; Class constructors.

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
; (I wish Scheme had currying)
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
	(apply modify-object
	       (cons o
		     (map (lambda (property)
			    (derive-property property o))
			  (list-transform-positive 
			      (class-properties class) property-derived?)))))
      null-object))

; ****************************************************************
; Input/Output variables.

(define input-var-list '())
(define output-var-list '())

(define (make-var value-thunk var-name var-type-name var-constraints)
  (list var-name var-type-name var-constraints value-thunk))
(define (var-name var) (first var))
(define (var-type-name var) (second var))
(define (var-constraints var) (third var))
(define (var-value-thunk var) (fourth var))
(define (var-value var) ((var-value-thunk var)))

(define (input-var! value-thunk var-name var-type-name . var-constraints)
  (let ((new-var (make-var value-thunk var-name 
			   var-type-name var-constraints)))
    (set! input-var-list (cons new-var input-var-list))
    new-var))
(define (output-var! value-thunk var-name var-type-name)
  (let ((new-var (make-var value-thunk var-name
			   var-type-name no-constraints)))
    (set! output-var-list (cons new-var output-var-list))
    new-var))

(defmacro-public define-input-var
  (name init-val var-type-name . var-constraints)
  `(begin
     (define ,name ,init-val)
     (input-var! (lambda () ,name) (quote ,name)
		 ,var-type-name ,@var-constraints)))

(defmacro-public define-input-output-var
  (name init-val var-type-name . var-constraints)
  `(begin
     (define ,name ,init-val)
     (input-var! (lambda () ,name) (quote ,name)
		 ,var-type-name ,@var-constraints)
     (output-var! (lambda () ,name) (quote ,name) ,var-type-name)))

(defmacro-public define-output-var (name var-type-name)
  `(begin
     (define ,name 'no-value)
     (output-var! (lambda () ,name) (quote ,name) ,var-type-name)))

(define (check-vars var-list)
  (for-all? var-list
	    (lambda (v) 
		   (if (not (check-type (var-type-name v) (var-value v)))
		       (error "wrong type for variable" (var-name v) 'type
			      (var-type-name v))
		       (if (not (check-constraints 
				 (var-constraints v) (var-value v)))
			   (error "failed constraint for" (var-name v))
			   true)))))

; ****************************************************************
; Defining external functions.

(define (make-external-function name read-inputs? write-outputs?
				return-type-name arg-type-names)
  (list name read-inputs? write-outputs? return-type-name arg-type-names))
(define external-function-name first)
(define external-function-read-inputs? second)
(define external-function-write-outputs? third)
(define external-function-return-type-name fourth)
(define external-function-arg-type-names fifth)

(define no-return-value 'none)

(define external-function-list '())
(define (external-function! name read-inputs? write-outputs?
			    return-type-name . arg-type-names)
  (set! external-function-list
	(cons (make-external-function name read-inputs? write-outputs?
				      return-type-name arg-type-names)
	      external-function-list)))

(define (external-function-aux-name name)
  (symbol-append name '-aux))
(define (external-function-aux name)
  (eval (external-function-aux-name name)))
  
(define (check-arg-types name args . arg-type-names)
  (if (not (= (length args) (length arg-type-names)))
      (begin
	(display-many "Expecting " (length arg-type-names) " arguments for "
		      name)
	(newline)
	(error "Wrong number of arguments for function" name))
      (for-each
       (lambda (arg arg-type-name)
	 (if (not (check-type arg-type-name arg))
	     (error "wrong type for argument" 'type arg-type-name 'in name)))
       args arg-type-names)))

(defmacro-public define-external-function
  (name read-inputs? write-outputs? return-type-name . arg-type-names)
  `(begin
     (define ,name
       (lambda args
	 (check-arg-types (quote ,name) args ,@arg-type-names)
	 (if ,read-inputs? (read-input-vars))
	 (let ((return-value
		(apply (external-function-aux (quote ,name)) args)))
	   (if ,write-outputs? (write-output-vars))
	      return-value)))
     (external-function! (quote ,name) ,read-inputs? ,write-outputs?
			 ,return-type-name ,@arg-type-names)))

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

(define (display-class indentby class)
  (indent indentby)
  (display-many "Class " (class-type-name class) ": ")
  (newline)
  (if (class-parent class)
      (display-class (+ indentby 4) (class-parent class)))
  (for-each (lambda (property)
	 (indent (+ indentby 4))
	 (display-many (type-string (property-type-name property)) " "
		       (property-name property))
	 (if (property-has-default? property)
	     (display-many " = " (property-default-value property)))
	 (newline))
       (class-properties class)))
			  
(define (class-help class) (display-class 0 class))

(define (variable-help var)
  (display-many (type-string (var-type-name var)) " "
		(var-name var) " = " (var-value var))
  (newline))

(define (help)
  (for-each class-help class-list)
  (newline)
  (display "Input variables: ") (newline)
  (for-each variable-help input-var-list)
  (newline)
  (display "Output variables: ") (newline)
  (for-each variable-help output-var-list))

; ****************************************************************

; More utilities.

; Return the arithemtic sequence (list): start start+step ... (n values)
(define (arith-sequence start step n)
  (if (= n 0)
      '() 
      (cons start (arith-sequence (binary+ start step) step (- n 1)))))

; Given a list of numbers, linearly interpolates n values between
; each pair of numbers.
(define (interpolate n nums)
  (cons 
   (car nums)
   (fold-right
    append '()
    (map
     (lambda (x y)
       (reverse (arith-sequence y (binary/ (binary- x y) (+ n 1)) (+ n 1))))
     (reverse (cdr (reverse nums))) ; nums w/o last value
     (cdr nums))))) ; nums w/o first value

; ****************************************************************
