; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998 Steven G. Johnson
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

(define (display-class indentby class)
  (indent indentby)
  (display-many "Class " (class-type-name class) ": ")
  (newline)
  (if (class-parent class)
      (display-class (+ indentby 4) (class-parent class)))
  (for-each
   (lambda (property)
     (if (not (property-derived? property))
	 (begin 
	   (indent (+ indentby 4))
	   (display-many (type-string (property-type-name property)) " "
			 (property-name property))
	   (if (property-has-default? property)
	       (display-many " = " (property-default-value property)))
	   (newline))))
   (class-properties class)))
			  
(define (class-help class) (display-class 0 class))

(define (variable-help var)
  (display-many (type-string (var-type-name var)) " "
		(var-name var) " = " (var-value var))
  (newline))

(define (help)
  (for-each class-help class-list)
  (newline)
  (display "Input variables: ") (newline)
  (for-each variable-help input-var-list)
  (newline)
  (display "Output variables: ") (newline)
  (for-each variable-help output-var-list))

; ****************************************************************
