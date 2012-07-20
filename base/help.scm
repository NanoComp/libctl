; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998-2012 Massachusetts Institute of Technology and Steven G. Johnson
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

(define (display-class indentby class)
  (indent indentby)
  (print "Class " (class-type-name class) ": ")
  (print "\n")
  (if (class-parent class)
      (display-class (+ indentby 4) (class-parent class)))
  (for-each
   (lambda (property)
     (if (not (property-derived? property))
	 (begin 
	   (indent (+ indentby 4))
	   (print (type-string (property-type-name property)) " "
			 (property-name property))
	   (if (property-has-default? property)
	       (print " = " (property-default-value property)))
	   (print "\n"))))
   (class-properties class)))
			  
(define (class-help class) (display-class 0 class))

(define (variable-help var)
  (print (type-string (var-type-name var)) " "
		(var-name var) " = " (var-value var))
  (print "\n"))

(define (help)
  (for-each class-help class-list)
  (print "\n")
  (print "Input variables: ") (print "\n")
  (for-each variable-help input-var-list)
  (print "\n")
  (print "Output variables: ") (print "\n")
  (for-each variable-help output-var-list))

; ****************************************************************
