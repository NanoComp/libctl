; ****************************************************************
; Replacements for Scheme functions missing from Guile 1.2.

(define true #t)
(define false #f)

(define (string-suffix? suff s)
  (if (> (string-length suff) (string-length s))
      #f
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
      #t
      (if (pred (car l))
	  (for-all? (cdr l) pred)
	  #f)))

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

(define (combine op list1 list2)
  (if (or (null? list1) (null? list2))
      '()
      (cons (op (car list1) (car list2))
	    (combine op (cdr list1) (cdr list2)))))

(define (vector-combine op v1 v2)
  (list->vector (combine op (vector->list v1) (vector->list v2))))

(define (vector-fold-right op init v)
  (fold-right op init (vector->list v)))

(define (vector-map func v) (list->vector (map func (vector->list v))))

(define (indent indentby)
  (display (make-string indentby #\space)))

(define (displaymany indentby . items)
  (indent indentby)
  (for-each (lambda (item) (display item)) items))

; ****************************************************************
; 3vector and associated operators.  (a type to represent 3-vectors)

(define (3vector . args)
  (if (= (length args) 0)
      (vector 0 0 0)
      (if (= (length args) 1)
	  (vector (first args) 0 0)
	  (if (= (length args) 2)
	      (vector (first args) (second args) 0)
	      (vector (first args) (second args) (third args))))))
(define (3vector? v)
  (and (vector v)
       (= (vector-length v) 3)
       (vector-for-all? v number?)))
(define (3vector+ v1 v2) (vector-combine + v1 v2))
(define (3vector- v1 v2) (vector-combine - v1 v2))
(define (3vector-dot v1 v2) (vector-fold-right + 0 (vector-combine * v1 v2)))
(define (3vector-scale s v) (vector-map (lambda (x) (* s x)) v))
(define (3vector* a b)
  (if (number? a)
      (3vector-scale a b)
      (if (number? b)
	  (3vector-scale b a)
	  (3vector-dot a b))))
(define (3vector-cross v1 v2)
  (3vector (- (* (vector-ref v1 1) (vector-ref v2 2))
	      (* (vector-ref v1 2) (vector-ref v2 1)))
	   (- (* (vector-ref v1 2) (vector-ref v2 0))
	      (* (vector-ref v1 0) (vector-ref v2 2)))
	   (- (* (vector-ref v1 0) (vector-ref v2 1))
	      (* (vector-ref v1 1) (vector-ref v2 0)))))
(define (3vector-norm v) (sqrt (3vector-dot v v)))

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
  (cons #t default-value))
(define no-default (cons #f '()))
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
(define (type-predicate type-name)
  (cond
   ((eq? type-name 'number) number?)
   ((eq? type-name 'integer) integer?)
   ((eq? type-name 'symbol) symbol?)
   ((eq? type-name 'list) list?)
   ((eq? type-name 'boolean) boolean?)
   ((eq? type-name '3vector) 3vector?)
   ((symbol? type-name) (object-memberp type-name))
   ((and (pair? type-name) (eq? (car type-name) 'list))
    (let ((listtype_p (type-predicate (cdr type-name))))
      (lambda (val) (and (list? val) (for-all? val listtype_p)))))
   (else (error "unknown type" type-name))))
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
	      (error "property" (property-name property) "must have type"
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
      #f))

(define no-parent #f)

(define (make class . property-values)
  (if (list? class)
      (extend-object (apply make (cons (class-parent class) property-values))
		     (class-type-name class)
		     (map (lambda (property)
			    (get-property-value property property-values))
			  (class-properties class)))
      null-object))

(define (display-class indentby class)
  (displaymany indentby "Class " (class-type-name class) ": ")
  (newline)
  (if (class-parent class)
      (display-class (+ indentby 4) (class-parent class)))
  (map (lambda (property)
	 (displaymany (+ indentby 2)
		      (property-type-name property) " "
		      (property-name property))
	 (if (property-has-default? property)
	     (displaymany 0
			  " = " (property-default-value property)))
	 (newline))
       (class-properties class))
  (display ""))
			  
(define (help class) (display-class 0 class))

; ****************************************************************
; Creating property-value constructors.

(define (property-value-constructor name)
  (lambda (x) (make-property-value name x)))

(define (3vector-property-value-constructor name)
  (lambda x (make-property-value name (apply 3vector x))))

(define (list-property-value-constructor name)
  (lambda x (make-property-value name x)))

; ****************************************************************

