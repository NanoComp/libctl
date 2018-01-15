Scientific software for performing large computations is typically managed using textual control files that specify the parameters of the computation. Historically, these control files have typically consisted of long, inflexible collections of numbers whose meaning and format is hard-coded into the program. With **libctl**, we make it easy for programmers to support a greatly superior control file structure, and with less effort than was required for traditional input formats.

The "ctl" in "libctl" stands for *Control Language*. By convention, libctl control files end with ".ctl" and are referred to as ctl files. Thus, libctl is the *Control Language Library* where the "lib" prefix follows the Unix idiom.

Design Principles
-----------------

The libctl design has the following goals:

-   **Input readability** The control file should be self-annotating and human-readable (as opposed to an inscrutable sequence of numbers). Of course, it should allow comments.
-   **Input flexibility**: The control file should not be sensitive to the ordering or spacing of the inputs.
-   **Input intelligence** The user should never have to enter any information that the program could reasonably infer. For example, reasonable defaults should be available wherever possible for unspecified parameters.
-   **Program flexibility**: It should be easy to add new parameters and features to the control file without breaking older control files or increasing complexity.
-   **Scriptability** Simple things should be simple, but complex things should be possible. The control file should be more than just a file format. It must be a programming language, able to script the computation and add new functionality without modifying the simulation source code.
-   **Programmer convenience**: All of this power should not come at the expense of the programmer. Rather, it should be easier to program than ever before &mdash; the programmer need only specify the interaction with the control file in an abstract form, and everything else should be taken care of automatically.

All of these goals are achieved by libctl with the help of [Guile](https://en.wikipedia.org/wiki/GNU_Guile), the [GNU](https://en.wikipedia.org/wiki/GNU) scripting and extensibility language. Guile does all of the hard work for us, and allows us to embed a complete interpreter in a program with minimal effort.

Despite its power, libctl is designed to be easy to use. A basic user only sees a convenient file format with a programming language to back it up if needs become more complex. For the programmer, all headaches associated with reading input files are lifted &mdash; once an abstract specification is supplied, all interaction with the user is handled automatically.

In the subsequent sections of this manual, we will discuss in more detail the interaction of the user and the programmer with libctl.
