(define material-type
  (make-class 'material-type no-parent
	      (make-property 'epsilon 'number no-default (list positive?))
	      (make-property 'conductivity 'number
			     (make-default 0.0) no-constraints)))

(define geometric-object
  (make-class 'geometric-object no-parent
	      (make-property 'material 'material-type
			     no-default no-constraints)
	      (make-property 'center 'vector3 no-default no-constraints)))

(define cylinder
  (make-class 'cylinder geometric-object
	      (make-property 'axis 'vector3
			     (make-default (vector3 0 0 1)) no-constraints)
	       (make-property 'radius 'number no-default (list positive?))
	       (make-property 'height 'number no-default (list positive?))))
	       
(define sphere
  (make-class 'sphere geometric-object
	      (make-property 'radius 'number no-default (list positive?))))
	       
(define block
  (make-class 'block geometric-object
	      (make-property 'e1 'vector3 (make-default (vector3 1 0 0))
			     no-constraints)
	      (make-property 'e2 'vector3 (make-default (vector3 0 1 0))
			     no-constraints)
	      (make-property 'e3 'vector3 (make-default (vector3 0 0 1))
			     no-constraints)
	      (make-property 'size 'vector3 no-default no-constraints)))
	       
(define epsilon (property-value-constructor 'epsilon))
(define conductivity (property-value-constructor 'conductivity))

(define material (property-value-constructor 'material))
(define center (vector3-property-value-constructor 'center))

(define radius (property-value-constructor 'radius))
(define height (property-value-constructor 'height))
(define axis (vector3-property-value-constructor 'axis))

(define e1 (vector3-property-value-constructor 'e1))
(define e2 (vector3-property-value-constructor 'e2))
(define e3 (vector3-property-value-constructor 'e3))
(define size (vector3-property-value-constructor 'size))

; ****************************************************************

(define vacuum (make material-type (epsilon 1.0)))
(define air vacuum)

; ****************************************************************

(define dimensions 3)
(input-var dimensions 'dimensions 'integer (list positive?))

(define geometry '())
(input-var
 geometry 'geometry (make-list-type-name 'geometric-object) '())

(define k-points '())
(input-var k-points 'k-points (make-list-type-name 'vector3) '())

(define dummy (vector3 3.7 2.3 1.9))
(input-output-var dummy 'dummy 'vector3 '())

(define mean-dielectric 0.0)
(output-var mean-dielectric 'mean-dielectric 'number)

(define gaps '())
(output-var gaps 'gaps (make-list-type-name 'number))
