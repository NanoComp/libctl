; libctl: flexible Guile-based control files for scientific software 
; Copyright (C) 1998, 1999 Steven G. Johnson
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
; File inclusion.

; Here, we supply an (include "<filename>") utility that is similar to
; C's #include "<filename>".  We need this because Guile's load
; function is broken--it doesn't allow you to use relative paths.  If
; you use (load "<filename>"), the filename is interpreted relative to
; the path of the top-level Guile invocation, which may not be the
; same as the path of the current Scheme file.  Our include function
; remembers the path of the current file and loads relative to this.

(define (string-suffix? suff s)
  (if (> (string-length suff) (string-length s))
      #f
      (string=? suff (substring s (- (string-length s)
				     (string-length suff))
				(string-length s)))))

(define (string-find-previous-char s c)
  (if (= (string-length s) 0)
      #f
      (let ((last-index (- (string-length s) 1)))
	(if (eq? (string-ref s last-index) c)
	    last-index
	    (string-find-previous-char (substring s 0 last-index) c)))))

(define (strip-trailing-slashes s)
  (if (string-suffix? "/" s)
      (strip-trailing-slashes (substring s 0 (- (string-length s) 1)))
      s))

(define (pathname-absolute? s)
  (and (> (string-length s) 0) (eq? (string-ref s 0) #\/)))

(define (split-pathname s)
  (let ((s2 (strip-trailing-slashes s)))
    (let ((last-slash (string-find-previous-char s2 #\/)))
      (if (not last-slash)
	  (cons "" s2)
	  (cons (substring s2 0 (+ 1 last-slash))
		(substring s2 (+ 1 last-slash) (string-length s2)))))))

(define include-dir "")

(define (include pathname)
  (let ((save-include-dir include-dir)
	(pathpair (split-pathname pathname)))
    (if (pathname-absolute? (car pathpair))
	(begin
	  (set! include-dir (car pathpair))
	  (load pathname))
	(begin
	  (set! include-dir (string-append include-dir (car pathpair)))
	  (load (string-append include-dir (cdr pathpair)))))
    (set! include-dir save-include-dir)))

; ****************************************************************
