; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998, 1999 Steven G. Johnson
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
