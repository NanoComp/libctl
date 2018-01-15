In this section, we list all of the functions provided for users by libctl. We do *not* attempt to document standard Scheme functions, with a couple of exceptions below, since there are plenty of good Scheme references.

Of course, the most important function is:

```
(help)
```

Outputs a listing of all the available classes, their properties, default values, and types. Also lists the input and output variables.

Remember, Guile lets you enter expressions and see their values interactively. This is the best way to learn how to use anything that confuses you &mdash; just try it and see how it works.

Basic Scheme Functions
----------------------

```
(set! variable value)
```

Change the value of `variable` to `value`.

```
(define variable value)
```

Define new `variable` with initial `value`.

```
(list [element1 element2 ...])
```

Returns a list consisting of zero or more elements.

```
(append [list1 list2 ...])
```

Concatenates zero or more lists into a single list.

```
(function [arg1 arg2 ...])
```

This is how you call a Scheme `function` in general.

```
(define (function [arg1 arg2 ...]) body)
```

Define a new `function` with zero or more arguments that returns the result of given `body` when it is invoked.

Command-Line Parameters
-----------------------

```
(define-param name default-value)
```

Define a variable `name` whose value can be set from the command line, and which assumes a value `default-value` if it is not set. To set the value on the command-line, include `name=value` on the command-line when the program is executed. In all other respects, `name` is an ordinary Scheme variable.

```
(set-param! name new-default-value)
```

Like `set!`, but does nothing if `name` was set on the command line.

All libctl arguments accept the command-line parameter `--verbose` to turn on a verbose mode. This sets the variable `verbose?` to `true` (and, depending on the program, may enable other outputs).

Complex Numbers
---------------

Scheme includes full support for complex numbers and arithmetic; all of the ordinary operations (`+`, `*`, `sqrt`, etcetera) just work. For the same reason, you can freely use complex numbers in libctl's vector and matrix functions, below.

To specify a complex number *a*+*b*i, you simply use the syntax `a+bi` if *a* and *b* are constants, and `(make-rectangular a b)` otherwise. You can also specify numbers in "polar" format a\*e<sup><small>ib</small></sup> by the syntax `a@b` or `(make-polar a b)`.

There are a few special functions provided by Scheme to manipulate complex numbers. `(real-part z)` and `(imag-part z)` return the real and imaginary parts of `z`, respectively. `(magnitude z)` returns the absolute value and `(angle z)` returns the phase angle. libctl also provides a `(conj z)` function, below, to return the complex conjugate.

3-Vector Functions
------------------

```
(vector3 x [y z])
```

Create a new 3-vector with the given components. If the `y` or `z` value is omitted, it is set to zero.

```
(vector3-x v)
(vector3-y v)
(vector3-z v)
```

Return the corresponding component of the vector `v`.

```
(vector3+ v1 v2)
(vector3- v1 v2)
(vector3-cross v1 v2)
```

Return the sum, difference, or cross product of the two vectors.

```
(vector3* a b)
```

If `a` and `b` are both vectors, returns their dot product. If one of them is a number and the other is a vector, then scales the vector by the number.

```
(vector3-dot v1 v2)
```

Returns the dot product of `v1` and `v2`.

```
(vector3-cross v1 v2)
```

Returns the cross product of `v1` and `v2`.

```
(vector3-cdot v1 v2)
```

Returns the conjugated dot product: *v1*\* dot *v2*.

```
(vector3-norm v)
```

Returns the length `(sqrt (vector3-cdot v v))` of the given vector.

```
(unit-vector3 x [y z])
```
```
(unit-vector3 v)
```

Given a vector or, alternatively, one or more components, returns a unit vector in that direction.

```
(vector3-close? v1 v2 tolerance)
```

Returns whether or not the corresponding components of the two vectors are within `tolerance` of each other.

```
(vector3= v1 v2)
```

Returns whether or not the two vectors are numerically equal. Beware of using this function after operations that may have some error due to the finite precision of floating-point numbers; use `vector3-close?` instead.

```
(rotate-vector3 axis theta v)
```

