; ***************************************************************************

; "Standard" Scheme functions missing from Guile 1.2:

(define (string-upcase s)
  (list->string (map (lambda (c)
		       (if (and (char>=? c #\a) (char<=? c #\z))
			   (integer->char (+ (char->integer c)
					     (char->integer #\A)
					     (- (char->integer #\a))))
			   c))
		     (string->list s))))

; ***************************************************************************

(define (c-identifier s)
  (list->string (map (lambda (c) 
		       (if (or (eq? c #\-) (eq? c #\space)) #\_ c))
		     (string->list s))))

(define symbol->c-identifier (compose c-identifier symbol->string))

(define c-type-string (compose c-identifier type-string))

(define declared-type-names '())
(define (declare-type-name type-name)
  (if (and (list-type-name? type-name)
	   (not (member (c-type-string type-name) declared-type-names)))
      (begin
	(if (list-type-name? (list-el-type-name type-name))
	    (declare-type-name (list-el-type-name type-name)))
	(display "typedef struct {") (newline)
	(display "int num_items;") (newline)
	(display-many (c-type-string (list-el-type-name type-name))
		      " *items;") (newline)
	(display-many "} " (c-type-string type-name) ";") (newline)
        (newline)
	(set! declared-type-names (cons (c-type-string type-name)
					declared-type-names)))))

(define (c-var-decl var-name var-type-name)
  (display-many (c-type-string var-type-name) " " 
		(symbol->c-identifier var-name) ";")
  (newline))

; ***************************************************************************

(define (find-direct-subclasses class)
  (list-transform-positive class-list
    (lambda (c) (eq? (class-parent c) class))))

(define (class-identifier class)
  (symbol->c-identifier (class-type-name class)))

(define (class-enum-name class)
  (string-upcase (class-identifier class)))

(define (c-class-decl class)
  (for-each (compose declare-type-name property-type-name)
	    (class-properties class))
  (display-many "typedef struct " (class-identifier class)
		"_struct {") (newline)
  (for-each 
   (lambda (property)
     (c-var-decl (property-name property) (property-type-name property)))
   (class-properties class))
  (let ((subclasses (find-direct-subclasses class)))
    (if (not (null? subclasses))
	(begin
	  (display-many "enum { " (class-enum-name class))
	  (for-each (lambda (sc) (display-many ", " (class-enum-name sc)))
		    subclasses)
	  (display " } which_subclass;") (newline)
	  (display "union {") (newline)
	  (for-each (lambda (sc)
		      (display-many
		       "struct " (class-identifier sc) "_struct *"
		       (class-identifier sc) "_data;") (newline))
		    subclasses)
	  (display "} subclass;") (newline))))
  (display-many "} " (class-identifier class) ";")
  (newline) (newline))
       
(define (display-c-class-decls)
  (display "/******* Type declarations *******/") (newline) (newline)
  (for-each c-class-decl (reverse class-list)))

; ***************************************************************************

(define (declare-var var)
  (c-var-decl (var-name var) (var-type-name var)))
(define (declare-extern-var var)
  (display "extern ")
  (c-var-decl (var-name var) (var-type-name var)))

(define (declarer-if-not-input-var declarer)
  (lambda (var)
    (if (not (member var input-var-list))
	(declarer var)
	(begin (display-many "/* " (var-name var) 
			     " is both input and output */") (newline)))))

(define (declare-var-types)
  (for-each (compose declare-type-name var-type-name)
	    (append (reverse input-var-list) (reverse output-var-list))))

(define (declare-vars declarer)
  (display "/******* Input variables *******/") (newline)
  (for-each declarer (reverse input-var-list))
  (newline)
  (display "/******* Output variables *******/") (newline)
  (for-each (declarer-if-not-input-var declarer) (reverse output-var-list))
  (newline))

(define (declare-vars-header)
  (declare-var-types)
  (declare-vars declare-extern-var))
(define (declare-vars-source) (declare-vars declare-var))

; ***************************************************************************

(define (input-value s-var-name-str c-var-name-str type-name getter)
  (let ((desc (get-type-descriptor type-name)))
    (cond
     ((eq? (type-descriptor-kind desc) 'simple)
      (display-many c-var-name-str " = " (getter type-name s-var-name-str) ";")
      (newline))
     ((eq? (type-descriptor-kind desc) 'object)
      (display-many (class-input-function-name type-name) "("
		    (getter 'object s-var-name-str) ", &" c-var-name-str ");")
      (newline))
     ((eq? (type-descriptor-kind desc) 'uniform-list)
      (input-list (getter 'list s-var-name-str) c-var-name-str
		  (list-el-type-name type-name))))))

(define (get-global type-symbol name-str)
  (string-append "ctl_get_" (symbol->c-identifier type-symbol)
		 "(\"" name-str "\")" ))

(define (property-getter object-name-str)
  (lambda (type-symbol name-str)
    (string-append (symbol->c-identifier type-symbol)
		   "_object_property(" object-name-str ", "
		   "\"" name-str "\")" )))

(define (list-getter lo-name-str)
  (lambda (type-symbol name-str)
    (string-append (symbol->c-identifier type-symbol)
                   "_list_ref(" lo-name-str ", "
                   name-str ")" )))

(define list-temp-suffix "_t")

(define (input-list list-object-get-str c-var-name-str type-name)
  (display "{") (newline)
  (let ((lo-name-str (string-append "lo" list-temp-suffix))
	(index-name-str (string-append "i" list-temp-suffix))
	(saved-list-temp-suffix list-temp-suffix))
    (set! list-temp-suffix (string-append list-temp-suffix "_t"))
    (display-many "list " lo-name-str " = "
		  list-object-get-str ";")
    (newline)
    (display-many "int " index-name-str ";") (newline)
    (display-many c-var-name-str ".num_items = list_length(" lo-name-str ");")
    (newline)
    (display-many c-var-name-str ".items = (" 
		  (c-type-string type-name)
		  "*) malloc(sizeof("
		  (c-type-string type-name) ") * "
		  c-var-name-str ".num_items);")
    (newline)
    (display-many "for (" index-name-str " = 0; " index-name-str " < "
		  c-var-name-str ".num_items; " index-name-str "++) {")
    (newline)
       (input-value index-name-str 
		    (string-append c-var-name-str ".items[" index-name-str "]")
		    type-name (list-getter lo-name-str))
    (display "}") (newline)
    (set! list-temp-suffix saved-list-temp-suffix))
  (display "}") (newline))

(define (class-input-function-name type-name)
  (string-append (symbol->c-identifier type-name)
		 "_input"))

(define (class-input-function-decl class)
  (display-many "void " 
		(class-input-function-name (class-type-name class))
		"(SCM so, "
		(c-type-string (class-type-name class)) " *o)"))

(define (class-input-function class)
  (class-input-function-decl class) (newline)
  (display "{") (newline)
  (for-each
   (lambda (property)
     (input-value (symbol->string (property-name property))
		  (string-append "o->" (symbol->c-identifier 
					(property-name property)))
		  (property-type-name property)
		  (property-getter "so")))
   (class-properties class))
  (let ((subclasses (find-direct-subclasses class)))
    (for-each
     (lambda (sc)
       (display-many "if (object_is_member(\"" (class-type-name sc) 
		     "\", so)) {")
       (newline)
       (display-many "o->which_subclass = " (class-enum-name sc) ";")
       (newline)
       (display-many "o->subclass." (class-identifier sc)
		     "_data = (" (class-identifier sc)
		     "*) malloc(sizeof("
		     (class-identifier sc) "));")
       (newline)
       (display-many (class-input-function-name (class-type-name sc)) 
		     "(so, o->subclass." (class-identifier sc) "_data);")
       (newline)
       (display "}") (newline)
       (display "else "))
     subclasses)
   (if (not (null? subclasses))
       (begin
	 (newline)
	 (display-many "o->which_subclass = " (class-enum-name class) ";")
	 (newline))))
  (display "}") (newline) (newline))

(define (output-class-input-functions-header)
  (display "/******* class input function prototypes *******/")
  (newline) (newline)
  (for-each
   (lambda (class)
     (display "extern ") (class-input-function-decl class)
     (display ";") (newline))
   class-list)
  (newline))

(define (output-class-input-functions-source)
  (display "/******* class input functions *******/") (newline) (newline)
  (for-each class-input-function class-list))

(define (input-vars-function)
  (display "/******* read input variables *******/") (newline) (newline)
  (display "SCM read_input_vars(void)") (newline)
  (display "{") (newline)
  (display "if (num_runs++) destroy_input_vars();") (newline)
  (for-each
   (lambda (var)
     (input-value
      (symbol->string (var-name var))
      (symbol->c-identifier (var-name var))
      (var-type-name var)
      get-global))
   (reverse input-var-list))
  (display "return SCM_UNSPECIFIED;") (newline)
  (display "}") (newline) (newline))

; ***************************************************************************

(define (export-object-value c-var-name-str type-name exporter)
  (error "object output variables are not yet supported. "
	 type-name c-var-name-str))

(define (export-list-value c-var-name-str type-name exporter)
  (let ((el-type-name (list-el-type-name type-name)))
    (let ((el-desc (get-type-descriptor el-type-name)))
      (cond
       ((eq? (type-descriptor-kind el-desc) 'simple)
	(exporter (string-append "make_" (type-descriptor-name-str el-desc)
				 "_list("
				 c-var-name-str ".num_items, "
				 c-var-name-str ".items)")))
       (else
	(error
	 "only export of lists of simple types is currently supported, not "
	 el-type-name))))))

(define (output-value s-var-name-str c-var-name-str type-name setter)
  (let ((desc (get-type-descriptor type-name)))
    (cond
     ((eq? (type-descriptor-kind desc) 'simple)
      (display-many (setter type-name s-var-name-str c-var-name-str))
      (newline))
     ((eq? (type-descriptor-kind desc) 'object)
      (export-object-value c-var-name-str type-name
			   (lambda (sobj-str)
			     (display 
			      (setter 'object s-var-name-str sobj-str))
			      (newline))))
     ((eq? (type-descriptor-kind desc) 'uniform-list)
      (export-list-value c-var-name-str type-name
			 (lambda (slist-str)
			   (display
			    (setter 'list s-var-name-str slist-str))
			    (newline)))))))

(define (set-global type-symbol s-name-str c-name-str)
  (string-append "ctl_set_" (symbol->c-identifier type-symbol)
                 "(\"" s-name-str "\", " c-name-str ");" ))

(define (output-vars-function)
  (display "/******* write output variables *******/") (newline) (newline)
  (display "SCM write_output_vars(void)") (newline)
  (display "{") (newline)
  (for-each
   (lambda (var)
     (output-value
      (symbol->string (var-name var))
      (symbol->c-identifier (var-name var))
      (var-type-name var)
      set-global))
   (reverse output-var-list))
  (display "return SCM_UNSPECIFIED;") (newline)
  (display "}") (newline) (newline))

; ***************************************************************************

(define (destroy-c-var var-str type-name)
  (let ((desc (get-type-descriptor type-name)))
    (cond 
     ((eq? (type-descriptor-kind desc) 'uniform-list)
      (destroy-list var-str (list-el-type-name type-name)))
     ((eq? (type-descriptor-kind desc) 'object)
      (destroy-object var-str type-name)))))

(define (class-destroy-function-name type-name)
  (string-append (symbol->c-identifier type-name)
		 "_destroy"))

(define (class-destroy-function-decl class)
  (display-many "void " 
		(class-destroy-function-name (class-type-name class))
		"("
		(c-type-string (class-type-name class)) " o)"))

(define (destroy-list var-str el-type-name)
  (let ((index-name (string-append "index" list-temp-suffix))
	(saved-suffix list-temp-suffix))
    (set! list-temp-suffix (string-append list-temp-suffix "_t"))
    (display "{") (newline)
       (display-many "int " index-name ";") (newline)
       (display-many "for (" index-name " = 0; " index-name " <= "
		     var-str ".num_items; " index-name "++) {")
       (newline)
       (destroy-c-var (string-append var-str ".items[" index-name "]")
		    el-type-name)
       (display "}") (newline)
    (display "}") (newline)
    (display-many "free(" var-str ".items);") (newline)
    (set! list-temp-suffix saved-suffix)))

(define (destroy-object var-str type-name)
  (display-many (class-destroy-function-name type-name) "(" var-str ");")
  (newline))

(define (destroy-property prefix-str property)
  (destroy-c-var (string-append prefix-str (symbol->c-identifier
					  (property-name property)))
		 (property-type-name property)))

(define (class-destroy-function class)
  (class-destroy-function-decl class) (newline)
  (display "{") (newline)
  (for-each
   (lambda (property) (destroy-property "o." property))
   (class-properties class))
  (let ((subclasses (find-direct-subclasses class)))
    (for-each
     (lambda (sc)
       (display-many "if (o.which_subclass == " (class-enum-name sc) ") {")
       (newline)
       (destroy-object (string-append "*o.subclass." 
				      (class-identifier sc) "_data")
		       (class-type-name sc))
       (display-many "free(o.subclass." (class-identifier sc) "_data);")
       (newline)
       (display "}") (newline)
       (display "else "))
     subclasses)
   (if (not (null? subclasses))
       (begin
	 (display "{ }")
	 (newline))))
  (display "}") (newline) (newline))

(define (output-class-destruction-functions-header)
  (display "/******* class destruction function prototypes *******/")
  (newline) (newline)
  (for-each
   (lambda (class)
     (display "extern ") (class-destroy-function-decl class)
     (display ";") (newline))
   class-list)
  (newline))

(define (output-class-destruction-functions-source)
  (display "/******* class destruction functions *******/") (newline) (newline)
  (for-each class-destroy-function class-list))

(define (destroy-input-vars-function)
  (display "/******* destroy input variables *******/") (newline) (newline)
  (display "SCM destroy_input_vars(void)") (newline)
  (display "{") (newline)
  (for-each
   (lambda (var)
     (destroy-c-var
      (symbol->c-identifier (var-name var))
      (var-type-name var)))
   (reverse input-var-list))
  (display "return SCM_UNSPECIFIED;") (newline)
  (display "}") (newline) (newline))

(define (destroy-output-vars-function)
  (display "/******* destroy output variables *******/") (newline) (newline)
  (display "SCM destroy_output_vars(void)") (newline)
  (display "{") (newline)
  (for-each
   (lambda (var)
     (if (not (member var input-var-list))
	 (destroy-c-var
	  (symbol->c-identifier (var-name var))
	  (var-type-name var))))
   (reverse output-var-list))
  (display "return SCM_UNSPECIFIED;") (newline)
  (display "}") (newline) (newline))

; ***************************************************************************

(define (output-header)
  (display-c-class-decls)
  (declare-vars-header)
  (display "extern int num_runs;") (newline)
  (newline)
  (display "extern SCM read_input_vars(void);") (newline)
  (display "extern SCM write_output_vars(void);") (newline)
  (display "extern SCM destroy_input_vars(void);") (newline)
  (display "extern SCM destroy_output_vars(void);") (newline))

(define (output-source)
  (declare-vars-source)
  (display "int num_runs = 0; /* number of calls to read_input_vars */")
  (newline) (newline)
  (output-class-input-functions-header)
  (output-class-destruction-functions-header)
  (output-class-input-functions-source)
  (output-class-destruction-functions-source)
  (input-vars-function)
  (output-vars-function)
  (destroy-input-vars-function)
  (destroy-output-vars-function))

