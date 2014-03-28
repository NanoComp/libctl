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
; The Nelder-Mead simplex algorithm for multidimensional minimization.
; See the simplex-minimize function, below.

(define (ax+by a x b y)
  (define (cdr-null x) (if (null? x) '() (cdr x)))
  (if (and (null? x) (null? y))
      '()
      (let ((ax (if (null? x) 0 (* a (car x))))
	    (by (if (null? y) 0 (* b (car y)))))
	(cons (+ ax by) (ax+by a (cdr-null x) b (cdr-null y))))))

(define (simplex-point x val) (cons x val))
(define simplex-point-x car)
(define simplex-point-val cdr)

(define (simplex-high s)
  (car (sort s (lambda (s1 s2) (> (simplex-point-val s1)
				  (simplex-point-val s2))))))
(define (simplex-high2 s)
  (cadr (sort s (lambda (s1 s2) (> (simplex-point-val s1)
				   (simplex-point-val s2))))))
(define (simplex-low s)
  (car (sort s (lambda (s1 s2) (< (simplex-point-val s1)
                                  (simplex-point-val s2))))))

(define (simplex-replace s s-old s-new)
  (if (null? s)
      '()
      (if (eq? (car s) s-old)
	  (cons s-new (cdr s))
	  (cons (car s) (simplex-replace (cdr s) s-old s-new)))))

(define (simplex-sum-x s)
  (if (null? s)
      '()
      (ax+by 1 (simplex-point-x (car s)) 1 (simplex-sum-x (cdr s)))))

(define (simplex-centroid-x s)
  (let ((sum (ax+by 1 (simplex-sum-x s) 
		    -1 (simplex-point-x (simplex-high s)))))
    (ax+by (/ (- (length s) 1)) sum 0.0 '())))

(define (simplex-shrink s-min f s)
  (if (null? s)
      '()
      (if (eq? s-min (car s))
	  (cons (car s) (simplex-shrink s-min f (cdr s)))
	  (let ((x (ax+by 0.5 (simplex-point-x s-min)
			  0.5 (simplex-point-x (car s)))))
	    (cons (simplex-point x (apply f x))
		  (simplex-shrink s-min f (cdr s)))))))

(define simplex-reflect-ratio 1.0)
(define simplex-expand-ratio 2.0)
(define simplex-contract-ratio 0.5)

(define (simplex-contract f s)
  (let ((s-h (simplex-high s))
        (s-l (simplex-low s))
        (x0 (simplex-centroid-x s)))
    (let ((xc (ax+by (- 1 simplex-contract-ratio) x0
		     simplex-contract-ratio (simplex-point-x s-h))))
      (let ((vc (apply f xc)))
	(if (< vc (simplex-point-val s-h))
	    (simplex-replace s s-h (simplex-point xc vc))
	    (simplex-shrink s-l f s))))))

(define (simplex-iter f s)
  (let ((s-h (simplex-high s))
	(s-h2 (simplex-high2 s))
	(s-l (simplex-low s))
	(x0 (simplex-centroid-x s)))
    (let ((xr (ax+by (+ 1 simplex-reflect-ratio) x0
		     (- simplex-reflect-ratio) (simplex-point-x s-h))))
      (let ((vr (apply f xr)))
	(if (and (<= vr (simplex-point-val s-h2))
		 (>= vr (simplex-point-val s-l)))
	    
	    (simplex-replace s s-h (simplex-point xr vr))
	    (if (< vr (simplex-point-val s-l))
		(let ((xe (ax+by (- 1 simplex-expand-ratio) x0
				 simplex-expand-ratio xr)))
		  (let ((ve (apply f xe)))
		    (if (>= ve vr)
			(simplex-replace s s-h (simplex-point xr vr))
			(simplex-replace s s-h (simplex-point xe ve)))))
		(if (and (< vr  (simplex-point-val s-h))
			 (> vr  (simplex-point-val s-h2)))
		    (simplex-contract f (simplex-replace 
					 s s-h (simplex-point xr vr)))
		    (simplex-contract f s))))))))

(define (simplex-iterate f s tol)
  (let ((s-h (simplex-high s))
        (s-l (simplex-low s)))
    (if (<= (magnitude (- (simplex-point-val s-h) (simplex-point-val s-l)))
	    (* 0.5 tol (+ tol (magnitude (simplex-point-val s-h)) 
			  (magnitude (simplex-point-val s-l)))))
	s-l
	(begin
	  (print "extremization: best so far is " s-l "\n")
	  (simplex-iterate f (simplex-iter f s) tol)))))

(define (simplex-shift-x x i)
  (let ((xv (list->vector x)))
    (let ((xv-i (vector-ref xv i)))
      (if (< (magnitude xv-i) 1e-6)
	  (vector-set! xv i 0.1)
	  (vector-set! xv i (* 0.9 xv-i)))
      (vector->list xv))))

(define (simplex-shift-list x)
  (define (ssl-aux i)
    (if (< i 0)
	'()
	(cons (simplex-shift-x x i) (ssl-aux (- i 1)))))
  (cons x (ssl-aux (- (length x) 1))))

; Use the Simplex method to minimize the function (f . x), where
; the initial guess is x0 and the fractional tolerance on the value
; of the solution is tol.
(define (simplex-minimize f x0 tol)
  (let ((s0 (map (lambda (x) (simplex-point x (apply f x)))
		 (simplex-shift-list x0))))
    (simplex-iterate f s0 tol)))
