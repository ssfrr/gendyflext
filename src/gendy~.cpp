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




// gendy~ - an implementation of Iannis Xenakis' Dynamic
// Stochastic Synthesis as an object for Pure Data.
//
// This project uses Thomas Grill's Flext framework, which
// is necessary to compile and run it.
//

#include <math.h>
#include <flext.h>
#include "gendy~.h"
#include "log.h"

using namespace std;

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 502)
#error You need at least flext version 0.5.2
#endif


// object class constructor(run at each gendy object creation)
gendy::gendy() {
	id = gendy_count;
	gendy_count++;
	if(debug)
		print_log("gendy~ #%d: Constructor initiated", id, LOG_DEBUG);
	AddInAnything("control input");	// control input
	AddOutSignal("audio out");		  // audio output

	display_buf = NULL;

	if(debug)
		print_log("gendy~ #%d: Constructor terminated", id, LOG_DEBUG);
}

gendy::~gendy() {
	if(debug)
		print_log("gendy~ #%d: Destructor initiated", id, LOG_DEBUG);
	gendy_count--;
	if(debug)
		print_log("gendy~ #%d: Destructor terminated", id, LOG_DEBUG);
}

void gendy::class_setup(t_classid thisclass) {
	// associate methods with incoming messages on inlet 0
	print_log("Class constructor beginning", LOG_DEBUG);
	FLEXT_CADDMETHOD_(thisclass, 0, "freq", set_frequency);
	FLEXT_CADDMETHOD_(thisclass, 0, "breakpoints", set_num_breakpoints);
	FLEXT_CADDMETHOD_(thisclass, 0, "h_step", set_h_step);
	FLEXT_CADDMETHOD_(thisclass, 0, "v_step", set_v_step);
	FLEXT_CADDMETHOD_(thisclass, 0, "h_pull", set_h_pull);
	FLEXT_CADDMETHOD_(thisclass, 0, "v_pull", set_v_pull);
	FLEXT_CADDMETHOD_(thisclass, 0, "linear", set_interpolation_lin);
	FLEXT_CADDMETHOD_(thisclass, 0, "cubic", set_interpolation_cubic);
	FLEXT_CADDMETHOD_(thisclass, 0, "spline", set_interpolation_spline);
	FLEXT_CADDMETHOD_(thisclass, 0, "sinc", set_interpolation_sinc);
	FLEXT_CADDMETHOD_(thisclass, 0, "flat", set_waveform_flat);
	FLEXT_CADDMETHOD_(thisclass, 0, "sine", set_waveform_sine);
	FLEXT_CADDMETHOD_(thisclass, 0, "square", set_waveform_square);
	FLEXT_CADDMETHOD_(thisclass, 0, "debug", set_debug);
	FLEXT_CADDMETHOD_(thisclass, 0, "table", set_outbuf);
	FLEXT_CADDMETHOD_(thisclass, 0, "redraw", redraw);
	print_log("",LOG_INFO);
	print_log("-- gendy~ v%d.%d.%d by Spencer Russell --",
			GENDY_MAJ, GENDY_MIN, GENDY_REV, LOG_INFO);
	print_log("gendy~: please report bugs to http://code.airwav.es/projects/gendy/issues", LOG_INFO);
	print_log("Class constructor ending", LOG_DEBUG);
}

// Now we define our DSP function. It gets these arguments:
//
// n: length of signal vector.
// float *const *in, float *const *out:
//  These are arrays of signal vectors(in is a pointer to const pointer to float)

void gendy::m_signal(int n, float *const *in, float *const *out) {
	waveform.get_block(out[0], n);
}

// Message handling functions

void gendy::set_frequency(float new_freq) {
	print_log("set_frequency(%f)", new_freq, LOG_DEBUG);
	waveform.set_avg_wavelength(Samplerate() / new_freq);
}

void gendy::set_num_breakpoints(float num_breakpoints) {
	print_log("set_num_breakpoints(%f)", num_breakpoints, LOG_DEBUG);
	waveform.set_num_breakpoints(num_breakpoints);
}

void gendy::set_h_step(float new_stepsize) {
	print_log("set_h_step(%f)", new_stepsize, LOG_DEBUG);
	waveform.set_step_width(new_stepsize);
}

