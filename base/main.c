/* libctl: flexible Guile-based control files for scientific software 
 * Copyright (C) 1998-2009, Steven G. Johnson
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

/* for basename and dirname functions */
#include <libgen.h>

#include "ctl-io.h"

#ifdef CXX_CTL_IO
using namespace ctlio;
#endif

/* define a global "verbose" variable set by the --verbose command-line opt. */
int verbose = 0;

/* a "quiet" variable, that if nonzero suppresses all non-error output...
   this is used by parallel software to suppress output from processes
   other than the master process */
int libctl_quiet = 0;

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
	       if (!libctl_quiet) {
		    char *guile_vers;
#ifdef VERSION_STRING
		    /* print version string, if defined: */
		    printf(VERSION_STRING);
#endif
#ifdef LIBCTL_VERSION
		    printf("\nUsing libctl %s", LIBCTL_VERSION);
#else
		    printf("\nUsing libctl");
#endif
		    guile_vers = gh_scm2newstr(gh_eval_str("(version)"), NULL);
		    printf(" and Guile %s.\n", guile_vers);
		    free(guile_vers);
	       }
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
	       if (!libctl_quiet)
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
	       if (!libctl_quiet)
		    fprintf(stderr, "Unknown option %s!  Use the --help option"
			    " for more information.\n", argv[i]);
	       exit(EXIT_FAILURE);
	  }
     }

     return i;
}

/**************************************************************************/

static int exists(const char *fname)
{
     FILE *f = fopen(fname, "r");
     if (f) {
	  fclose(f);
	  return 1;
     }
     return 0;
}

static char *make_name(const char *for_dir, const char *for_base)
{
     char *dir0, *dir, *base0, *base, *name = 0;
     dir0 = (char *) malloc(sizeof(char) * (strlen(for_dir) + 1));
     base0 = (char *) malloc(sizeof(char) * (strlen(for_base) + 1));
     strcpy(dir0, for_dir); dir = dirname(dir0);
     if (strlen(dir)) {
	  strcpy(base0, for_base); base = basename(base0);
	  name = (char *) malloc(sizeof(char) * (strlen(dir) + 1 +
						 strlen(base) + 1));
	  strcpy(name, dir);
	  strcat(name, "/");
	  strcat(name, base);
	  free(base0);
     }
     free(dir0);
     return name;
}

/**************************************************************************/

#ifdef HAVE_CTL_HOOKS
static int ctl_stop_hook_called = 0;

extern void ctl_start_hook(int *argc, char **argv[]);
extern void ctl_stop_hook(void);
#endif

#ifdef HAVE_CTL_EXPORT_HOOK
extern void ctl_export_hook(void);
#endif

extern SCM nlopt_minimize_scm(SCM algorithm_scm,
		       SCM f_scm,
                       SCM lb_scm, SCM ub_scm, SCM x_scm,
		       SCM minf_max_scm, SCM ftol_rel_scm, SCM ftol_abs_scm,
		       SCM rest);


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

  /* Export the subplex minimization routine: */
  gh_new_procedure ("subplex", (SCM (*)(void)) subplex_scm, 7, 0, 0);

#ifdef HAVE_NLOPT
  /* Export the nlopt minimization routine, if available: */
  gh_new_procedure ("nlopt-minimize", (SCM (*)(void)) nlopt_minimize_scm, 
		    8, 0, 1);
#endif

  /* Export the adaptive integration routines: */
  gh_new_procedure ("adaptive-integration", 
		    (SCM (*)(void)) adaptive_integration_scm, 6, 0, 0);

#ifdef CTL_HAS_COMPLEX_INTEGRATION
  gh_new_procedure ("cadaptive-integration", 
		    (SCM (*)(void)) cadaptive_integration_scm, 6, 0, 0);
#endif

#ifdef HAVE_CTL_EXPORT_HOOK
  ctl_export_hook();
#endif

  /* load include.scm if it was given at compile time */
#ifdef INCLUDE_SCM
  gh_load(INCLUDE_SCM);
#endif

  /* load ctl.scm if it was given at compile time */
#ifdef CTL_SCM
  ctl_include(CTL_SCM);
#endif

  i = handle_args(argc, argv, &spec_file_loaded, &continue_run);

  {
       char definestr[] = "(define verbose? false)";
       strcpy(definestr, "(define verbose? ");
       strcat(definestr, verbose ? "true)" : "false)");
       gh_eval_str(definestr);
  }

  if (!continue_run)
       goto done;

  /* load the specification file if it was given at compile time,
     and if it wasn't specified on the command-line: */
#ifdef SPEC_SCM
  if (!spec_file_loaded) {
       /* try first to load it in the program directory if it
	  was specified explicitly (e.g. "./foo"), for cases
	  where we are running a program that has not been installed */
       char *spec_name = make_name(argv[0], SPEC_SCM);
       if (spec_name && exists(spec_name))
	    ctl_include(spec_name);
       else
	    ctl_include(SPEC_SCM);
       free(spec_name);
  }
#endif

  /* define any variables and load any scheme files specified on the
     command line: */
  for (; i < argc; ++i) {
    if (strchr(argv[i],'=')) {
      char *eq;
      char *definestr = (char*) malloc(sizeof(char) * (strlen("(define ") + 
						       strlen(argv[i]) + 2));
      if (!definestr) {
	   fprintf(stderr, __FILE__ ": out of memory!\n");
	   exit(EXIT_FAILURE);
      }
      strcpy(definestr,"(define ");
      strcat(definestr,argv[i]);
      strcat(definestr,")");
      eq = strchr(definestr,'=');
      *eq = ' ';
      if (!libctl_quiet) printf("command-line param: %s\n", argv[i]);
      gh_eval_str(definestr);
      { /* add the name of the defined variable to params-set-list */
	   char *remember_define;
	   strcpy(definestr,argv[i]);
	   eq = strchr(definestr,'=');
	   *eq = 0;
	   remember_define = (char*) malloc(sizeof(char) * (strlen("(set! params-set-list (cons (quote x) params-set-list))") + strlen(definestr)));
	   if (!remember_define) {
		fprintf(stderr, __FILE__ ": out of memory!\n");
		exit(EXIT_FAILURE);
	   }
	   strcpy(remember_define, "(set! params-set-list (cons (quote ");
	   strcat(remember_define, definestr);
	   strcat(remember_define, ") params-set-list))");
	   gh_eval_str(remember_define);
	   free(remember_define);
      }
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

 done:
  ;
#ifdef HAVE_CTL_HOOKS
  /* Note that the stop hook will never be called if we are in
     interactive mode, because gh_repl calls exit().  Oh well. */
  ctl_stop_hook_called = 1;
  ctl_stop_hook();
#endif
}

int main (int argc, char *argv[])
{
#ifdef HAVE_CTL_HOOKS
  ctl_start_hook(&argc, &argv);
#endif
  gh_enter (argc, argv, main_entry);
#ifdef HAVE_CTL_HOOKS
  if (!ctl_stop_hook_called)
       ctl_stop_hook();
#endif
  return EXIT_SUCCESS;
}
