---
# Developer Experience
---

If you are thinking of using libctl in a program that you are writing, you might be rolling your eyes at this point, thinking of all the work that it will be. A full programming language? Complicated data structures? Information passing back and forth? Surely, it will be a headache to support all of these things.

In fact, however, using libctl is much easier than writing your program for a traditional, fixed-format input file. You simply describe in an abstract specifications file the variables and data types that your program expects to exchange with the ctl file, and the functions by which it is called. From these specifications, code is automatically generated to export and import the information to and from Guile.

The specifications file is written in Scheme, and consists of definitions for the classes and input/output variables the program expects. It may also contain any predefined functions or variables that might be useful in ctl files for the program, and says which functions in your program are callable from the ctl script.

Defining input variables
------------------------

To define an input variable (a variable specified by the ctl file and input into the program), use the following construction:

```scm
(define-input-var name value type [ constraints ... ])
```


Here, `name` is the name of the variable, and `value` is its initial value &mdash; so far, this is just like a normal `define` statement. However, input variables have constraints on them, the simplest of which is that they have a specific type. The *`type`* parameter can be one of:

-   `'number` &mdash; a real number
-   `'cnumber` &mdash; a complex number
-   `'integer` &mdash; an integer
-   `'vector3` &mdash; a real 3-vector
-   `'matrix3x3` &mdash; a real 3x3 matrix
-   `'cvector3` &mdash; a complex 3-vector
-   `'cmatrix3x3` &mdash; a complex 3x3 matrix
-   `'boolean` &mdash; a boolean value, `true` or `false`
-   `'string` &mdash; a string
-   `'function` &mdash; a function (in C, a Guile SCM function pointer)
-   `'class` &mdash; an member of `class`
-   `(make-list-type el-type)` &mdash; a list of elements of type `el-type`
-   `'SCM` &mdash; a generic Scheme object

Note that the quote before a type name is Scheme's way of constructing a **symbol**, which is somewhat similar to a C enumerated constant.

The final argument is an optional sequence of constraints. Each constraint is a function that, given a value, returns `true` or `false` depending on whether that value is valid. For example, if an input variable is required to be positive, one of the constraints would be the `positive?` function (predefined by Guile). More complicated functions can, of course, be constructed.

Here are a few examples:

```
(define-input-var dimensions 3 'integer positive?)
(define-input-var default-epsilon 1.0 'number positive?)
(define-input-var geometry '() (make-list-type 'geometric-object))
(define-input-var k-points '() (make-list-type 'vector3))
```


Notice that all input variables have initial values, meaning that a user need not specify a value in the ctl file if the default value is acceptable. If you want to force the user to explicitly give a value to a variable, set the initial value to `'no-value`. This way, if the variable is not set by the user, it will fail the type-constraint and an error will be flagged. Such behavior is deprecated, however.

Defining output variables
-------------------------

Output variables, which are passed from the simulation to the ctl script, are defined in a manner similar to input variables:

```scm
(define-output-var name type)
```

Notice that output variables have no initial value and no constraints. Your C program is responsible for assigning the output variables when it is called (as discussed below).

A variable can be both an input variable and an output variable at the same time. Such input-output variables are defined with the same parameters as an input variable:

```
(define-input-output-var name value type [constraints])
```

Defining classes
----------------

To define a class, one has to supply the parent class and the properties:

```scm
(define-class name parent [ properties... ])
```

`name` is the name of the new class and `parent` is the name of the parent class, or `no-parent` if there is none.

The `properties` of the class are zero or more of the following definitions, which give the name, type, default value, and (optional) constraints for a property:

```scm
(define-property name default-value type [ constraints... ])
```

`name` is the name of the property. It is okay for different classes to have properties with the same name (for example, both a sphere and a cylinder class might have `radius` properties) &mdash; however, it is important that properties with the same name have the same type. The `type` and optional `constraints` are the same as for `define-input-var`, described earlier.

If `default-value` is `no-default`, then the property has no default value and users are required to specify it. To give a property a default value, `default-value` should simply be that default value.

For example, this is how we might define classes for materials and dielectric objects in an electromagnetic simulation:

