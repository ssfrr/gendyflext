#include "splines.h"

void get_cspline_coefs(double *xp, double *yp, double *coefs) {
	double h[3];
	// h[n] is the x-distance between x[n] and x[n+1]
	for(int i = 0; i < 3; i++)
		h[i] = xp[i+1] - xp[i];
	// d[i] is the slope of the line segment connecting point i
	// to point i+1
	double d[3];
	for(int i = 0; i < 3; i++) 
		d[i] = (yp[i+1] - yp[i]) / h[i];
	//yd[n] is the derivitive of f(x) at xp[1] and xp[2]. It's a
	//weighted average of the two adjacent line segments
	double yd[3];
	for(int i = 1; i < 3; i++) 
		yd[i] = (d[i] * h[i-1] + d[i-1] * h[i]) / (h[i-1] + h[i]);
	// Here we actually calculate the coefficients
	coefs[0] = (h[1]*(yd[2] - yd[1]) - 
			2 * (yp[2] - yp[1] - h[1] * yd[1])) / (h[1] * h[1] * h[1]);
	coefs[1] = (3 * (yp[2] - yp[1] - h[1] * yd[1]) - 
			h[1] * (yd[2] - yd[1])) / (h[1] * h[1]);
	coefs[2] = yd[1];
	coefs[3] = yp[1];
}

double cspline_interp(double *coefs, double x) {
	return coefs[3] + x * (coefs[2] + x * (coefs[1] + coefs[0] * x));
}
