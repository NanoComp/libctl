; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998, 1999, 2000 Steven G. Johnson
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
	(* (matrix3x3-ref m 1 2) (matrix3x3-ref m 2 1)))
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

; Return the rotation matrix for rotating by theta around axis:
(define (rotation-matrix3x3 axis theta)
  (matrix3x3
   (rotate-vector3 axis theta (vector3 1 0 0))
   (rotate-vector3 axis theta (vector3 0 1 0))
   (rotate-vector3 axis theta (vector3 0 0 1))))

; ****************************************************************