```scm
(define-class material-type no-parent
  (define-property epsilon no-default 'number positive?)
  (define-property conductivity 0.0 'number))
```

```
(define-class geometric-object no-parent
  (define-property material no-default 'material-type)
  (define-property center no-default 'vector3))
```

```
(define-class cylinder geometric-object
  (define-property axis (vector3 0 0 1) 'vector3)
  (define-property radius no-default 'number positive?)
  (define-property height no-default 'number positive?))
```

```
(define-class sphere geometric-object
  (define-property radius no-default 'number positive?))
```

### Derived Properties

Sometimes, it is convenient to store other properties with an object that are not input by the user, but which instead are computed based on the other user inputs. A mechanism is provided for this called "derived" properties, which are created by:

```
(define-derived-property name type derive-func)
```

Here, `derive-func` is a function that takes an object of the class the property is in, and returns the value of the property. See below for an example. `derive-func` is called after all of the non-derived properties of the object have been assigned their values.

### Post-Processed Properties

It is often useful to store a function of the user input into a property, instead of just storing the input itself. For example, you might want to scale an input vector so that it is stored as a unit vector. The syntax for defining such a property is the same as `define-property` except that it has one extra argument:

```scm
(define-post-processed-property name default-value type process-func [ constraints... ])
```

`process-func` is a function that takes one argument and returns a value, both of the same type as the property. Any user-specified value for the property is passed to `process-func`, and the result is assigned to the property.

Here is an example that defines a new type of geometric object, a `block`. Blocks have a `size` property that specifies their dimensions along three unit vectors, which are post-processed properties (with default values of the coordinate axes). When computing whether a point falls within a block, it is necessary to know the projection matrix, which is the inverse of the matrix whose columns are the basis vectors. We make this projection matrix a derived property, computed via the libctl-provided matrix routines, freeing us from the necessity of constantly recomputing it.

```scm
(define-class block geometric-object
  (define-property size no-default 'vector3)
```

```scm
  ; the basis vectors, which are forced to be unit-vectors
  ; by the unit-vector3 post-processing function:
  (define-post-processed-property e1 (vector3 1 0 0) 'vector3 unit-vector3)
  (define-post-processed-property e2 (vector3 0 1 0) 'vector3 unit-vector3)
  (define-post-processed-property e3 (vector3 0 0 1) 'vector3 unit-vector3)
```

```scm
  ; the projection matrix, which is computed from the basis vectors
  (define-derived-property projection-matrix 'matrix3x3
    (lambda (object)
      (matrix3x3-inverse
       (matrix3x3
        (object-property-value object 'e1)
        (object-property-value object 'e2)
        (object-property-value object 'e3))))))
```

Exporting Your Subroutines
--------------------------

In order for the ctl script to do anything, one of your C routines will eventually have to be called.

To export a C routine, you write the C routine as you would normally, using the data types defined in ctl.h and ctl-io.h (see below) for parameters and return value. All parameters must be passed by value (with the exception of strings, which are of type `char *`).

Then, in your specifications file, you must add a declaration of the following form:

```
(define-external-function name read-inputs? write-outputs? return-type [ arg0-type arg1-type ... ])
```

`name` is the name of the function, and is the name by which it will be called in a ctl script. This should be identical to the name of the C subroutine, with the exception that underscores are turned into hyphens (this is not required, but is the convention we adopt everywhere else).

If `read-inputs?` is `true`, then the input variables will be automatically imported into C global variables before the subroutine is called each time. If you don't want this to happen, this argument should be `false`. Similarly, `write-outputs?` says whether or not the output variables will be automaticaly exported from the C globals after the subroutine is called. All of this code, including the declarations of the C input/output globals, is generated automatically (see below). So, when your function is called, the input variables will already contain all of their values, and you need only assign/allocate data to the output variables to send data back to Guile. If `write-outputs?` is `true`, the output variables **must** have valid contents when your routine exits.

`return-type` is the return type of the subroutine, or `no-return-value` if there is no return value (i.e. the function is of type `void`). The remaining arguments are the types of the parameters of the C subroutine.

