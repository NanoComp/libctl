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
; define-param: defining local variables that can be set easily
; from the command-line (or assume a default value if not set).

(define params-set-list '())
(defmacro-public define-param (name value)
  `(if (not (defined? (quote ,name)))
       (define ,name ,value)))

(defmacro-public set-param! (name value)
  `(if (not (memq (quote ,name) params-set-list))
       (set! ,name ,value)))

; ****************************************************************
; Input/Output variables.

(define input-var-list '())
(define output-var-list '())

(define (make-var value-thunk var-name var-type-name var-constraints)
  (list var-name var-type-name var-constraints value-thunk))
(define (var-name var) (first var))
(define (var-type-name var) (second var))
(define (var-constraints var) (third var))
(define (var-value-thunk var) (fourth var))
(define (var-value var) ((var-value-thunk var)))

(define (input-var! value-thunk var-name var-type-name . var-constraints)
  (let ((new-var (make-var value-thunk var-name 
			   var-type-name var-constraints)))
    (set! input-var-list (cons new-var input-var-list))
    new-var))
(define (output-var! value-thunk var-name var-type-name)
  (let ((new-var (make-var value-thunk var-name
			   var-type-name no-constraints)))
    (set! output-var-list (cons new-var output-var-list))
    new-var))

(defmacro-public define-input-var
  (name init-val var-type-name . var-constraints)
  `(begin
     (define-param ,name ,init-val)
     (input-var! (lambda () ,name) (quote ,name)
		 ,var-type-name ,@var-constraints)))

(defmacro-public define-input-output-var
  (name init-val var-type-name . var-constraints)
  `(begin
     (define ,name ,init-val)
     (input-var! (lambda () ,name) (quote ,name)
		 ,var-type-name ,@var-constraints)
     (output-var! (lambda () ,name) (quote ,name) ,var-type-name)))

(defmacro-public define-output-var (name var-type-name)
  `(begin
     (define ,name 'no-value)
     (output-var! (lambda () ,name) (quote ,name) ,var-type-name)))

(define (check-vars var-list)
  (for-all? var-list
	    (lambda (v) 
		   (if (not (check-type (var-type-name v) (var-value v)))
		       (error "wrong type for variable" (var-name v) 'type
			      (var-type-name v))
		       (if (not (check-constraints 
				 (var-constraints v) (var-value v)))
			   (error "failed constraint for" (var-name v))
			   true)))))

; ****************************************************************
