; Copyright (C) 1998, 1999, 2000 Steven G. Johnson
;
; This file may be used without restriction.  It is in the public
; domain, and is NOT restricted by the terms of any GNU license.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Lesser General Public License for more details. 

(define-class material-type no-parent
  (define-property epsilon 'number no-default positive?)
  (define-property conductivity 'number (make-default 0.0)))

; use the solid geometry classes, variables, etcetera in libgeom:
; (one specifications file can include another specifications file)
(include "../utils/geom.scm")

; ****************************************************************

; Add some predefined variables, for convenience:

(define vacuum (make material-type (epsilon 1.0)))
(define air vacuum)

(define infinity 1.0e20) ; big number for infinite dimensions of objects

(set! default-material air)

; ****************************************************************

(define-input-var k-points '() (make-list-type 'vector3))

(define-input-output-var dummy (vector3 3.7 2.3 1.9) 'vector3)

(define-output-var mean-dielectric 'number)

(define-output-var gaps (make-list-type 'number))

; ****************************************************************

(define-external-function run-program true true
  no-return-value)

(define (run)
  (set! interactive? #f)  ; don't be interactive if we call (run)
  (run-program))

(define-external-function energy-in-object false false
  'number 'geometric-object)
