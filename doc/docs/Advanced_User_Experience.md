---
# Advanced User Experience
---

Many more things can be accomplished in a control file besides simply specifying the parameters of a computation, and even that can be done in a more sophisticated way than we have already described. The key to this functionality is the fact that the ctl file is actually written in a full programming language, called Scheme. This language is interpreted and executed at run-time using an interpreter named Guile. The fact that it is a full programming language means that you can do practically anything &mdash; the only limitations are in the degree of interaction supported by the simulation program.

In a [later section](Guile_and_Scheme_links.md), we provide links to more information on Scheme and Guile.

Interactive Mode
----------------

The easiest way to learn Scheme is to experiment. Guile supports an interactive mode where you can type in commands and have them executed immediately. To get into this mode, you can just type `guile` at the command-line.

If you run your libctl program without passing any arguments, or pass a ctl file that never invokes `(run)`, this will also drop you into a Guile interactive mode. What's more, all the special features supported by libctl and your program are available from this interactive mode. So, you can set parameters of your program, invoke it with `(run)`, get help with `(help)`, and do anything else you might otherwise do in a ctl file. It is possible that your program supports other calls than just `(run)`, in which case you could control it on an even more detailed level.

There is a boolean variable called `interactive?` that controls whether interactive mode will be entered. This variable is `true` initially, but is typically set to `false` by `(run)`. You can force interactive mode to be entered or not by `set!`-ing this variable to `true` or `false`, respectively.

Command-Line Parameters
-----------------------

It is often useful to be able to set parameters of your ctl file from the command-line when you run the program. For example, you might want to vary the radius of some object with each run. To do this, you would define a parameter `R` in your ctl file:

```
(define-param R 0.2)
```

You would then use `R` instead of a numeric value whenever you wanted this radius. If nothing is specified on the command-line, `R` will take on a default value of 0.2. However, you can change the value of `R` on a particular run by specifying `R=value` on the command-line. For instance, to set `R` to 0.3, you would use:

`program R=0.3 ctl-file`

You can have as many command-line parameters as you want. In fact, all of the predefined input variables for a program are defined via `define-param` already, so you can set them via the command line too.

To change the parameter once it is defined, but to still allow it to be overridden from the command line, you can use

```
(set-param! R 0.5)
```

where the above command line would change the value of `R` to 0.3. If you want to change the parameter to a new value *regardless* of what appears on the command line, you can just use `set!`:

```
(set! R 1.3)
```

Note that the predefined input variables for a typical libctl-using program are all created via `define-param`, so they can be overridden using `set-param!`.

Programmatic Parameter Control
------------------------------

A simple use of the programmatic features of Scheme is to give you more power in assigning the variables in the control file. You can use arithmetic expressions, loops and functions, or define your own variables and functions.

For example, consider the following case where we set the `k-points` of a band-structure computation such as [MPB](https://mpb.readthedocs.io/). We define the corners of the Brillouin zone, and then call a libctl-provided function, `interpolate`, to linearly interpolate between them.

```scm
(define Gamma-point (vector3 0 0))
(define X-point (vector3 0.5 0))
(define M-point (vector3 0.5 0.5))
(set! k-points (list Gamma-point X-point M-point Gamma-point))
(set! k-points (interpolate 4 k-points))
```

The resulting list has 4 points interpolated between each pair of corners:

`> (0,0,0) (0.1,0,0) (0.2,0,0) (0.3,0,0) (0.4,0,0) (0.5,0,0) (0.5,0.1,0) (0.5,0.2,0) (0.5,0.3,0)
(0.5,0.4,0) (0.5,0.5,0) (0.4,0.4,0) (0.3,0.3,0) (0.2,0.2,0) (0.1,0.1,0) (0,0,0)`

The `interpolate` function is provided as a convenience by libctl, but you could have written it yourself if it weren't. With past programs, it has often been necessary to write a program to generate control files &mdash; now, the program can be in the control file itself.

Interacting with the Simulation
-------------------------------

So far, the communication with the simulation program has been one-way, with us passing information to the simulation. It is possible, however, to get information back. The `(help)` command lists not only input variables, but also *output* variables &mdash; these variables are set by the simulation and are available for the ctl program to examine after `(run)` returns.

For example, a band-structure computation might return a list of the band-gaps. Using this, the ctl file could vary, say, the radius of a sphere and loop until a band-gap is maximized.