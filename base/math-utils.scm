; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998, 1999, 2000, 2001, 2002, Steven G. Johnson
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
; Miscellaneous math utilities

; Return the arithmetic sequence (list): start start+step ... (n values)
(define (arith-sequence start step n)
  (define (s x n L) ; tail-recursive helper function
    (if (= n 0)
      L
      (s (binary+ x step) (- n 1) (cons x L))))
  (reverse (s start n '())))

; Given a list of numbers, linearly interpolates n values between
; each pair of numbers.
(define (interpolate n nums)
  (cons 
   (car nums)
   (fold-right
    append '()
    (map
     (lambda (x y)
       (reverse (arith-sequence y (binary/ (binary- x y) (+ n 1)) (+ n 1))))
     (reverse (cdr (reverse nums))) ; nums w/o last value
     (cdr nums))))) ; nums w/o first value

; ****************************************************************
; Minimization and root-finding utilities (useful in ctl scripts)

; The routines are:
;    minimize: minimize a function of one argument
;    minimize-multiple: minimize a function of multiple arguments
;    maximize, maximize-multiple : as above, but maximize
;    find-root: find the root of a function of one argument
; All routines use quadratically convergent methods.

; ****************************************************************

(define min-arg car)
(define min-val cdr)
(define max-arg min-arg)
(define max-val min-val)

; One-dimensional minimization (using Brent's method):

; (minimize f tol) : minimize (f x) with fractional tolerance tol
; (minimize f tol guess) : as above, but gives starting guess
; (minimize f tol x-min x-max) : as above, but gives range to optimize in
;                                (this is preferred)
; All variants return a result that contains both the argument and the
; value of the function at its minimum.
;      (min-arg result) : the argument of the function at its minimum
;      (min-val result) : the value of the function at its minimum

(define (minimize f tol . min-max)
  (define (midpoint a b) (* 0.5 (+ a b)))

  (define (quadratic-min-denom x a b fx fa fb)
    (magnitude (* 2.0 (- (* (- x a) (- fx fb)) (* (- x b) (- fx fa))))))
  (define (quadratic-min-num x a b fx fa fb)
    (let ((den (* 2.0 (- (* (- x a) (- fx fb)) (* (- x b) (- fx fa)))))
	  (num (- (* (- x a) (- x a) (- fx fb))
		  (* (- x b) (- x b) (- fx fa)))))
      (if (> den 0) (- num) num)))

  (define (tol-scale x) (* tol (+ (magnitude x) 1e-6)))
  (define (converged? x a b)
    (<= (magnitude (- x (midpoint a b))) (- (* 2 (tol-scale x)) (* 0.5 (- b a)))))
  
  (define golden-ratio (* 0.5 (- 3 (sqrt 5))))
  (define (golden-interpolate x a b)
    (* golden-ratio (if (>= x (midpoint a b)) (- a x) (- b x))))

  (define (sign x) (if (< x 0) -1 1))

  (define (brent-minimize x a b v w fx fv fw prev-step prev-prev-step)
    (define (guess-step proposed-step)
      (let ((step (if (> (magnitude proposed-step) (tol-scale x))
		      proposed-step
		      (* (tol-scale x) (sign proposed-step)))))
	(let ((u (+ x step)))
	  (let ((fu (f u)))
	    (if (<= fu fx)
		(if (> u x)
		    (brent-minimize u x b w x fu fw fx step prev-step)
		    (brent-minimize u a x w x fu fw fx step prev-step))
		(let ((new-a (if (< u x) u a))
		      (new-b (if (< u x) b u)))
		  (if (or (<= fu fw) (= w x))
		      (brent-minimize x new-a new-b w u fx fw fu
				      step prev-step)
		      (if (or (<= fu fv) (= v x) (= v w))
			  (brent-minimize x new-a new-b u w fx fu fw
					  step prev-step)
			  (brent-minimize x new-a new-b v w fx fv fw
					  step prev-step)))))))))
	      
    (if (converged? x a b)
	(cons x fx)
	(if (> (magnitude prev-prev-step) (tol-scale x))
	    (let ((p (quadratic-min-num x v w fx fv fw))
		  (q (quadratic-min-denom x v w fx fv fw)))
	      (if (or (>= (magnitude p) (magnitude (* 0.5 q prev-prev-step)))
		      (< p (* q (- a x))) (> p (* q (- b x))))
		  (guess-step (golden-interpolate x a b))
		  (guess-step (/ p q))))
	    (guess-step (golden-interpolate x a b)))))

  (define (bracket-minimum a b c fa fb fc)
    (if (< fb fc)
	(list a b c fa fb fc)
	(let ((u (/ (quadratic-min-num b a c fb fa fc)
		    (max (quadratic-min-denom b a c fb fa fc) 1e-20)))
	      (u-max (+ b (* 100 (- c b)))))
	  (cond
	   ((positive? (* (- b u) (- u c)))
	    (let ((fu (f u)))
	      (if (< fu fc)
		  (bracket-minimum b u c fb fu fc)
		  (if (> fu fb)
		      (bracket-minimum a b u fa fb fu)
		      (bracket-minimum b c (+ c (* 1.6 (- c b)))
				       fb fc (f (+ c (* 1.6 (- c b)))))))))
	   ((positive? (* (- c u) (- u u-max)))
	    (let ((fu (f u)))
	      (if (< fu fc)
		  (bracket-minimum c u (+ c (* 1.6 (- c b)))
				   fc fu (f (+ c (* 1.6 (- c b)))))
		  (bracket-minimum b c u fb fc fu))))
	   ((>= (* (- u u-max) (- u-max c)) 0)
	    (bracket-minimum b c u-max fb fc (f u-max)))
	   (else
	    (bracket-minimum b c (+ c (* 1.6 (- c b)))
			     fb fc (f (+ c (* 1.6 (- c b))))))))))

   (if (= (length min-max) 2)
       (let ((x-min (first min-max))
	     (x-max (second min-max)))
	 (let ((xm (midpoint x-min x-max)))
	   (let ((fm (f xm)))
	     (brent-minimize xm x-min x-max xm xm fm fm fm 0 0))))
       (let ((a (if (= (length min-max) 1) (first min-max) 1.0)))
	 (let ((b (if (= a 0) 1.0 0)))
	   (let ((fa (f a)) (fb (f b)))
	     (let ((aa (if (> fb fa) b a))
		   (bb (if (> fb fa) a b))
		   (faa (max fa fb))
		   (fbb (max fa fb)))
	       (let ((bracket
		      (bracket-minimum aa bb (+ bb (* 1.6 (- bb aa)))
				       faa fbb (f (+ bb (* 1.6 (- bb aa)))))))
		 (brent-minimize
		  (second bracket)
		  (min (first bracket) (third bracket))
		  (max (first bracket) (third bracket))
		  (first bracket)
		  (third bracket)
		  (fifth bracket)
		  (fourth bracket)
		  (sixth bracket)
		  0 0))))))))

; ****************************************************************

; (minimize-multiple f tol arg1 arg2 ... argN) :
;      Minimize a function f of N arguments, given the fractional tolerance
; desired and initial guesses for the arguments.
;
; (min-arg result) : list of argument values at the minimum
; (min-val result) : list of function values at the minimum

(define (minimize-multiple-expert f tol max-iters fmin guess-args arg-scales)
  (let ((best-val 1e20) (best-args '()))
    (subplex
     (lambda (args)
       (let ((val (apply f args)))
	 (if (or (null? best-args) (< val best-val))
	     (begin
	       (print "extremization: best so far is " 
			     val " at " args "\n")
	       (set! best-val val)
	       (set! best-args args)))
	 val))
     guess-args tol max-iters
     (if fmin fmin 0.0) (if fmin true false)
     arg-scales)))

(define (minimize-multiple f tol . guess-args)
  (minimize-multiple-expert f tol 999999999 false guess-args '(0.1)))

; Yet another alternate multi-dimensional minimization (Simplex algorithm).
(define (simplex-minimize-multiple f tol . guess-args)
  (let ((simplex-result (simplex-minimize f guess-args tol)))
    (cons (simplex-point-x simplex-result)
	  (simplex-point-val simplex-result))))

; Alternate multi-dimensional minimization (using Powell's method):
; (not the default since it seems to have convergence problems sometimes)

(define (powell-minimize-multiple f tol . guess-args)
  (define (create-unit-vector i n)
    (let ((v (make-vector n 0)))
      (vector-set! v i 1)
      v))
  (define (initial-directions n)
    (make-initialized-list n (lambda (i) (create-unit-vector i n))))

  (define (v- v1 v2) (vector-map - v1 v2))
  (define (v+ v1 v2) (vector-map + v1 v2))
  (define (v* s v) (vector-map (lambda (x) (* s x)) v))
  (define (v-dot v1 v2) (vector-fold-right + 0 (vector-map * v1 v2)))
  (define (v-norm v) (sqrt (v-dot v v)))
  (define (unit-v v) (v* (/ (v-norm v)) v))

  (define (fv v) (apply f (vector->list v)))
  (define guess-vector (list->vector guess-args))
  (define (f-dir p0 dir) (lambda (x) (fv (v+ p0 (v* x dir)))))

  (define (minimize-dir p0 dir)
    (let ((min-result (minimize (f-dir p0 dir) tol)))
      (cons
       (v+ p0 (v* (min-arg min-result) dir))
       (min-val min-result))))

  (define (minimize-dirs p0 dirs)
    (if (null? dirs)
	(cons p0 '())
	(let ((min-result (minimize-dir p0 (car dirs))))
	  (let ((min-results (minimize-dirs (min-arg min-result) (cdr dirs))))
	    (cons (min-arg min-results)
		  (cons (min-val min-result) (min-val min-results)))))))

  (define (replace= val vals els el)
    (if (null? els) '()
	(if (= (car vals) val)
	    (cons el (cdr els))
	    (cons (car els) (replace= val (cdr vals) (cdr els) el)))))
  
  ; replace direction where largest decrease occurred:
  (define (update-dirs decreases dirs p0 p)
    (replace= (apply max decreases) decreases dirs (v- p p0)))

  (define (minimize-aux p0 fp0 dirs)
    (let ((min-results (minimize-dirs p0 dirs)))
      (let ((decreases (map (lambda (val) (- fp0 val)) (min-val min-results)))
	    (p (min-arg min-results))
	    (fp (first (reverse (min-val min-results)))))
	(if (<= (v-norm (v- p p0))
		(* tol 0.5 (+ (v-norm p) (v-norm p0) 1e-20)))
	    (cons (vector->list p) fp)
	    (let ((min-result (minimize-dir p (v- p p0))))
	      (minimize-aux (min-arg min-result) (min-val min-result)
			    (update-dirs decreases dirs p0 p)))))))

  (minimize-aux guess-vector (fv guess-vector)
		(initial-directions (length guess-args))))

; Maximization variants of the minimize functions:

(define (maximize f tol . min-max)
  (let ((result (apply minimize (append (list (compose - f) tol) min-max))))
    (cons (min-arg result) (- (min-val result)))))

(define (maximize-multiple f tol . guess-args)
  (let ((result (apply minimize-multiple
		       (append (list (compose - f) tol) guess-args))))
    (cons (min-arg result) (- (min-val result)))))

; ****************************************************************
; Find a root of a function of one argument using Ridder's method.

; (find-root f tol x-min x-max) : returns the root of the function (f x),
; within a fractional tolerance tol.  x-min and x-max must bracket the
; root; that is, (f x-min) must have a different sign than (f x-max).

(define (find-root f tol x-min x-max)
  (define (midpoint a b) (* 0.5 (+ a b)))
  (define (sign x) (if (< x 0) -1 1))
  
  (define (best-bracket a b x1 x2 fa fb f1 f2)
    (if (positive? (* f1 f2))
	(if (positive? (* fa f1))
	    (list (max x1 x2) b (if (> x1 x2) f1 f2) fb)
	    (list a (min x1 x2) fa (if (< x1 x2) f1 f2)))
	(if (< x1 x2)
	    (list x1 x2 f1 f2)
	    (list x2 x1 f2 f1))))

  (define (converged? a b x) (< (min (magnitude (- x a)) (magnitude (- x b))) 
				(* tol (magnitude x))))
  
  ; find the root by Ridder's method:
  (define (ridder a b fa fb)
    (if (or (= fa 0) (= fb 0))
	(if (= fa 0) a b)
	(begin
	  (if (> (* fa fb) 0)
	      (error "x-min and x-max in find-root must bracket the root!"))
	  (let ((m (midpoint a b)))
	    (let ((fm (f m)))
	      (let ((x (+ m (/ (* (- m a) (sign (- fa fb)) fm)
			       (sqrt (- (* fm fm) (* fa fb)))))))
		(if (or (= fm 0) (converged? a b x))
		    (if (= fm 0) m x)
		    (let ((fx (f x)))
		      (apply ridder (best-bracket a b x m fa fb fx fm))))))))))
	
  (ridder x-min x-max (f x-min) (f x-max)))

; ****************************************************************
; Find a root by Newton's method with bounds and bisection,
; given a function f that returns a pair of (value . derivative)

(define (find-root-deriv f tol x-min x-max . x-guess)
  ; Some trickiness: we only need to evaluate the function at x-min and
  ; x-max if a Newton step fails, and even then only if we haven't already
  ; bracketed the root, so do this via lazy evaluation.
  (define f-memo (memoize f))
  (define (lazy x) (if (number? x) x (x)))
  (define ((pick-bound which?))
    (let ((fmin-pair (f-memo x-min)) (fmax-pair (f-memo x-max)))
      (let ((fmin (car fmin-pair)) (fmax (car fmax-pair)))
	(if (which? fmin) x-min
	    (if (which? fmax) x-max
		(error "failed to bracket the root in find-root-deriv"))))))

  (define (in-bounds? x f df a b)
    (negative? (* (- f (* df (- x a)))
		  (- f (* df (- x b))))))
	  
  (define (newton x a b dx)
    (if (< (abs dx) (abs (* tol x)))
	x
	(let ((fx-pair (f-memo x)))
	  (let ((f (car fx-pair)) (df (cdr fx-pair)))
	    (if (= f 0)
		x
		(let ((a' (if (< f 0) x a)) (b' (if (> f 0) x b)))
		  (if (and (if (and (number? a) (number? b))
			       (in-bounds? x f df a b)
			       (in-bounds? x f df x-min x-max))
;			   (> (abs (* 0.5 dx df)) (abs f))
			   )
		      (newton (- x (/ f df)) a' b' (/ f df))
		      (let ((av (lazy a)) (bv (lazy b)))
			(let ((dx' (* 0.5 (- bv av)))
			      (a'' (if (eq? a a') av a'))
			      (b'' (if (eq? b b') bv b')))
			  (newton (* (+ av bv) 0.5) a'' b'' dx'))))))))))

  (newton (if (null? x-guess) (* (+ x-min x-max) 0.5) (car x-guess))
	  (pick-bound negative?)
	  (pick-bound positive?)
	  (- x-max x-min)))

; ****************************************************************

; Numerical differentiation:
;   Compute the numerical derivative of a function f at x, using
; Ridder's method of polynomial extrapolation, described e.g. in
; Numerical Recipes in C (section 5.7).

; This is the basic routine, but we wrap it in another interface below
; so that dx and tol can be optional arguments.
(define (do-derivative f x dx tol)

  ; Using Neville's algorithm, compute successively higher-order
  ; extrapolations of the derivative (the "Neville tableau"):
  (define (deriv-a a0 prev-a fac fac0)
    (if (null? prev-a)
	(list a0)
	(cons a0 (deriv-a (binary/
			   (binary- (binary* a0 fac) (car prev-a)) 
			   (- fac 1))
			  (cdr prev-a) (* fac fac0) fac0))))
  
  (define (deriv dx df0 err0 prev-a fac0)
    (let ((a (deriv-a (binary/ (binary- (f (+ x dx)) (f (- x dx))) (* 2 dx))
		      prev-a fac0 fac0)))
      (if (null? prev-a)
	  (deriv (/ dx (sqrt fac0)) (car a) err0 a fac0)
	  (let* ((errs
		  (map max
		       (map unary-abs (map binary- (cdr a) (reverse (cdr (reverse a)))))
		       (map unary-abs (map binary- (cdr a) prev-a))))
		 (errmin (apply min errs))
		 (err (min errmin err0))
		 (df (if (> err err0)
			 df0
			 (cdr (assoc errmin (map cons errs (cdr a)))))))
	    (if (or (< err (* tol (unary-abs df)) )
		    (> (unary-abs (binary- (car (reverse a)) (car (reverse prev-a))))
		       (* 2 err)))
		(list df err)
		(deriv (/ dx (sqrt fac0)) df err a fac0))))))

  (deriv dx 0 1e30 '() 2))
      
(define (do-derivative-wrap f x dx-and-tol)
  (let ((dx (if (> (length dx-and-tol) 0)
		(car dx-and-tol)
		(max (magnitude (* x 0.01)) 0.01)))
	(tol (if (> (length dx-and-tol) 1)
		 (cadr dx-and-tol)
		 0)))
    (do-derivative f x dx tol)))

(define derivative-df car)
(define derivative-df-err cadr)
(define derivative-d2f caddr)
(define derivative-d2f-err cadddr)

(define (derivative f x . dx-and-tol)
  (do-derivative-wrap f x dx-and-tol))
(define (deriv f x . dx-and-tol)
  (derivative-df (do-derivative-wrap f x dx-and-tol)))

; Compute both the first and second derivatives at the same time
; (using minimal extra function evaluations).
(define (derivative2 f x . dx-and-tol)
  (define f-memo (memoize f))
  (define (f-deriv y)
    (binary* (binary- (f-memo y) (f-memo x)) (/ 2 (- y x))))
  (append
   (do-derivative-wrap f-memo x dx-and-tol)
   (do-derivative-wrap f-deriv x dx-and-tol)))

(define (deriv2 f x . dx-and-tol)
  (derivative-d2f (apply derivative2 (cons f (cons x dx-and-tol)))))

; ****************************************************************

; Some simple integration routines using an adaptive trapezoidal rule
; (see e.g. Numerical Recipes, Sec. 4.2).  It might be nice to have
; Gaussian quadratures and what-not, but on the other hand the
; functions we are integrating may well be the result of a computation
; on a finite grid (somehow interpolated), and so will not be smooth.
; Also, implementing thse simple algorithms in Scheme lets us use our
; polymorphic arithmetic functions so that we can easily integrate
; real, complex, and vector-valued functions.

; Integrate the 1d function (f x) from x=a..b to within the specified
; fractional tolerance.
(define (integrate-1d f a b tol)
  (define (trap0 n sum)
    (binary*
     0.5
     (binary+
      sum
      (if (<= n 1)
	  (binary* (- b a) (binary+ (f a) (f b)))
	  (let ((steps (pow2 (- n 2))))
	    (let ((dx (/ (- b a) steps)))
	      (binary* 
	       dx
	       (do ((cur-sum 0) (i 0 (+ i 1)) (x (+ a dx) (+ x dx)))
		   ((>= i steps) cur-sum)
		 (set! cur-sum (binary+ cur-sum (f x)))))))))))
  (define (trap n sum)
    (let ((newsum (trap0 n sum)))
      (if (and (> n 5)
	       (or (> n 20) 
		   (binary= newsum sum)
		    (< (unary-abs (binary- newsum sum))
		       (* tol (unary-abs newsum)))))
	  newsum
	  (trap (+ n 1) newsum))))
  (trap 1 0.0))
	  
; Integrate the multi-dimensional function f from a..b, within the
; specified tolerance.  a and b are either numbers (for 1d integrals),
; or vectors/lists of the same length giving the bounds in each dimension.
(define (integrate f a b tol)
  (define (int f a b)
    (if (null? a)
	(f)
	(integrate-1d
	 (lambda (x) (int (lambda (. y) (apply f (cons x y))) (cdr a) (cdr b)))
	 (car a) (car b) tol)))
  (cond
   ((and (vector? a) (vector? b))
    (integrate f (vector->list a) (vector->list b) tol))
   ((and (number? a) (number? b))
    (integrate f (list a) (list b) tol))
   (else (int f a b))))

; ****************************************************************
