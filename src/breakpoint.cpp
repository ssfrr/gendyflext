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




#include "breakpoint.h"
#include "log.h"
#include <cmath>
#include "util.h"

breakpoint::breakpoint() {
	print_log("gendy~: New breakpoint with default args", LOG_DEBUG);
	duration = 0;
	amplitude = 0;
	center_dur = 0;
	center_amp = 0;
}

breakpoint::breakpoint(gendydur_t duration, gendyamp_t amplitude) {
	print_log("gendy~: New breakpoint with duration %u", duration, LOG_DEBUG);
	print_log("gendy~: \t\t\tamplitude %f", amplitude, LOG_DEBUG);
	this->duration = duration;
	this->amplitude = amplitude;
}

breakpoint::breakpoint(gendydur_t duration, gendyamp_t amplitude,
		gendydur_t center_dur, gendyamp_t center_amp) {
	print_log("gendy~: New breakpoint with duration %u", duration, LOG_DEBUG);
	print_log("gendy~: \t\t\tamplitude %f", amplitude, LOG_DEBUG);
	print_log("gendy~: \t\t\tcenter_dur %u", center_dur, LOG_DEBUG);
	print_log("gendy~: \t\t\tcenter_amp %f", center_amp, LOG_DEBUG);
	this->duration = duration;
	this->amplitude = amplitude;
	this->center_dur = center_dur;
	this->center_amp = center_amp;
}

// elastic_move
//
// elastic_move sets a new position for the breakpoint. The breakpoint is
// moved (vertically and horizontally) from its current position by a normal
// distributed distance, the standard deviations of which are h_step and
// v_step.
//
// the breakpoint is also pulled toward its center by the factors h_pull and
// v_pull, with a 1 corresponding to immediately jumping to the center point
// and 0 not pulling toward the center point at all.
//
// amplitudes out of the audio range [-1,1] are mirrored back in.
// durations less than 2 samples are set to 2.
//
void breakpoint::elastic_move(gendydur_t h_step, gendyamp_t v_step,
		gendydur_t h_pull, gendyamp_t v_pull) {
	gendydur_t old_duration = duration;
	gendydur_t new_duration;
	gendyamp_t new_amplitude;

	new_duration = old_duration *
			pow(center_dur / old_duration,h_pull * h_step) *
			exp(gauss() *0.1 * h_step * (1.0-h_pull));
	/*new_duration = old_duration + round((1.0 - h_pull) * h_step * gauss() +
		(h_pull * (center_dur - old_duration))); */
	// set boundaries on new_duration
	if(new_duration < 2)
		new_duration = 2;
	
	new_amplitude = amplitude + v_step *
			(v_pull * (center_amp - amplitude) +
			(1.0 - v_pull) * gauss());
	/*new_amplitude = amplitude + (1 - v_pull) * v_step * gauss() +
		v_pull * (center_amp - amplitude);*/
	// set mirror boundaries on new_amplitude
	do {
		if(new_amplitude > 1)
			new_amplitude = 2 - new_amplitude;
		if(new_amplitude < -1)
			new_amplitude = -2 - new_amplitude;
	} while (new_amplitude < -1 || new_amplitude > 1);

	duration = new_duration;
	amplitude = new_amplitude;
}

void breakpoint::set_duration(gendydur_t new_duration) {
	duration = new_duration;
}

void breakpoint::set_amplitude(gendyamp_t new_amplitude) {
	amplitude = new_amplitude;
}

void breakpoint::set_position(gendydur_t new_duration, gendyamp_t new_amplitude) {
	//TODO: argument sanitization
	duration = new_duration;
	amplitude = new_amplitude;
}

void breakpoint::set_center_duration(gendydur_t new_duration) {
	center_dur = new_duration;
}

void breakpoint::set_center_amplitude(gendyamp_t new_amplitude) {
	center_amp = new_amplitude;
}

// set_center
// accessor function to set the amplitude and duration of a breakpoint's
// center position

void breakpoint::set_center(gendydur_t new_duration, gendyamp_t new_amplitude) {
	//TODO: argument sanitization
	center_dur= new_duration;
	center_amp = new_amplitude;
}

gendydur_t breakpoint::get_duration() const {
	return duration;
}

gendyamp_t breakpoint::get_amplitude() const {
	return amplitude;
}

gendydur_t breakpoint::get_center_duration() const {
	return center_dur;
}

gendyamp_t breakpoint::get_center_amplitude() const {
	return center_amp;
}
