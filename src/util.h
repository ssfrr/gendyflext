#ifndef UTIL_H
#define UTIL_H

// misc utility functions

// return a uniformly distributed double-precision float between 0 and 1
double randf();

// returns gaussian random variable with mu 0 and sigma 1
// From the GNU Scientific Library, src/randist/gauss.c, released under GPL
double gauss();

// returns nearest integer. X.5 always rounded to X+1, so it's non-symmetrical
int round(float num);

#endif /* UTIL_H */
