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




#ifndef GENDY_WAVEFORM_H
#define GENDY_WAVEFORM_H

#include "types.h"
#include "breakpoint.h"
#include <list>

class gendy_waveform
{
	// keep track of where we are in the current segment(in samples)
	gendydur_t phase;
	// average wavelength in samples. 	
	float average_wavelength;
	// list of breakpoints plus guard points(for continuity)
	std::list<breakpoint> breakpoint_list;
	// the first breakpoint after the guard points
	std::list<breakpoint>::iterator breakpoint_begin;
	// the first guard point after the breakpoints
	std::list<breakpoint>::iterator breakpoint_end;
	// the current breakpoint that the next request block will start with
	std::list<breakpoint>::iterator breakpoint_current;;
	// set the type of interpolation(see defines at top)
	interpolation_t interpolation_type;
	// the waveshape that the breakpoints will gravitate to
	waveshape_t waveshape;
	// constrain endpoints to 0
	bool constrain_endpoints;
	// determines the motion of the breakpoints. expected to be 0-1, where
	// 0 is no motion and 1 is basically fully random jumps
	float step_width;
	float step_height;
	// the extent to which the duration and amplitude are pulled to match
	// the waveshape. ranges from 0 to 1
	float duration_pull;
	float amplitude_pull;
	// eventually debugging info will be switchable on an object-basis
	bool debug;

	void move_breakpoints();
	void generate_from_breakpoints();
	void add_breakpoint();
	void remove_breakpoint();
	void center_breakpoints();
	void reset_breakpoints();
	void set_pre_guardpoints(unsigned int guardpoints);
	void set_post_guardpoints(unsigned int guardpoints);

	public:
	gendy_waveform();
	~gendy_waveform();
	//gendy_waveform(float freq);
	void set_num_breakpoints(int new_size);
	void set_avg_wavelength(float new_wavelength);
	void set_interpolation(interpolation_t new_interpolation);
	void set_waveshape(waveshape_t new_waveshape);
	void set_step_width(float new_width);
	void set_step_height(float new_height);
	void set_amplitude_pull(float new_pull);
	void set_duration_pull(float new_pull);
	void set_constrain_endpoints(bool constrain);
	float get_wavelength() const;
	unsigned int get_num_breakpoints() const;
	unsigned int get_num_guardpoints() const;
	unsigned int get_block(gendysamp_t *dest, unsigned int bufsize);
	unsigned int get_cycle(gendysamp_t *dest, unsigned int bufsize) const;
}; //end gendy_waveform class def

#endif /* GENDY_WAVEFORM_H */
