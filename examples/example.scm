(define material-type
  (make-class 'material-type no-parent
	      (make-property 'epsilon 'number no-default (list positive?))
	      (make-property 'conductivity 'number
			     (make-default 0.0) no-constraints)))

(define geometric-object
  (make-class 'geometric-object no-parent
	      (make-property 'material 'material-type
			     no-default no-constraints)
	      (make-property 'center '3vector no-default no-constraints)))

(define cylinder
  (make-class 'cylinder geometric-object
	      (make-property 'axis '3vector
			     (make-default (3vector 0 0 1)) no-constraints)
	       (make-property 'radius 'number no-default (list positive?))
	       (make-property 'height 'number no-default (list positive?))))
	       
(define epsilon (property-value-constructor 'epsilon))
(define conductivity (property-value-constructor 'conductivity))

(define material (property-value-constructor 'material))
(define center (3vector-property-value-constructor 'center))

(define radius (property-value-constructor 'radius))
(define height (property-value-constructor 'height))
(define axis (3vector-property-value-constructor 'axis))

; ****************************************************************

(define vacuum (make material-type (epsilon 1.0)))
(define air vacuum)

; ****************************************************************
