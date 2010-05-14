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

#ifndef GENDY_H
#define GENDY_H
#include "gendy_waveform.h"
//
// gendy~ version 0.6.0:
const int GENDY_MAJ = 0;
const int GENDY_MIN = 6;
const int GENDY_REV = 0;

// A flext dsp external ("tilde object") inherits from the class flext_dsp
class gendy:  public flext_dsp {
	// flext macro magic
	FLEXT_HEADER_S(gendy, flext_dsp, class_setup)

	public:
		gendy();	
		~gendy();
	
	protected:
		// here we declare the virtual DSP function
		virtual void m_signal(int n, float *const *in, float *const *out);

		// Message handling functions
		void set_frequency(float new_freq);
		void set_num_breakpoints(float num_breakpoints);
		void set_h_step(float new_stepsize);
		void set_v_step(float new_stepsize);
		void set_h_pull(float new_pull);
		void set_v_pull(float new_pull);
		void set_interpolation_lin();
		void set_interpolation_cubic();
		void set_interpolation_spline();
		void set_interpolation_sinc();
		void set_waveform_flat();
		void set_waveform_sine();
		void set_waveform_square();
		void set_debug(int new_debug);
		void set_outbuf(short argc, t_atom *argv);
		void redraw();

	private:	
		gendy_waveform waveform;
		static bool debug;
		// class-wide variable to keep track of how many objects exist
		static unsigned int gendy_count;
		// instance ID
		unsigned int id;

		// waveform display buffer variables
		// buffer to copy to for waveform display
		flext::buffer *display_buf;
		
		// Internal class methods
		static void class_setup(t_classid thisclass);
		void set_interpolation(interpolation_t interpolation);
		void set_waveform(waveshape_t waveform);

		// register the callbacks, and tell flext their calling format
		FLEXT_CALLBACK_F(set_frequency)
		FLEXT_CALLBACK_I(set_num_breakpoints)
		FLEXT_CALLBACK_F(set_h_step)
		FLEXT_CALLBACK_F(set_v_step)
		FLEXT_CALLBACK_F(set_h_pull)
		FLEXT_CALLBACK_F(set_v_pull)
		FLEXT_CALLBACK(set_interpolation_lin)
		FLEXT_CALLBACK(set_interpolation_cubic)
		FLEXT_CALLBACK(set_interpolation_spline)
		FLEXT_CALLBACK(set_interpolation_sinc)
		FLEXT_CALLBACK(set_waveform_flat)
		FLEXT_CALLBACK(set_waveform_sine)
		FLEXT_CALLBACK(set_waveform_square)
		FLEXT_CALLBACK_I(set_debug)
		FLEXT_CALLBACK_V(set_outbuf)
		FLEXT_CALLBACK(redraw)
};
unsigned int gendy::gendy_count = 0;
bool gendy::debug = true;
#endif /* GENDY_H */
