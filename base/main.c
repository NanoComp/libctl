/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998, 1999 Steven G. Johnson
 *
 * This file may be used without restriction.  It is in the public
 * domain, and is NOT restricted by the terms of any GNU license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * Steven G. Johnson can be contacted at stevenj@alum.mit.edu.
 */

/**************************************************************************/

/* main program for a simulation that uses libctl.

   You should not need to modify this file.

   The resulting program will run any Scheme files that are passed
   as parameters on the command line.  It will automatically load
   ctl.scm and the specification file if the CTL_SCM and SPEC_SCM
   preprocessor symbols are defined to the corresponding filenames
   (e.g. see the accompanying Makefile).
*/

/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <guile/gh.h>

#include <ctl-io.h>

/* define a global "verbose" variable set by the --verbose command-line opt. */
int verbose = 0;

/**************************************************************************/

/* Handle command-line args, returning first arg not handled.
   Also return, in spec_file_loaded, whether we have loaded
   the specifications file due to a command-line arg.  Also return,
   in continue_run, whether or not to continue the run.  */
int handle_args(int argc, char *argv[], 
		boolean *spec_file_loaded, boolean *continue_run)
{
     int i;

     *continue_run = 1;
     *spec_file_loaded = 0;

     for (i = 1; i < argc; ++i) {
	  if (argv[i][0] != '-')
	       break;
	  if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-V")) {
	       char *guile_vers;
#ifdef VERSION_STRING
	       /* print version string, if defined: */
	       printf(VERSION_STRING);
#endif
	       guile_vers = gh_scm2newstr(gh_eval_str("(version)"), NULL);
	       printf("\nUsing libctl and Guile %s.\n", guile_vers);
	       free(guile_vers);
	       *continue_run = 0;
	  }
	  else if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v"))
	       verbose = 1;
	  else if (!strncmp(argv[i], "--spec-file=", strlen("--spec-file="))) {
	       ctl_include(argv[i] + strlen("--spec-file="));
	       *spec_file_loaded = 1;
	  }
	  else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
	       char *slash = strrchr(argv[0], '/');
	       printf("Usage: %s [options] [definitions] [ctl files]\n"
		      "options:\n"
		      "             --help, -h: this help\n"
		      "          --version, -V: display version information\n"
		      "          --verbose, -v: enable verbose output\n"
		      "     --spec-file=<file>: use <file> for spec. file\n"
		      "definitions: assignments of the form "
		      "<variable>=<value>\n"
		      "ctl files: zero or more Scheme/ctl files to execute\n",
		      slash ? slash + 1 : argv[0]);
	       *continue_run = 0;
	  }
	  else {
	       fprintf(stderr, "Unknown option %s!  Use the --help option"
		       " for more information.\n", argv[i]);
	       exit(EXIT_FAILURE);
	  }
     }

     return i;
}

/**************************************************************************/

/* Main program.  Start up Guile, declare functions, load any
   scripts passed on the command-line, and drop into interactive
   mode if read-input-vars was never called. */

void main_entry(int argc, char *argv[])
{
  int i;
  boolean spec_file_loaded, continue_run;
  SCM interactive;

  /* Notify Guile of functions that we are making callable from Scheme.
     These are defined in the specifications file, from which the
     export_external_functions routine is automatically generated. */
  export_external_functions();

  /* Also export the read_input_vars and write_output_vars routines
     that are automatically generated from the specifications file: */
  gh_new_procedure ("read-input-vars", read_input_vars, 0, 0, 0);
  gh_new_procedure ("write-output-vars", write_output_vars, 0, 0, 0);

  /* load include.scm if it was given at compile time */
#ifdef INCLUDE_SCM
  ctl_include(INCLUDE_SCM);
#endif

  /* load ctl.scm if it was given at compile time */
#ifdef CTL_SCM
  ctl_include(CTL_SCM);
#endif

  i = handle_args(argc, argv, &spec_file_loaded, &continue_run);

  if (!continue_run)
       return;

  /* load the specification file if it was given at compile time,
     and if it wasn't specified on the command-line: */
#ifdef SPEC_SCM
  if (!spec_file_loaded)
       ctl_include(SPEC_SCM);
#endif

  /* define any variables and load any scheme files specified on the
     command line: */
  for (; i < argc; ++i) {
    if (strchr(argv[i],'=')) {
      char *eq;
      char *definestr = malloc(strlen("(define ") + strlen(argv[i]) + 2);
      strcpy(definestr,"(define ");
      strcat(definestr,argv[i]);
      strcat(definestr,")");
      eq = strchr(definestr,'=');
      *eq = ' ';
      gh_eval_str(definestr);
      free(definestr);
      argv[i][0] = 0;
    }
    else if (argv[i][0])
      ctl_include(argv[i]);
  }

  /* Check if we should run an interactive prompt.  We do this if
     either the Scheme variable "interactive?" is true, or if it is not
     defined. */

  interactive = gh_lookup("interactive?");
  if (interactive != SCM_BOOL_F)
       gh_repl(argc - i, argv + i); /* skip already-handled args */
}

int main (int argc, char *argv[])
{
  gh_enter (argc, argv, main_entry);
  return EXIT_SUCCESS;
}
