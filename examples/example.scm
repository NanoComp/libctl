; Copyright (C) 1998 Steven G. Johnson
;
; This file may be used without restriction.  It is in the public
; domain, and is NOT restricted by the terms of any GNU license.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Library General Public License for more details. 

(define-class material-type no-parent
  (define-property epsilon 'number no-default positive?)
  (define-property conductivity 'number (make-default 0.0)))

(define-class geometric-object no-parent
  (define-property material 'material-type no-default)
  (define-property center 'vector3 no-default))

(define-class cylinder geometric-object
  (define-property axis 'vector3 (make-default (vector3 0 0 1)))
  (define-property radius 'number no-default positive?)
  (define-property height 'number no-default positive?))
	       
(define-class sphere geometric-object
  (define-property radius 'number no-default positive?))
	       
(define-class block geometric-object
  (define-property e1 'vector3 (make-default (vector3 1 0 0)))
  (define-property e2 'vector3 (make-default (vector3 0 1 0)))
  (define-property e3 'vector3 (make-default (vector3 0 0 1)))
  (define-property size 'vector3 no-default))
	       
; ****************************************************************

; Add some predefined variables, for convenience:

(define vacuum (make material-type (epsilon 1.0)))
(define air vacuum)

(define infinity 1.0e20) ; big number for infinite dimensions of objects

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

; ****************************************************************

(define-input-var dimensions 3 'integer positive?)

(define-input-var geometry '() (make-list-type-name 'geometric-object))

(define-input-var k-points '() (make-list-type-name 'vector3))

(define-input-output-var dummy (vector3 3.7 2.3 1.9) 'vector3)

(define-output-var mean-dielectric 'number)

(define-output-var gaps (make-list-type-name 'number))
