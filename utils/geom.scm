; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998, 1999 Steven G. Johnson
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

(if (not (defined? 'material-type))
    (define-class material-type no-parent)) ; define dummy class

(define-class geometric-object no-parent
  (define-property material 'material-type no-default)
  (define-property center 'vector3 no-default))

(define-class cylinder geometric-object
  (define-post-processed-property axis 'vector3 unit-vector3
    (make-default (vector3 0 0 1)))
  (define-property radius 'number no-default positive?)
  (define-property height 'number no-default positive?))
	       
(define-class sphere geometric-object
  (define-property radius 'number no-default positive?))
	       
(define-class block geometric-object
  (define-post-processed-property e1 'vector3 unit-vector3 
    (make-default (vector3 1 0 0)))
  (define-post-processed-property e2 'vector3 unit-vector3 
    (make-default (vector3 0 1 0)))
  (define-post-processed-property e3 'vector3 unit-vector3 
    (make-default (vector3 0 0 1)))
  (define-property size 'vector3 no-default)
  (define-derived-property projection-matrix 'matrix3x3
    (lambda (object)
      (matrix3x3-inverse
       (matrix3x3
	(object-property-value object 'e1)
	(object-property-value object 'e2)
	(object-property-value object 'e3))))))

(define-class ellipsoid block
  (define-derived-property inverse-semi-axes 'vector3
    (lambda (object)
      (vector-map (lambda (x) (/ 2.0 x))
		  (object-property-value object 'size)))))

; ****************************************************************

(define-class lattice no-parent
  (define-post-processed-property basis1 'vector3 unit-vector3 
    (make-default (vector3 1 0 0)))
  (define-post-processed-property basis2 'vector3 unit-vector3 
    (make-default (vector3 0 1 0)))
  (define-post-processed-property basis3 'vector3 unit-vector3 
    (make-default (vector3 0 0 1)))

  (define-property size 'vector3 (make-default (vector3 1 1 1)))

  (define-derived-property basis 'matrix3x3
    (lambda (object)
      (let ((B (matrix3x3
		(object-property-value object 'basis1)
		(object-property-value object 'basis2)
		(object-property-value object 'basis3))))
	(if (zero? (matrix3x3-determinant B))
	    (error "lattice basis vectors must be linearly independent!"))
	B)))
  (define-derived-property metric 'matrix3x3
    (lambda (object)
      (let ((B (matrix3x3
		(object-property-value object 'basis1)
		(object-property-value object 'basis2)
		(object-property-value object 'basis3))))
	(matrix3x3* (matrix3x3-transpose B) B)))))

; ****************************************************************

; Define some utility functions:

(define (shift-geometric-object go shift-vector)
  (let ((c (object-property-value go 'center)))
    (modify-object go (center (vector3+ c shift-vector)))))

(define (geometric-object-duplicates shift-vector min-multiple max-multiple go)
  (if (<= min-multiple max-multiple)
      (cons (shift-geometric-object
	     go (vector3-scale min-multiple shift-vector))
	    (geometric-object-duplicates shift-vector
					 (+ min-multiple 1) max-multiple
					 go))
      '()))

(define (geometric-objects-duplicates shift-vector min-multiple max-multiple
				      go-list)
  (fold-right append '()
	     (map (lambda (go)
		    (geometric-object-duplicates
		     shift-vector min-multiple max-multiple go))
		  go-list)))

(define (geometric-objects-lattice-duplicates go-list)
  (let ((b1 (object-property-value geometry-lattice 'basis1))
	(b2 (object-property-value geometry-lattice 'basis2))
	(b3 (object-property-value geometry-lattice 'basis3))
	(n1 (vector-ref (object-property-value geometry-lattice 'size) 0))
	(n2 (vector-ref (object-property-value geometry-lattice 'size) 1))
	(n3 (vector-ref (object-property-value geometry-lattice 'size) 2)))
    (geometric-objects-duplicates
     b1 (- (floor (/ (- n1 1) 2))) (ceiling (/ (- n1 1) 2))
     (geometric-objects-duplicates
      b2 (- (floor (/ (- n2 1) 2))) (ceiling (/ (- n2 1) 2))
      (geometric-objects-duplicates
       b3 (- (floor (/ (- n3 1) 2))) (ceiling (/ (- n3 1) 2))
       go-list)))))

; ****************************************************************

(define-input-var dimensions 3 'integer)
(define-input-var default-material '() 'material-type)
(define-input-var geometry-lattice (make lattice) 'lattice)
(define-input-var geometry '() (make-list-type 'geometric-object))
(define-input-var ensure-periodicity true 'boolean)

(define-external-function point-in-object? true false
  'boolean 'vector3 'geometric-object)

(define-external-function point-in-periodic-object? true false
  'boolean 'vector3 'geometric-object)

; (define-external-function material-of-point true false
;   'material-type 'vector3)

(define-external-function display-geometric-object-info false false
  no-return-value 'integer 'geometric-object)

(define-external-function square-basis false false
  'matrix3x3 'matrix3x3 'vector3)
