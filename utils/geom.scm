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

(if (defined? 'ctl-io-c-only?) (set! ctl-io-c-only? true))

(define MATERIAL-TYPE (if (defined? 'material-type) 'material-type 'SCM))

; A default material so that we don't have to specify a material for
; an object when we just care about its geometry.  If material-type is
; an "abstract superclass" (no properties of its own), programs could
; interpret this as equating to default-material (below).  However, we
; only define this default if (make material-type) works, i.e. if
; defaults exist for all properties (if any) of material-type.
(define nothing (if (eq? MATERIAL-TYPE 'SCM)
                    '()
                    (if (for-all? (class-properties-all material-type)
                                  property-has-default?)
                        (make material-type)
                        no-default)))

(define-class geometric-object no-parent
  (define-property material nothing MATERIAL-TYPE)
  (define-property center no-default 'vector3))


(define-class compound-geometric-object geometric-object
  (define-property component-objects '()
    (make-list-type 'geometric-object)))

(define (non-negative? x) (not (negative? x)))

(define-class cylinder geometric-object
  (define-post-processed-property axis (vector3 0 0 1) 'vector3 unit-vector3)
  (define-property radius no-default 'number non-negative?)
  (define-property height no-default 'number non-negative?))

(define-class cone cylinder
  (define-property radius2 0 'number))

(define-class wedge cylinder
  (define-property wedge-angle (* 8 (atan 1)) 'number)
  (define-property wedge-start (vector3 1 0 0) 'vector3)
  (define-derived-property e1 'vector3
    (lambda (object)
      (let ((a (object-property-value object 'axis))
	    (s (object-property-value object 'wedge-start)))
	(unit-vector3 (vector3- s (vector3-scale (vector3-dot a s) a))))))
  (define-derived-property e2 'vector3
    (lambda (object)
      (let ((a (object-property-value object 'axis))
	    (e1 (object-property-value object 'e1)))
	(vector3-cross a e1)))))

(define-class sphere geometric-object
  (define-property radius no-default 'number non-negative?))

(define-class block geometric-object
  (define-post-processed-property e1 (vector3 1 0 0) 'vector3 unit-vector3)
  (define-post-processed-property e2 (vector3 0 1 0) 'vector3 unit-vector3)
  (define-post-processed-property e3 (vector3 0 0 1) 'vector3 unit-vector3)
  (define-property size no-default 'vector3)
  (define-derived-property projection-matrix 'matrix3x3
    (lambda (object)
      (matrix3x3-inverse
       (matrix3x3
	(object-property-value object 'e1)
	(object-property-value object 'e2)
	(object-property-value object 'e3))))))

(define identity_matrix (matrix3x3 (vector3 1 0 0)
                                   (vector3 0 1 0)
                                   (vector3 0 0 1)))

; some notes regarding prisms:
;  (a) When instantiating a prism, typically only the
;      fields `vertices`, `height,` `sidewall_angle`, and (optionally) `axis`
;      will be initialized by the user; all remaining fields are
;      derived properties that are computed internally. (So, morally
;      they should be thought of as having been declared using 
;      `define-derived-property` or `define-post-processed-property,`
;      except here the code that does the derivation or
;      post-processing is implemented in C, not scheme.)
;  (b) The suffix _p (for "prism") is used to identify variables
;      that store coordinates of points or components of vectors
;      in the prism coordinate system. (The prism coordinate system 
;      is defined by the condition that the prism axis is the z-axis 
;      and the prism floor lies in the xy plane at z==0.) Variables
;      with no suffix refer to quantities in ordinary 3D space.
;  (c) "centroid" refers to the centroid of the prism floor polygon; this is
;      the origin of the prism coordinate system [i.e. by definition
;      we have centroid_p=(0 0 0)].
;  (d) If 'axis' is left unspecified, it is inferred to be the
;      normal to the plane of the prism floor, with sign defined
;      by a right-hand-rule with respect to the first two vertices, i.e.
;      axis = normal_vector( (v1-centroid) x (v2-centroid) )
;  (e) The specification of the prism vertices and height suffices to
;      determine the center of the geometric object
;      (center = centroid + 0.5*height*axis), so---in contrast to all other
;      types of geometric-object---there is no need to specify the `center`
;      field when instantiating a prism.
;  (f) The sidwall angle determines an angle at which the prism is extruded.
;      A positive sidewall angle determines a prism that extrudes inward at
;      the given angle, and a negative sidewall angle determines a prisms
;      that extrudes outward. This is useful for modeling a prism formed in
;      a foundry that cannot grow objects with a perfectly normal sidewall.
(define-class prism geometric-object
; fields to be filled in by users
  (define-property vertices '() (make-list-type 'vector3))
  (define-property height 0 'number)
  (define-property axis (vector3 0 0 0) 'vector3)
  (define-property sidewall_angle 0 'number)
; derived fields computed internally
  (define-property vertices_p '() (make-list-type 'vector3))
  (define-property top_polygon_diff_vectors '() (make-list-type 'vector3))
  (define-property top_polygon_diff_vectors_scaled '() (make-list-type 'vector3))
  (define-property top_polygon_diff_vectors_p '() (make-list-type 'vector3))
  (define-property top_polygon_diff_vectors_scaled_p '() (make-list-type 'vector3))
  (define-property centroid (vector3 0 0 0) 'vector3)
  (define-property workspace '() (make-list-type 'number))
  (define-property m_c2p identity_matrix 'matrix3x3)
  (define-property m_p2c identity_matrix 'matrix3x3))

(define-class ellipsoid block
  (define-derived-property inverse-semi-axes 'vector3
    (lambda (object)
      (vector-map (lambda (x) (/ 2.0 x))
		  (object-property-value object 'size)))))

; ****************************************************************
(define-class lattice no-parent
  (define-post-processed-property basis1 (vector3 1 0 0) 'vector3 unit-vector3)
  (define-post-processed-property basis2 (vector3 0 1 0) 'vector3 unit-vector3)
  (define-post-processed-property basis3 (vector3 0 0 1) 'vector3 unit-vector3)

  (define-property size (vector3 1 1 1) 'vector3)
  (define-property basis-size (vector3 1 1 1) 'vector3)

  (define-derived-property b1 'vector3
    (lambda (object)
      (vector3-scale (vector3-x (object-property-value object 'basis-size))
				(object-property-value object 'basis1))))
  (define-derived-property b2 'vector3
    (lambda (object)
      (vector3-scale (vector3-y (object-property-value object 'basis-size))
				(object-property-value object 'basis2))))
  (define-derived-property b3 'vector3
    (lambda (object)
      (vector3-scale (vector3-z (object-property-value object 'basis-size))
				(object-property-value object 'basis3))))

  (define-derived-property basis 'matrix3x3
    (lambda (object)
      (let ((B (matrix3x3
		(object-property-value object 'b1)
		(object-property-value object 'b2)
		(object-property-value object 'b3))))
	(if (zero? (matrix3x3-determinant B))
	    (error "lattice basis vectors must be linearly independent!"))
	B)))
  (define-derived-property metric 'matrix3x3
    (lambda (object)
      (let ((B (object-property-value object 'basis)))
	(matrix3x3* (matrix3x3-transpose B) B)))))

; ****************************************************************

; Define some utility functions:

(define (shift-geometric-object go shift-vector)
  (let ((c (object-property-value go 'center)))
    (modify-object go (center (vector3+ c shift-vector)))))

(define (geometric-object-duplicates shift-vector min-multiple max-multiple go)
  (define (g-o-d min-multiple L)
    (if (<= min-multiple max-multiple)
	(g-o-d (+ min-multiple 1)
	       (cons (shift-geometric-object
		      go (vector3-scale min-multiple shift-vector))
		     L))
	L))
  (g-o-d min-multiple '()))

(define (geometric-objects-duplicates shift-vector min-multiple max-multiple
				      go-list)
  (fold-left append '()
	     (map (lambda (go)
		    (geometric-object-duplicates
		     shift-vector min-multiple max-multiple go))
		  go-list)))

(define (lattice-duplicates lat go-list . usize)
  (define (lat->lattice v)
    (cartesian->lattice (matrix3x3* (object-property-value lat 'basis) v)))
  (let ((u1 (if (>= (length usize) 1) (list-ref usize 0) 1))
	(u2 (if (>= (length usize) 2) (list-ref usize 1) 1))
	(u3 (if (>= (length usize) 3) (list-ref usize 2) 1))
	(s (object-property-value lat 'size)))
    (let ((b1 (lat->lattice (vector3 u1 0 0)))
	  (b2 (lat->lattice (vector3 0 u2 0)))
	  (b3 (lat->lattice (vector3 0 0 u3)))
	  (n1 (ceiling (/ (vector3-x s) u1)))
	  (n2 (ceiling (/ (vector3-y s) u2)))
	  (n3 (ceiling (/ (vector3-z s) u3))))
      (geometric-objects-duplicates
       b1 (- (floor (/ (- n1 1) 2))) (ceiling (/ (- n1 1) 2))
       (geometric-objects-duplicates
	b2 (- (floor (/ (- n2 1) 2))) (ceiling (/ (- n2 1) 2))
	(geometric-objects-duplicates
	 b3 (- (floor (/ (- n3 1) 2))) (ceiling (/ (- n3 1) 2))
	 go-list))))))

(define (geometric-objects-lattice-duplicates go-list . usize)
  (apply lattice-duplicates (cons geometry-lattice
				  (cons go-list usize))))

; ****************************************************************

(define-input-var dimensions 3 'integer)
(define-input-var default-material nothing MATERIAL-TYPE)
(define-input-var geometry-center (vector3 0) 'vector3)
(define-input-var geometry-lattice (make lattice) 'lattice)
(define-input-var geometry '() (make-list-type 'geometric-object))
(define-input-var ensure-periodicity true 'boolean)

; special vector3 that signifies 'no value specified'
(define auto-center (vector3 (nan) (nan) (nan)))

(define-external-function point-in-object? true false
  'boolean 'vector3 'geometric-object)

(define-external-function normal-to-object true false
  'vector3 'vector3 'geometric-object)

(define-external-function point-in-periodic-object? true false
  'boolean 'vector3 'geometric-object)

; (define-external-function material-of-point true false
;   MATERIAL-TYPE 'vector3)

(define-external-function display-geometric-object-info false false
  no-return-value 'integer 'geometric-object)

(define-external-function range-overlap-with-object true false
  'number 'vector3 'vector3 'geometric-object 'number 'integer)

(define-external-function square-basis false false
  'matrix3x3 'matrix3x3 'vector3)

; ****************************************************************
; Functions and variables for determining the grid size

(define no-size 1e-20) ; for when a particular lattice dimension has no size
(define-param resolution 10)   ; the resolution (may be a vector3)
(define-param grid-size false) ; force grid size, if set

(define (get-resolution)
  (if (vector? resolution)
      resolution
      (vector3 resolution resolution resolution)))
(define (get-grid-size)
  (if grid-size
      grid-size
      (let ((res (get-resolution)))
	(vector-map
	 (lambda (x) (inexact->exact (max (ceiling x) 1)))
	 (vector-map * res (object-property-value geometry-lattice 'size))))))
(define (get-grid-size-prod)
  (let ((s (get-grid-size)))
    (* (vector3-x s) (vector3-y s) (vector3-z s))))

; ****************************************************************
; Cartesian conversion and rotation for lattice and reciprocal coords:

; The following conversion routines work for vector3 and matrix3x3 arguments:

(define (lattice->cartesian x)
  (if (vector3? x)
      (matrix3x3*
       (object-property-value geometry-lattice 'basis) x)
      (matrix3x3*
       (matrix3x3*
	(object-property-value geometry-lattice 'basis) x)
       (matrix3x3-inverse (object-property-value geometry-lattice 'basis)))))

(define (cartesian->lattice x)
  (if (vector3? x)
      (matrix3x3*
       (matrix3x3-inverse (object-property-value geometry-lattice 'basis)) x)
      (matrix3x3*
       (matrix3x3*
	(matrix3x3-inverse (object-property-value geometry-lattice 'basis)) x)
       (object-property-value geometry-lattice 'basis))))

(define (reciprocal->cartesian x)
  (let ((s (vector-map
	    (lambda (x) (if (= x no-size) 1 x))
	    (object-property-value geometry-lattice 'size))))
    (let ((Rst
	   (matrix3x3-transpose
	    (matrix3x3* (object-property-value geometry-lattice 'basis)
			(matrix3x3 (vector3 (vector3-x s) 0 0)
				   (vector3 0 (vector3-y s) 0)
				   (vector3 0 0 (vector3-z s)))))))
      (if (vector3? x)
	  (matrix3x3* (matrix3x3-inverse Rst) x)
	  (matrix3x3* (matrix3x3* (matrix3x3-inverse Rst) x) Rst)))))

(define (cartesian->reciprocal x)
  (let ((s (vector-map
	    (lambda (x) (if (= x no-size) 1 x))
	    (object-property-value geometry-lattice 'size))))
    (let ((Rst
	   (matrix3x3-transpose
	    (matrix3x3* (object-property-value geometry-lattice 'basis)
			(matrix3x3 (vector3 (vector3-x s) 0 0)
				   (vector3 0 (vector3-y s) 0)
				   (vector3 0 0 (vector3-z s)))))))
      (if (vector3? x)
	  (matrix3x3* Rst x)
	  (matrix3x3* (matrix3x3* Rst x) (matrix3x3-inverse Rst))))))

(define (lattice->reciprocal x) (cartesian->reciprocal (lattice->cartesian x)))
(define (reciprocal->lattice x) (cartesian->lattice (reciprocal->cartesian x)))

; rotate vectors in lattice/reciprocal coords (note that the axis
; is also given in the corresponding basis):

(define (rotate-lattice-vector3 axis theta v)
  (cartesian->lattice
   (rotate-vector3 (lattice->cartesian axis) theta
		   (lattice->cartesian v))))

(define (rotate-reciprocal-vector3 axis theta v)
  (cartesian->reciprocal
   (rotate-vector3 (reciprocal->cartesian axis) theta
		   (reciprocal->cartesian v))))

; ****************************************************************

