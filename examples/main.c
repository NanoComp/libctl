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

/* Keep a count of the calls to run_program.  If it is never called,
   then drop into interactive mode. */
static int count_runs = 0;

/**************************************************************************/

/* Run your program (i.e. run a simulation).  This may be called
   multiple times by the control script!

   Return true (SCM_BOOL_T) if successful, and
   false (SCM_BOOL_F) if an error is encountered.

   (To keep main.c clean, I recommend putting the bulk of your simulation
   in a routine in some other file, and just calling that from here.) */
SCM run_program(void)
{
  ++count_runs;

  printf("Run #%d.\nThis routine would normally run a simulation.\n",
	 count_runs);

  return SCM_BOOL_T; /* return true if successful */
}

/* Declare any functions that you want to be callable from a contrl script.
   By default, only run_program is declared.

   By convention, we convert underscores to hyphens in identifiers.

   (See gh_new_procedure in the Guile reference for more information.) */
void declare_functions(void)
{
  gh_new_procedure("run-program", run_program,0,0,0);
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

  /* if run-program was never called, drop into interactive mode: */
  if (count_runs == 0)
    gh_repl(argc, argv);
}

int main (int argc, char *argv[])
{
  gh_enter (argc, argv, main_entry);
  return EXIT_SUCCESS;
}