Usually, your program will export a `run` subroutine that performs the simulation given the input variables, and returns data to the ctl script through the output variables. Such a subroutine would be declared in C as:

```
void run(void);
```

and in the specifications file by:

```
(define-external-function run true true no-return-value)
```

As another example, imagine a subroutine that takes a geometric object and returns the fraction of electromagnetic energy in the object. It does not use the input/output global variables, and would be declared in C and in the specifications file by:

```
 /* C declaration: */
 number energy_in_object(geometric_object obj);

 ; Specifications file:
 (define-external-function energy-in-object false false
                           'number 'geometric-object)
 
```

### Data Structures and Types

The data structures for holding classes and other variable types are defined automatically in the generated file `ctl-io.h` (see below). They are fairly self-explanatory, but it should be noted that they use some data types defined in `src/ctl.h`, mostly mirrors of the corresponding Scheme types. (e.g. `number` is a synonym for `double`, and `vector3` is a structure with `x`, `y`, and `z` fields.) `ctl.h` also declares several functions for manipulating vectors and matrices, e.g. `vector3_plus`.

### Allocating and Deallocating Data

The input variables are allocated and deallocated automatically, as necessary, but you are responsible for allocating and deallocating the output data. As a convenience, the function `destroy_output_vars()` is defined, which deallocates all of the output data pointed to by the output variables. You are responsible for calling this when you want to deallocate the output.

Often, after each run, you will simply want to (re)allocate and assign the output variables. To avoid memory leaks, however, you should first deallocate the old output variables on runs after the first. To do this, use the following code:

```
if (num_write_output_vars > 0)
	destroy_output_vars();
	
/* ... allocate & assign the output variables ... */
```

The global variable `num_write_output_vars` is automatically set to the number of times the output variables have been written.

Remember, you are **required** to assign all of the output variables to legal values, or the resulting behavior will be undefined.

Other Useful Things to Put in a Specifications File
---------------------------------------------------

The specifications file is loaded before any user ctl file, making it a good place to put definitions of variables and functions that will be useful for your users. For example, the electromagnetic simulation might define a default material, `air`:

```
(define air (make material-type (epsilon 1.0)))
```

You can also define functions (or do anything else that Scheme allows), e.g. a function to duplicate geometric objects on a grid. (See the `examples/` directory of libctl for an example of this.)

To change the Guile prompt in interactive mode to your own prompt, do:

```
(ctl-set-prompt! "my prompt string")
```

We defined our own function so that we have something that works in both Guile 1.x and 2.x.

Writing your Program
--------------------

Once the specifications have been written, you have to do very little to support them in your program.

First, you need to generate C code to import/export the input/output variables from/to Guile. This is done automatically by the `gen-ctl-io` script in the `utils/` directory (installed into a `bin` directory by `make install`):

`gen-ctl-io --code specifications-file`
`gen-ctl-io --header specifications-file`

The `gen-ctl-io` commands above generate two files, `ctl-io.h` and `ctl-io.c`. The former defines global variables and data structures for the input/output variables and classes, and the latter contains code to exchange this data with Guile.

Second, you should use the `main.c` file from the `base/` directory; if you use the example `Makefile` (see below), this is done automatically for you. This file defines a main program that starts up Guile, declares the routines that you are exporting, and loads control files from the command line. You should not need to modify this file, but you should define preprocessor symbols telling it where libctl and your specification file are (again, this is done for you automatically by the example `Makefile`).

For maximum convenience, if you are wisely using GNU autoconf, you should also copy the `Makefile.in` from `examples/`; you can use the `Makefile` otherwise. At the top of this file, there are places to specify your object files, specification file, and other information. The `Makefile` will then generate the `ctl-io` files and do everything else needed to compile your program.

You then merely need to write the functions that you are exporting (see above for how to export functions). This will usually include, at least, a `run` function (see above).

The default `main.c` handles a couple of additional command-line options, including `--verbose` (or `-v`), which sets a global variable `verbose` to 1 (it is otherwise 0). You can access this variable (it is intended to enable verbose output in programs) by declaring the global `extern int verbose` in your program.
