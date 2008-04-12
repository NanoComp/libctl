/* wrapper around NLopt nonlinear optimization library (if installed) */

#ifdef HAVE_NLOPT

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <ctl.h>
#include <nlopt.h>

static double f_scm_wrap(integer n, const double *x, double *grad, void *f_scm_p)
{
     SCM *f_scm = (SCM *) f_scm_p;
     SCM ret = gh_call1(*f_scm, make_number_list(n, x));
     if (scm_real_p(ret))
	  return scm_to_double(ret);
     else { /* otherwise must be a list of value, gradient components,
	       i.e. (cons value gradient). */
	  SCM gscm = ret;
	  int i;
	  for (i = 0; i < n; ++i) {
	       gscm = SCM_CDR(gscm);
	       grad[i] = scm_to_double(SCM_CAR(gscm));
	  }
	  return scm_to_double(SCM_CAR(ret));
     }
}

/* Scheme-callable wrapper for nlopt_minimize() function. */
SCM nlopt_minimize_scm(SCM algorithm_scm,
		       SCM f_scm,
		       SCM lb_scm, SCM ub_scm, SCM x_scm,
		       SCM minf_max_scm, SCM ftol_rel_scm, SCM ftol_abs_scm,
		       SCM xtol_rel_scm, SCM xtol_abs_scm,
		       SCM maxeval_scm, SCM maxtime_scm)
{
     nlopt_algorithm algorithm = (nlopt_algorithm) scm_to_int(algorithm_scm);
     int i, n = list_length(x_scm);
     double *x, *lb, *ub, *xtol_abs = 0;
     double minf_max = scm_to_double(minf_max_scm);
     double ftol_rel = scm_to_double(ftol_rel_scm);
     double ftol_abs = scm_to_double(ftol_abs_scm);
     double xtol_rel = scm_to_double(xtol_rel_scm);
     int maxeal = scm_to_int(maxeval_scm);
     double maxtime = scm_to_double(maxtime_scm);
     double minf;
     nlopt_result result;
     SCM v, ret;

     x = (double *) malloc(sizeof(double) * n * 4);
     lb = x + n; ub = lb + n;
     if (!x) {
	  fprintf(stderr, "nlopt_minimize_scm: out of memory!\n");
	  exit(EXIT_FAILURE);
     }
     if (list_length(lb_scm) != n || list_length(ub_scm) != n
	 || (list_length(xtol_abs_scm) != n && !list_length(xtol_abs_scm))) {
	  fprintf(stderr, "nlopt_minimize_scm: invalid arguments\n");
	  exit(EXIT_FAILURE);
     }
	  
     for (v=x_scm, i=0; i < n; ++i) {
	  x[i] = scm_to_double(SCM_CAR(v));
	  v = SCM_CDR(v);
     }
     for (v=lb_scm, i=0; i < n; ++i) {
	  lb[i] = scm_to_double(SCM_CAR(v));
	  v = SCM_CDR(v);
     }
     for (v=ub_scm, i=0; i < n; ++i) {
	  ub[i] = scm_to_double(SCM_CAR(v));
	  v = SCM_CDR(v);
     }
     if (list_length(xtol_abs_scm)) {
	  xtol_abs = ub + n;
	  for (v=xtol_abs_scm, i=0; i < n; ++i) {
	       xtol_abs[i] = scm_to_double(SCM_CAR(v));
	       v = SCM_CDR(v);
	  }
     }

     result = nlopt_minimize(algorithm, n, f_scm_wrap, &f_scm,
			     lb, ub, x, &minf,
			     minf_max, ftol_rel, ftol_abs, xtol_rel, xtol_abs,
			     maxeal, maxtime);

     ret = scm_cons(scm_from_int((int) result),
		    scm_cons(scm_from_double(minf), make_number_list(n, x)));

     free(x);

     return ret;
}

#endif /* HAVE_NLOPT */
