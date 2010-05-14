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




#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "types.h"

class breakpoint
{
	// store current breakpoint location
	// duration is in samples
	gendyamp_t amplitude;
	gendydur_t duration;

	// store the center point the breakpoint gravitates to
	gendyamp_t center_amp;
	gendydur_t center_dur;

	public:
	breakpoint();
	breakpoint(gendydur_t duration, gendyamp_t amplitude);
	breakpoint(gendydur_t duration, gendyamp_t amplitude,
			gendydur_t center_dur, gendyamp_t center_amp);
	void elastic_move(gendydur_t h_step, gendyamp_t v_step,
			gendydur_t h_pull, gendyamp_t v_pull);
	void set_duration(gendydur_t new_duration);
	void set_amplitude(gendyamp_t new_amplitude);
	void set_position(gendydur_t new_duration, gendyamp_t new_amplitude);
	void set_center_duration(gendydur_t new_duration);
	void set_center_amplitude(gendyamp_t new_amplitude);
	void set_center(gendydur_t new_duration, gendyamp_t new_amplitude);
	gendydur_t get_duration() const;
	gendyamp_t get_amplitude() const;
	gendydur_t get_center_duration() const;
	gendyamp_t get_center_amplitude() const;
};
#endif /* BREAKPOINT_H */
