Welcome to the manual for **libctl**, a Guile-based library implementing flexible control files for scientific simulations! This documentation is divided into the following sections, which you should read roughly in order if you are new to libctl:

-   [Introduction](/Libctl_Introduction) — The introductory section describes the motivation, history, and high-level structure of libctl.
-   [Basic User Experience](/Libctl_Basic_User_Experience) — Here, we disuss what a basic **ctl** control file looks like. From this perspective, ctl is just another control-file format with lots of parentheses.
-   [Advanced User Experience](/Libctl_Advanced_User_Experience) — The advanced user can take advantage of the fact that the ctl file is actually a Scheme program running in a full interpreter (called Guile). Literally anything is possible, especially since the simulation program can support dynamic passing of information back and forth with the control file.  -   [User Reference](/libctl_User_Reference ) — A compact listing of the various functions provided for the user by libctl.
-   [Developer Experience](/Libctl_Developer_Experience) — libctl is powerful for the developer, too. One merely specifies an abstract **specification file** that describes the information that is exchanged with the ctl file, and nearly everything else is automatic.
-   [Guile and Scheme Information](/Guile_and_Scheme_links) — Guile is a standard GNU program for adding scripting and extensibility to software. It implements an embeddable interpreter for the Scheme language. There are many places that you can go to learn more about Guile and Scheme, and we link to a few of them here.
-   [License and Copyright](/Libctl_License_and_Copyright) — libctl is free software under the [GNU General Public License](http://www.gnu.org/copyleft/gpl.html) (GNU LGPL).
-   [Release notes](/Libctl_release_notes) — libctl is free software under the [GNU General Public License](http://www.gnu.org/copyleft/gpl.html) (GNU LGPL).

Feedback
--------

If you have comments or questions regarding libctl, you can contact [Steven G. Johnson](http://math.mit.edu/~stevenj) at <stevenj@alum.mit.edu>.
