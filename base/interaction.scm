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

; Utilities to make it easier to interact with the user.

(define (yes/no? question)
  (print question " (y/n) ") (flush-all-ports)
  (let ((input (read-line)))
    (let ((c (if (string-null? input) #f (string-ref input 0))))
      (cond
       ((or (eq? c #\y) (eq? c #\Y)) #t)
       ((or (eq? c #\n) (eq? c #\N)) #f)
       (else (print "  -- please enter y or n\n")
	     (yes/no? question))))))

(define (menu-choice . items)
  (define (display-items index items)
    (if (not (null? items))
	(begin
	  (print "   " index ". " (car items) "\n")
	  (display-items (+ index 1) (cdr items)))))
  
  (define (get-choice n)
    (print "Enter your selection (1.." n ") ==> ") (flush-all-ports)
    (let ((input (read-line)))
      (let ((choice (string->positive-integer input)))
	(if (or (eq? choice #f) (< choice 1) (> choice n))
	    (begin
	      (print "  -- invalid selection!\n")
	      (get-choice n))
	    choice))))

  (if (null? items)
      #f
      (begin
	(display-items 1 items)
	(- (get-choice (length items)) 1))))

; ****************************************************************
