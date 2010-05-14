/*********************************************
 *
 * libgendy
 *
 * a library implementing Iannis Xenakis's Dynamic Stochastic Synthesis
 *
 * Copyright 2009,2010 Spencer Russell
 * Released under the GPLv3
 *
 * This file is part of libgendy.
 *
 * libgendy is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * libgendy is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * libgendy.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 ********************************************/




#include "util.h"
#include <math.h>
#include <cstdlib>

// return a uniformly distributed double-precision float between 0 and 1
double randf() {
	return rand() / (double)RAND_MAX;
}

// returns gaussian random variable with mu 0 and sigma 1
// From the GNU Scientific Library, src/randist/gauss.c

double gauss() {
	double x, y, r2;
	do {
		/* choose x,y in uniform square (-1,-1) to (+1,+1) */
		x = -1 + 2 * randf();
		y = -1 + 2 * randf();

		/* see if it is in the unit circle */
		r2 = x * x + y * y;
	} while (r2 > 1.0 || r2 == 0);

	/* Box-Muller transform */
	return y * sqrt (-2.0 * log(r2) / r2);
}

int round(float num) {
	return int(floor(num + 0.5));
}
