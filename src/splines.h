#ifndef SPLINES_H
#define SPLINES_H

void get_cspline_coefs(double *xp, double *yp, double *coefs);
double cspline_interp(double *coefs, double x);

#endif /* SPLINES_H */
