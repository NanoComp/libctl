; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998-2014 Massachusetts Institute of Technology and Steven G. Johnson
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
; vector3 and associated operators.  (a type to represent 3-vectors)

; Guile 1.6 does not support exact->inexact on complex numbers, grrr
(define (ctl-exact->inexact x)
  (if (real? x) (exact->inexact x) x))
(define (vector3->inexact v) (vector-map ctl-exact->inexact v))
(define (vector3->exact v) (vector-map inexact->exact v))
(define (vector3 . args)
  (vector3->inexact
   (if (= (length args) 0)
       (vector 0 0 0)
       (if (= (length args) 1)
	   (vector (first args) 0 0)
	   (if (= (length args) 2)
	       (vector (first args) (second args) 0)
	       (vector (first args) (second args) (third args)))))))
(define cvector3 vector3)
(define (vector3? v)
  (and (vector? v)
       (= (vector-length v) 3)
       (vector-for-all? v number?)))
(define (real-vector3? v)
  (and (vector3? v) (vector-for-all? v real?)))
(define (vector3-x v) (vector-ref v 0))
(define (vector3-y v) (vector-ref v 1))
(define (vector3-z v) (vector-ref v 2))

(define (vector3+ v1 v2) (vector-map + v1 v2))
(define (vector3- v1 v2) (vector-map - v1 v2))
(define (vector3-dot v1 v2) (vector-fold-left + 0 (vector-map * v1 v2)))
(define (vector3-conj v) (vector-map conj v))
(define (vector3-cdot v1 v2) (vector3-dot (vector3-conj v1) v2))
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
(define (vector3-norm v) (sqrt (magnitude (vector3-cdot v v))))

(define (unit-vector3 . args)
  (let ((v (if (and (= (length args) 1) (vector3? (car args)))
	       (car args)
	       (apply vector3 args))))
    (vector3-scale (/ (vector3-norm v)) v)))

(define (vector3-close? v1 v2 tolerance)
  (and (<= (magnitude (- (vector-ref v1 0) (vector-ref v2 0))) tolerance)
       (<= (magnitude (- (vector-ref v1 1) (vector-ref v2 1))) tolerance)
       (<= (magnitude (- (vector-ref v1 2) (vector-ref v2 2))) tolerance)))
(define (vector3= v1 v2) (vector3-close? v1 v2 0.0))

; Define polymorphic operators (work on both vectors and numbers):

(define (binary+ x y)
  (cond
   ((and (number? x) (zero? x)) y)
   ((and (number? y) (zero? y)) x)
   ((and (vector3? x) (vector3? y)) (vector3+ x y))
   (else (+ x y))))
(define (binary- x y)
  (if (and (vector3? x) (vector3? y)) (vector3- x y) (- x y)))
(define (binary* x y)
  (if (or (vector3? x) (vector3? y)) (vector3* x y) (* x y)))
(define (binary/ x y)
  (if (and (vector3? x) (number? y)) (vector3-scale (/ y) x) (/ x y)))
(define (binary= x y)
  (cond
   ((and (vector3? x) (vector3? y)) (vector3= x y))
   ((and (number? x) (number? y)) (= x y))
   (else false)))

(define (unary-abs x) (if (vector3? x) (vector3-norm x) (magnitude x)))

(define (unary->inexact x) (if (vector3? x) 
			       (vector3->inexact x)
			       (ctl-exact->inexact x)))

; ****************************************************************

; Rotating a vector (see also rotation-matrix3x3 in matrix3x3.scm):

(define (deg->rad theta)
  (* theta (/ 3.141592653589793238462643383279502884197 180.0)))

(define (rad->deg theta)
  (* theta (/ 180.0 3.141592653589793238462643383279502884197)))

(define (rotate-vector3 axis theta v)
  (let ((u (unit-vector3 axis)))
    (let ((vpar (vector3-scale (vector3-dot u v) u))
	  (vcross (vector3-cross u v)))
      (let ((vperp (vector3- v vpar)))
	(vector3+ vpar (vector3+ (vector3-scale (cos theta) vperp)
				 (vector3-scale (sin theta) vcross)))))))

; ****************************************************************

