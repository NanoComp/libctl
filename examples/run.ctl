; Copyright (C) 1998 Steven G. Johnson
;
; This file may be used without restriction.  It is in the public
; domain, and is NOT restricted by the terms of any GNU license.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Library General Public License for more details. 

; Sample control file (for photonic-crystal.scm specification).
; You should edit this file in Scheme mode if you are using emacs:
; type M-x scheme-mode or modify your .emacs file to automatically
; put .ctl files in Scheme mode.

; Define some dielectric materials:
(define GaAs (make material-type (epsilon 11.56)))
(define AlOx (make material-type (epsilon 2.25)))

; Set the dimensions.  Use set! rather than define to change the value
; of an existing variable.
(set! dimensions 2)

; Set the k-point list:
(set! k-points 
  (list
   (vector3 0 0)
   (vector3 0.5 0)
   (vector3 0.5 0.5)
   (vector3 0 0)))

; Reset the k-point list for fun, in a fancier way:
; Here, we will use the built-in interpolate function to interpolate
; points between the corners of the Brillouin zone.

(define Gamma-point (vector3 0 0))
(define X-point (vector3 0.5 0))
(define M-point (vector3 0.5 0.5))
(set! k-points (interpolate 4 (list Gamma-point X-point M-point Gamma-point)))

; Set the geometry:
(set! geometry
      (list
       (make cylinder (material air) (center 2 0) (radius 0.2) (height 0.5))
       (make sphere (material GaAs) (center 0.1 -0.1) (radius 0.1))
       (make block (material AlOx) (center 1 2) (size 3.2 2.3))))

; Append a 2d 3x3 lattice of cylinders to the geometry:

(set! geometry
      (append geometry
	      (geometric-objects-duplicates
	       (vector3 1 0) -1 1
	       (geometric-object-duplicates
		(vector3 0 1) -1 1
		(make cylinder (material GaAs) 
		      (center 0 0) (radius 0.1) (height 1.0))))))
					    

; Normally, at the end of the input file, we call (run) to run
; the simulation.  Here, though, we comment it out--when we don't
; call (run), we are dropped into interactive mode, where we can
; enter more commands and/or type (run) manually.

; (run)
