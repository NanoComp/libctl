---
# Basic User Experience
---

At their most basic level, **ctl** files are simply a collection of values for parameters required by the simulation.

The ctl syntax for all programs using libctl is similar, although the specific parameters needed will vary. The following examples are given for a fictitious libctl-using program, in order to illustrate its general style.

A Fictitious Example
--------------------

For example, suppose that the simulation solves a one-dimensional differential equation and requires an input called "grid-size" specifying the number of grid points used in the discretization of the problem. We might specify this in a ctl file by the statement:

```scm
(set! grid-size 128)
```

All input variable settings can follow the format `(set! variable value)`. The parentheses are important, but white space is ignored. Alternatively, we can use:

```scm
(set-param! grid-size 128)
```

which works exactly like `set!` except that now `grid-size` can be overridden from the command-line. For this reason, `set-param!` (a libctl extension to Scheme) is usually preferred. See also [Command-line parameters](Advanced_User_Experience#command-line-parameters).

Settings of input variables can appear in any order in the file. They can even be omitted completely in many cases, and a reasonable default will be used. Variables can be of many different types, including integers, real numbers, boolean values (`true` and `false`), strings, 3-vectors, and lists. Here is how we might set some parameters of various types:

```scm
(set-param! time-step-dt 0.01)                       ; a real number
(set-param! output-file-name "data.hdf")             ; a string
(set-param! propagation-direction (vector3 0 0.2 7)) ; a 3-vector
(set! output-on-time-steps                           ; a list of integers...
 (list 25 1000
         257 128 4096))
```

Everything appearing on a line after a semicolon (";") is a **comment** and is ignored. Note also that we are free to split inputs over several lines &mdash; as we mentioned earlier, white space is ignored.

3-vectors are constructed using `(vector3 x [y [z]])`. If the *y* or *z* components are omitted, they are set to zero. Lists may contain any number of items (including zero items), and are constructed with `(list [item1 item2 ...])`.

A typical control file is terminated with a single statement, something like:

```
(run) ; run the computation
```

This tells the program to run its computation with whatever parameter values have been specified up to the point of the `(run)`. This command can actually appear multiple times in the ctl file, causing multiple runs, or not at all, which drops the user into an interactive mode that we will discuss later.

Running a Simulation
--------------------

The user runs the simulation program simply by:

*program ctl-files*

Here, *`program`* is the name of the simulation program executable and *ctl-files* are any ctl files that you want to use for the run. The result is as if all the *ctl-files* were concatenated, in sequence, into a single file.

Structured Data Types
---------------------

For many programs, it is useful to structure the input into more complicated data types than simple numbers, vectors, and lists. For example, an electromagnetic simulation might take as input a list of geometric objects specifying the dielectric structure. Each object might have several parameters &mdash; for example, a sphere might have a radius, a center, and a dielectric constant.

libctl allows programs to specify structured datatypes, called **classes**, that have various properties which may be set. Here is what a list of geometric objects for a dielectric structure might look like:

```scm
(set! geometry
   (list
      (make sphere (epsilon 2.8) (center 0 0 1) (radius 0.3))
      (make block (epsilon 1.7) (center 0 0 1) (size 1 3.5 2)))) 
```

In this case, the list consists of two objects of classes called `sphere` and `block`. The general format for constructing an object (instance of a class) is `(make class properties)`. *Properties* is a sequence of `(property value)` items setting the properties of the object.

Properties may have default values that they assume if nothing is specified. For example, the `block` class might have properties `e1`, `e2`, and `e3` that specify the directions of the block edges, but which default to the coordinate axes if they are not specified. Typically, each class will have some properties that have defaults, and some that you are required to specify.

Property values can be any of the primitive types mentioned earlier, but they can also be other objects. For example, instead of specifying a dielectric constant, you might instead supply an object describing the material type:

```scm
(define Si (make material-type (epsilon 11.56)))
(define SiO2 (make material-type (epsilon 2.1)))
(set! geometry
   (list
      (make sphere (material Si) (center 0 0 1) (radius 0.3))
      (make block (material SiO2) (center 0 0 1) (size 1 3.5 2))))
```

We have snuck in another feature here: `(define new-variable value)` is a way of defining new variables for our own use in the control file. This and other features of the Scheme language are discussed in the next section.

What Do I Enter?
----------------

Every program will have a different set of variables that it expects you to set, and a different set of classes with different properties. Whatever program you are using should come with documentation saying what it expects.

You can also get the program to print out help by inserting the `(help)` command in your ctl file, or by entering it in [interactive mode](Advanced_User_Experience/#interactive-mode). You can also simply enter the following command in your shell:

`echo "(help)" | program`

For example, the output of `(help)` in the electromagnetic simulation we have been using in our examples might look like:

```
Class block:
    Class geometric-object:
        material-type material
        vector3 center
    vector3 e1 = #(1 0 0)
    vector3 e2 = #(0 1 0)
    vector3 e3 = #(0 0 1)
    vector3 size
Class sphere:
    Class geometric-object:
        material-type material
        vector3 center
    number radius
Class geometric-object:
    material-type material
    vector3 center
Class material-type:
    number epsilon
    number conductivity = 0.0
```

```
Input variables:
vector3 list k-points = ()
geometric-object list geometry = ()
integer dimensions = 3
```

```
Output variables:
number list gaps = ()
number mean-dielectric = 0.0
```

As can be seen from above, the help output lists all of the classes and their properties, along with the input and output variables (the latter will be described later). Any default values for properties are also given. Along with each variable or property is given its type.

You should also notice that the class `geometric-object` is listed as a part of the classes `block` and `sphere`. These two classes are **subclasses** of `geometric-object`. A subclass inherits the property list of its superclass and can be used any place its superclass is allowed. So, for example, both spheres and blocks can be used in the `geometry` list, which is formally a list of geometric-objects. The astute reader will notice the object-oriented-programming origins of our class concept; our classes, however, differ from OOP in that they have no methods.
