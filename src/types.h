#ifndef TYPES_H
#define TYPES_H

// define interpolation types
enum interpolation_t{ LINEAR, CUBIC, SPLINE, SINC };

// define center waveform shapes
enum waveshape_t { FLAT, SINE, SQUARE, TRIANGLE, SAWTOOTH };

// define data types
typedef float gendydur_t;
typedef float gendyamp_t;
typedef float gendysamp_t;

#endif /* TYPES_H */
