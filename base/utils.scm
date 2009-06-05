; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998-2009, Steven G. Johnson
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
; Replacements for MIT Scheme functions missing from Guile 1.2.

(define true #t)
(define false #f)

(define (list-transform-positive l pred)
  (define (tp Lrest Lpos)
    (if (null? Lrest)
	Lpos
	(if (pred (car Lrest))
	    (tp (cdr Lrest) (cons (car Lrest) Lpos))
	    (tp (cdr Lrest) Lpos))))
  (reverse (tp l '())))

(define (list-transform-negative l pred)
  (list-transform-positive l (lambda (x) (not (pred x)))))

(define (alist-copy al)
  (if (null? al) '()
      (cons (cons (caar al) (cdar al)) (alist-copy (cdr al)))))

(define (for-all? l pred)
  (if (null? l)
      true
      (if (pred (car l))
	  (for-all? (cdr l) pred)
	  false)))

(define (first list) (list-ref list 0))
(define (second list) (list-ref list 1))
(define (third list) (list-ref list 2))
(define (fourth list) (list-ref list 3))
(define (fifth list) (list-ref list 4))
(define (sixth list) (list-ref list 5))

; fold-left and fold-right: combine elements of list using an operator
; op, with initial element init, associating from the right or from
; the left.  These two are equivalent if op is associative.

(define (fold-left op init list)
  (if (null? list)
      init
      (fold-left op (op init (car list)) (cdr list))))

(define (fold-right op init list)
  (fold-left (lambda (x y) (op y x)) init (reverse list)))

; ****************************************************************
; Miscellaneous utility functions.

(define (compose f g) (lambda args (f (apply g args))))

(define (car-or-x p) (if (pair? p) (car p) p))

(define (sqr x) (* x x))

; complex conjugate of x:
(define (conj x) (make-rectangular (real-part x) (- (imag-part x))))

; combine 2 alists.  returns a list containing all of the associations
; in a1 and any associations in a2 that are not in a1
(define (combine-alists a1 a2)
  (if (null? a2)
      a1
      (combine-alists
       (if (assoc (caar a2) a1) a1 (cons (car a2) a1))
       (cdr a2))))

(define (vector-for-all? v pred) (for-all? (vector->list v) pred))

(define (vector-fold-right op init v)
  (fold-right op init (vector->list v)))

(define (vector-fold-left op init v)
  (fold-left op init (vector->list v)))

(define (vector-map func . v)
  (list->vector (apply map (cons func (map vector->list v)))))

(define (indent indentby)
  (print (make-string indentby #\space)))

(define print-ok? true) ; so that the user can disable output

(define (print . items)
  (if print-ok? 
      (begin
	(for-each (lambda (item) (display item)) items)
	(flush-all-ports))))

(define display-many print) ; backwards compatibility with earlier libctl

(define (make-initialized-list size init-func)
  (define (aux i)
    (if (>= i size) '()
	(cons (init-func i) (aux (+ i 1)))))
  (aux 0))

; ****************************************************************

; Some string utilities:

(define (string-find-next-char-in-list s l)
  (define (aux index s)
    (if (string-null? s)
	#f
	(if (member (string-ref s 0) l)
	    index
	    (aux (+ index 1) (substring s 1 (string-length s))))))
  (aux 0 s))

(define (string-find-next-char-not-in-list s l)
  (define (aux index s)
    (if (string-null? s)
	#f
	  (if (not (member (string-ref s 0) l))
	      index
	      (aux (+ index 1) (substring s 1 (string-length s))))))
  (aux 0 s))

(define (string->positive-integer s)
  (let ((non-blank (string-find-next-char-not-in-list
		    s '(#\space #\ht #\vt #\nl #\cr))))
    (let ((s2 (if (eq? non-blank #f)
		  s (substring s non-blank (string-length s)))))
      (let ((int-start (string-find-next-char-in-list
			s2 (string->list "0123456789"))))
	(if (eq? int-start 0)
	    (let ((int-end (string-find-next-char-not-in-list
			    (substring s2 1 (string-length s2))
			    (string->list "0123456789"))))
	      (if (eq? int-end #f)
		  (eval-string s2)
		  (if (string-find-next-char-not-in-list
		       (substring s2 (+ 1 int-end) (string-length s2))
		       '(#\space #\ht #\vt #\nl #\cr))
		      #f
		      (eval-string s2))))
	    #f)))))

; ****************************************************************

; timing functions

; Display the message followed by the time t in minutes and seconds,
; returning t in seconds.
(define (display-time message t)
  (let ((hours (quotient t 3600))
	(minutes (remainder (quotient t 60) 60))
	(seconds (remainder t 60)))
    (print message)
    (if (> hours 1)
	(print hours " hours, ")
	(if (> hours 0)
	    (print hours " hour, ")))
    (if (> minutes 1)
	(print minutes " minutes, ")
	(if (> minutes 0)
	    (print minutes " minute, ")))
    (print seconds " seconds.\n"))
  t)

; (begin-time message ...statements...) works just like (begin
; ...statements...) except that it also displays 'message' followed by
; the elapsed time to execute the statements.  Additionally, it returns
; the elapsed time in seconds, rather than the value of the last statement.
(defmacro-public begin-time (message . statements)
  `(begin
     (let ((begin-time-start-t (current-time)))
       ,@statements
       (display-time ,message (- (current-time) begin-time-start-t)))))

; like begin-time, but returns the value of the last statement,
; similar to (begin ...).  In retrospect, returning the time by
; default was probably a mistake.
(defmacro-public begin-timed (message . statements)
  `(let ((begin-time-start-t (current-time)))
     (let ((val (begin "no statements" ,@statements)))
       (display-time ,message (- (current-time) begin-time-start-t))
       val)))

; ****************************************************************

; Return a 'memoized' version of the function f, which caches its
; arguments and return values so as never to compute the same thing twice.

(define (memoize f)
  (let ((f-memo-tab '()))
    (lambda (. y)
      (let ((tab-val (assoc y f-memo-tab)))
	(if tab-val
	    (cdr tab-val)
	    (let ((fy (apply f y)))
	      (set! f-memo-tab (cons (cons y fy) f-memo-tab))
	      fy))))))

; ****************************************************************
