/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998 Steven G. Johnson
 *
 * This file may be used without restriction.  It is in the public
 * domain, and is NOT restricted by the terms of any GNU license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * Steven G. Johnson can be contacted at stevenj@alum.mit.edu.
 */

/**************************************************************************/

/* Sample main program for a simulation that uses libctl.

   To use this in your own program, modify run_program to call a
   routine that runs your simulation.  You can also modify
   declare_functions to make more functions callable from Guile.

   The resulting program will run any Scheme files that are passed
   as parameters on the command line.  The first parameter passed
   should be ctl.scm.
*/

/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <guile/gh.h>

#include "ctl-io.h"

/**************************************************************************/

/* declare this function (in example.c) since I don't feel like
   creating a header file just to declare the example function. */
extern void example_do_stuff(void);

/* Run your program (i.e. run a simulation).  This may be called
   multiple times by the control script!

   Return true (SCM_BOOL_T) if successful, and
   false (SCM_BOOL_F) if an error is encountered.

   (To keep main.c clean, I recommend putting the bulk of your simulation
   in a routine in some other file, and just calling that from here.)

   You should NOT call (run-program) directly from a control script.
   Instead, call (run), which will also read/write the input/output vars.
*/
SCM run_program(void)
{
  example_do_stuff();

  return SCM_BOOL_T; /* return true if successful */
}

/* Declare any functions that you want to be callable from a contrl script.
   By default, only run_program and the functions to read and write
   the input and output vars are declared.

   By convention, we convert underscores to hyphens in identifiers.

   (See gh_new_procedure in the Guile reference for more information.) */
void declare_functions(void)
{
  gh_new_procedure("run-program", run_program,0,0,0);
  gh_new_procedure("read-input-vars", read_input_vars,0,0,0);
  gh_new_procedure("write-output-vars", write_output_vars,0,0,0);
}

/**************************************************************************/

/* Main program.  Start up Guile, declare functions, load any
   scripts passed on the command-line, and drop into interactive
   mode if run-program was never called. */

void main_entry(int argc, char *argv[])
{
  int i;

  declare_functions();

  /* load any scheme files specified on the command-line: */
  for (i = 1; i < argc; ++i)
    gh_eval_file(argv[i]);

  /* if run was never called, drop into interactive mode: */
  /* (the num_runs count is kept by ctl-io in read_input_vars) */
  if (num_runs == 0)
    gh_repl(argc, argv);
}

int main (int argc, char *argv[])
{
  gh_enter (argc, argv, main_entry);
  return EXIT_SUCCESS;
}