void gendy::set_v_step(float new_stepsize) {
	print_log("set_v_step(%f)", new_stepsize, LOG_DEBUG);
	waveform.set_step_height(new_stepsize);
}

void gendy::set_h_pull(float new_pull) {
	print_log("set_h_pull(%f)", new_pull, LOG_DEBUG);
	waveform.set_duration_pull(new_pull);
}

void gendy::set_v_pull(float new_pull) {
	print_log("set_v_pull(%f)", new_pull, LOG_DEBUG);
	waveform.set_amplitude_pull(new_pull);
}

void gendy::set_interpolation_lin() {
	print_log("set_interpolation_lin()", LOG_DEBUG);
	set_interpolation(LINEAR);
}

void gendy::set_interpolation_cubic() {
	print_log("set_interpolation_cubic()", LOG_DEBUG);
	set_interpolation(CUBIC);
}

void gendy::set_interpolation_spline() {
	print_log("set_interpolation_spline()", LOG_DEBUG);
	set_interpolation(SPLINE);
}

void gendy::set_interpolation_sinc() {
	print_log("set_interpolation_sinc()", LOG_DEBUG);
	set_interpolation(SINC);
}

void gendy::set_waveform_flat() {
	print_log("set_waveform_flat()", LOG_DEBUG);
	set_waveform(FLAT);
}

void gendy::set_waveform_sine() {
	print_log("set_waveform_sine()", LOG_DEBUG);
	set_waveform(SINE);
}

void gendy::set_waveform_square() {
	print_log("set_waveform_square()", LOG_DEBUG);
	set_waveform(SQUARE);
}

void gendy::set_debug(int new_debug) {
	print_log("set_debug(%d)", new_debug, LOG_DEBUG);
	if(new_debug)
		debug = true;
	else
		debug = false;
}

// TODO let flext take care of buffer setting argument checking?

void gendy::set_outbuf(short argc, t_atom *argv) {
	if(argc == 0)
		// no argument toggles waveform display
		print_log("gendy~: missing buffer name", LOG_ERROR);
	else if(argc == 1) {
		if(IsFloat(argv[0]))
			print_log("gendy~: invalid buffer name", LOG_ERROR);
		else if(IsSymbol(argv[0])) {
			// symbol argument, set output buffer
			// TODO: better error reporting
			// delete existing buffer reference
			if(display_buf) {
				delete display_buf;
			}
			display_buf = new buffer(GetSymbol(argv[0]));
			if(!display_buf->Ok()) {
				print_log("gendy~: buffer not valid", LOG_ERROR);
				delete display_buf;
				display_buf = NULL;
			}
		}
	}
}

// private class methods
void gendy::set_interpolation(interpolation_t new_interpolation) {
	waveform.set_interpolation(new_interpolation);
}

void gendy::set_waveform(waveshape_t new_waveform) {
	waveform.set_waveshape(new_waveform);
}

void gendy::redraw() {
	int n = 0;
	gendysamp_t *temp_buf;

	if(!display_buf || !display_buf->Ok()) {
		print_log("gendy~: Invalid Buffer", LOG_ERROR);
		return;
	}

	//TODO: is there a way to do this without the temp buffer?
	flext::buffer::lock_t state = display_buf->Lock();
	display_buf->Update();
	// resize table to fit 1 wavelength
	//display_buf->Frames(current_wavelength, false, false);
	int bufsize = display_buf->Frames();
	temp_buf = new gendysamp_t[bufsize];
	//TODO: this is not threadsafe. wavelength could change.
	int wavelength = waveform.get_wavelength();
	waveform.get_cycle(temp_buf, bufsize);
	// here we copy from the raw float array to the flext buffer object
	for(; n < wavelength && n < bufsize; ++n)
		(*display_buf)[n] = temp_buf[n];
	delete temp_buf;
	// zero out the rest of the buffer
	while(n < bufsize)
		(*display_buf)[n++]= 0;
	display_buf->Dirty(true);
	display_buf->Unlock(state);
}

//register the gendy class as a PD or Max object
FLEXT_NEW_DSP("gendy~", gendy)