Returns the vector `v` rotated by an angle `theta` (in radians) in the right-hand direction around the `axis` vector (whose length is ignored). You may find the functions `(deg->rad theta-deg)` and `(rad->deg theta-rad)` useful to convert angles between degrees and radians.

```
(vector3->exact v)
```

Round a vector3 *v* to the nearest "exact" representation in Scheme: an integer or a rational number. This is mainly useful if you have an all-integer vector and you want to force Guile to treat it as integers rather than floating-point numbers.

3x3 Matrix Functions
--------------------

```
(matrix3x3 c1 c2 c3)
```

Creates a 3x3 matrix with the given 3-vectors as its columns.

```
(matrix3x3-transpose m)
(matrix3x3-adjoint m)
(matrix3x3-determinant m)
(matrix3x3-inverse m)
```

Return the transpose, adjoint (conjugate transpose), determinant, or inverse of the given matrix.

```
(matrix3x3+ m1 m2)
(matrix3x3- m1 m2)
(matrix3x3* m1 m2)
```

Return the sum, difference, or product of the given matrices.

```
(matrix3x3* v m)
(matrix3x3* m v)
```

Returns the (3-vector) product of the matrix `m` by the vector `v`, with the vector multiplied on the left or the right respectively.

```
(matrix3x3* s m)
(matrix3x3* m s)
```

Scales the matrix `m` by the number `s`.

```
(rotation-matrix3x3 axis theta)
```

Like `rotate-vector3`, except returns the (unitary) rotation matrix that performs the given rotation. i.e., `(matrix3x3* (rotation-matrix3x3 axis theta) v)` produces the same result as `(rotate-vector3 axis theta v)`.

Objects (Members of Classes)
----------------------------

```
(make class [ properties ... ])
```

Make an object of the given `class`. Each property is of the form `(property-name property-value)`. A property need not be specified if it has a default value, and properties may be given in any order.

```
(object-property-value object property-name)
```

Return the value of the property whose name (symbol) is `property-name` in `object`. For example, `(object-property-value a-circle-object 'radius)`. Returns `false` if `property-name` is not a property of `object`.

Miscellaneous Utilities
-----------------------

```
(conj x)
```

