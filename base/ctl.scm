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
  (and (vector v)
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

; ****************************************************************
; Class constructors.

(define class-list '())

(define (make-property-value name value)
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
(define (modify-object object property-values)
  (make-object (object-type-names object)
	       (combine-alists property-values
			       (object-property-values object))))
(define null-object (make-object '() '()))

(define (make-default default-value)
  (cons true default-value))
(define no-default (cons false '()))
(define (has-default? default) (car default))
(define (default-value default) (cdr default))

(define (make-property name type-name default constraints)
  (list name type-name default constraints))
(define no-constraints '())
(define (property-name property) (first property))
(define (property-type-name property) (second property))
(define (property-default property) (third property))
(define (property-constraints property) (fourth property))
(define (property-has-default? property)
  (has-default? (property-default property)))
(define (property-default-value property)
  (default-value (property-default property)))

(define (check-constraints constraints value)
  (for-all? constraints (lambda (c) (c value))))

(define (make-list-type-name el-type-name)
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
			  (make-property-value
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

(define no-parent false)

(define (make class . property-values)
  (if (list? class)
      (extend-object (apply make (cons (class-parent class) property-values))
		     (class-type-name class)
		     (map (lambda (property)
			    (get-property-value property property-values))
			  (class-properties class)))
      null-object))

; ****************************************************************
; Input/Output variables.

(define input-var-list '())
(define output-var-list '())

(define (make-var value var-name var-type-name var-constraints)
  (list var-name var-type-name var-constraints value))
(define (var-name var) (first var))
(define (var-type-name var) (second var))
(define (var-constraints var) (third var))
(define (var-value var) (fourth var))

(define (input-var value var-name var-type-name var-constraints)
  (let ((new-var (make-var value var-name var-type-name var-constraints)))
    (set! input-var-list (cons new-var input-var-list))
    new-var))
(define (output-var value var-name var-type-name)
  (let ((new-var (make-var value var-name var-type-name no-constraints)))
    (set! output-var-list (cons new-var output-var-list))
    new-var))
(define (input-output-var value var-name var-type-name var-constraints)
  (let ((new-var (make-var value var-name var-type-name var-constraints)))
    (set! input-var-list (cons new-var input-var-list))
    (set! output-var-list (cons new-var output-var-list))
    new-var))

(define (check-vars var-list)
  (for-all? (lamba (v) 
		   (if (not (check-type (var-type-name v) (var-value v)))
		       (error "wrong type for variable" (var-name v) 'type
			      (var-type-name v))
		       (if (not (check-constraints 
				 (var-constraints v) (var-value v)))
			   (error "failed constraint for" (var-name v))
			   true)))
	    var-list))

; ****************************************************************
; Creating property-value constructors.

(define (property-value-constructor name)
  (lambda (x) (make-property-value name x)))

(define (vector3-property-value-constructor name)
  (lambda x (make-property-value name (apply vector3 x))))

(define (list-property-value-constructor name)
  (lambda x (make-property-value name x)))

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

(define (run)
  (read-input-vars)
  (run-program)
  (write-output-vars))
