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
; Defining external functions.

(define (make-external-function name read-inputs? write-outputs?
				return-type-name arg-type-names)
  (list name read-inputs? write-outputs? return-type-name arg-type-names))
(define external-function-name first)
(define external-function-read-inputs? second)
(define external-function-write-outputs? third)
(define external-function-return-type-name fourth)
(define external-function-arg-type-names fifth)

(define no-return-value 'none)

(define external-function-list '())
(define (external-function! name read-inputs? write-outputs?
			    return-type-name . arg-type-names)
  (set! external-function-list
	(cons (make-external-function name read-inputs? write-outputs?
				      return-type-name arg-type-names)
	      external-function-list)))

(define (external-function-aux-name name)
  (symbol-append name '-aux))
  
(define (check-arg-types name args . arg-type-names)
  (if (not (= (length args) (length arg-type-names)))
      (begin
	(print "Expecting " (length arg-type-names) " arguments for "
		      name)
	(print "\n")
	(error "Wrong number of arguments for function" name))
      (for-each
       (lambda (arg arg-type-name)
	 (if (not (check-type arg-type-name arg))
	     (error "wrong type for argument" 'type arg-type-name 'in name)))
       args arg-type-names)))

(defmacro-public define-external-function
  (name read-inputs? write-outputs? return-type-name . arg-type-names)
  `(begin
     (define ,name
       (lambda args
	 (check-arg-types (quote ,name) args ,@arg-type-names)
	 (if ,read-inputs? (read-input-vars))
	 (let ((return-value
		(apply ,(external-function-aux-name name) args)))
	   (if ,write-outputs? (write-output-vars))
	      return-value)))
     (external-function! (quote ,name) ,read-inputs? ,write-outputs?
			 ,return-type-name ,@arg-type-names)))

; ****************************************************************
