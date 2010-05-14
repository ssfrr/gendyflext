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