Return the complex conjugate of a number `x` (for some reason, Scheme doesn't provide such a function).

```
(interpolate n list)
```

Given a `list` of numbers or 3-vectors, linearly interpolates between them to add `n` new evenly-spaced values between each pair of consecutive values in the original list.

```
(interpolate-uniform n list)
```

Similar to `interpolate`, but attempts to maintain a (roughly) uniform average spacing of the points in the interpolated list. In this case `n` is only the number of points interpolated *on average* between each pair of values in the original list.

```
(print expressions...)
```

Calls the Scheme `display` function on each of its arguments from left to right (printing them to standard output). Note that, like `display`, it does *not* append a newline to the end of the outputs; you have to do this yourself by including the `"\n"` string at the end of the expression list. In addition, there is a global variable `print-ok?`, defaulting to `true`, that controls whether `print` does anything; by setting `print-ok?` to false, you can disable all output.

```
(begin-time message-string statements...)
```

Like the Scheme `(begin ...)` construct, this executes the given sequence of statements one by one. In addition, however, it measures the elapsed time for the statements and outputs it as `message-string`, followed by the time, followed by a newline. The return value of `begin-time` is the elapsed time in seconds.

```
(minimize function tolerance)
```

Given a `function` of one (number) argument, finds its minimum within the specified fractional `tolerance`. If the return value of `minimize` is assigned to a variable `result`, then `(min-arg result)` and `(min-val result)` give the argument and value of the function at its minimum. If you can, you should use one of the variant forms of `minimize`, described below.

```
(minimize function tolerance guess)
```

The same as above, but you supply an initial `guess` for where the minimum is located.

```
(minimize function tolerance arg-min arg-max)
```

The same as above, but you supply the minimum and maximum function argument values within which to search for the minimum. This is the most preferred form of `minimize`, and is faster and more robust than the other two variants.

```
(minimize-multiple function tolerance arg1 ... argN)
```

Minimize a `function` of N numeric arguments within the specified fractional `tolerance`. `arg1` ... `argN` are an initial guess for the function arguments. Returns both the arguments and value of the function at its minimum. A list of the arguments at the minimum are retrieved via `min-arg`, and the value via `min-val`.

```
maximize, maximize-multiple
```

These are the same as the `minimize` functions except that they maximize the function instead of minimizing it. The functions `max-arg` and `max-val` are provided instead of `min-arg` and `min-val`.

```
(find-root function tolerance arg-min arg-max)
```

Find a root of the given `function` to within the specified fractional `tolerance`. `arg-min` and `arg-max` **bracket** the desired root; the function must have opposite signs at these two points.

```
(find-root-deriv function tolerance arg-min arg-max [arg-guess])
```

As `find-root`, but `function` should return a `cons` pair of (*function-value . function-derivative*); the derivative information is exploited to achieve faster convergence via Newton's method, compared to `find-root`. The optional argument `arg-guess` should be an initial guess for the root location.

```
(derivative function x [dx tolerance])
(deriv function x [dx tolerance])
(derivative2 function x [dx tolerance])
(deriv2 function x [dx tolerance])
```

Compute the numerical derivative of the given `function` at `x` to within *at best* the specified fractional `tolerance` (defaulting to the best achievable tolerance), using Ridder's method of polynomial extrapolation. `dx` should be a *maximum* displacement in `x` for derivative evaluation; the `function` should change by a significant amount (much larger than the numerical precision) over `dx`. `dx` defaults to 1% of `x` or `0.01`, whichever is larger.

If the return value of `derivative` is assigned to a variable `result`, then `(derivative-df result)` and `(derivative-df-err result)` give the derivative of the function and an estimate of the numerical error in the derivative, respectively.

The `derivative2` function computes both the first and second derivatives, using minimal extra function evaluations; the second derivative and its error are then obtained by `(derivative-d2f result)` and `(derivative-d2f-err result)`.

`deriv` and `deriv2` are identical to `derivative` and `derivative2`, except that they directly return the value of the first and second derivatives, respectively (no need to call `derivative-df` or `derivative-d2f`). They don't provide the error estimate, however, or the ability to compute first and second derivatives simulataneously.

There are also modified versions of these functions that compute *one-sided* derivatives: they only evaluate the function at arguments *x* that are ≥0 (or only ≤0). These functions are named `derivative+`, `deriv+` etc. (or `derivative-` etc. for negative arguments) and otherwise behave identically. They are generally less accurate than the two-sided derivative, above, but are useful for functions with discontinuous derivatives.

```
(integrate f a b relerr [ abserr maxeval ])
```

Return the definite integral of the function `f` from `a` to `b`, to within the specified relative error `relerr`, using an adaptive Gaussian quadrature (in 1d) or adaptive cubature (in multiple dimensions). The optional arguments `abserr` and `maxeval` specify an absolute error tolerance (default is zero) and a maximum number of function evaluations (default is no limit). Integration stops when *either* the relative error *or* the absolute error *or* the maximum number of evaluations is met (note that error estimates are only approximate, though).

This function can compute multi-dimensional integrals, in which case `f` is a function of *N* variables and `a` and `b` are either lists or vectors of length *N*, giving the (constant) integration bounds in each dimension. Non-constant integration bounds, i.e. non-rectilinear integration domains, can be handled by an appropriate mapping of the function `f`.

```
(fold-left op init list)
```

Combine the elements of `list` using the binary "operator" function `(op x y)`, with initial value `init`, associating from the left of the list. That is, if `list` consist of the elements `(a b c d)`, then `(fold-left op init list)` computes `(op (op (op (op init a) b) c) d)`. For example, if `list` contains numbers, then `(fold-left + 0 list)` returns the sum of the elements of `list`.

```
(fold-right op init list)
```

As `fold-left`, but associate from the right. For example, `(op a (op b (op c (op d init))))`.

```
(memoize func)
```

Return a function wrapping around the function `func` that "memoizes" its arguments and return values. That is, it returns the same thing as `func`, but if passed the same arguments as a previous call it returns a cached return value from the previous call instead of recomputing it.
