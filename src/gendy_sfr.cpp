// TODO:
// check constructors/destructors/copy constructors/assignment
// check waveform buffer resizing
// add license stuff to header/implementation files
//

#include "gendy_sfr.h"
#include <limits>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

using namespace std;

// shows msg if allowed by LOG_LEVEL
#ifdef PD_MAJOR_VERSION
extern void print_log(const char *msg, int level){
  if (LOG_LEVEL >= level) {
    post(msg);
  }
}

extern void print_log(const char *msg, int arg1, int level){
  if (LOG_LEVEL >= level) {
    post(msg, arg1);
  }
}

extern void print_log(const char *msg, unsigned int arg1, int level){
  if (LOG_LEVEL >= level) {
    post(msg, arg1);
  }
}

extern void print_log(const char *msg, float arg1, int level){
  if (LOG_LEVEL >= level) {
    post(msg, arg1);
  }
}
#else
extern void print_log(char *msg, int level) {}
extern void print_log(const char *msg, float arg1, int level){}
extern void print_log(const char *msg, float arg1, int level){}
#endif

// wave buffer will be initialized large enough to hold 10
// seconds of data at 44.1kHz
const int WAVE_BUFFER_INIT_SIZE = 441000;

breakpoint::breakpoint() {
	print_log("gendy~: New breakpoint with default args", LOG_DEBUG);
	duration = 0;
	amplitude = 0;
	center_dur = 0;
	center_amp = 0;
	max_duration = 0;
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
	max_duration = center_dur * 10;
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
// amplitudes out of the audio range [-1,1] are mirrored back in.  negative
// durations are mirrored across 0 to become positive. Durations greater 
// than max_duration are mirrored across max_duration
//
void breakpoint::elastic_move(gendydur_t h_step, gendyamp_t v_step, 
		gendydur_t h_pull, gendyamp_t v_pull) {
	gendydur_t old_duration = duration;
	gendydur_t new_duration;
	gendyamp_t new_amplitude;

	new_duration = old_duration + h_step * 
			(h_pull * (center_dur - old_duration) + 
			(1.0 - h_pull) * gauss() * center_dur);
	/*new_duration = old_duration + round((1.0 - h_pull) * h_step * gauss() +
		(h_pull * (center_dur - old_duration))); */
	// set mirror boundaries on new_duration
	do {
		if(new_duration > max_duration)
			new_duration = (2 * max_duration) - new_duration;
		if(new_duration < 0)
			new_duration = -new_duration;
	} while (new_duration < 0 || new_duration > max_duration);
	
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
	max_duration = new_duration * 10;
}

void breakpoint::set_max_duration(gendydur_t new_max) {
	max_duration = new_max;
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

// gendy_waveform class constructor with all default arguments
gendy_waveform::gendy_waveform() {
	//initialize variables to defaults
	step_width = 30;
	step_height = 0.1;
	duration_pull = 0.7;
	amplitude_pull = 0.4;
	constrain_endpoints = true;
	
	// start with a single breakpoint that spans the whole wavelength
	breakpoint_list.push_front(breakpoint(147,0,147,0));
	// and add an guard point to the end (for linear interpolation)
	breakpoint_list.push_back(breakpoint(147,0,147,0));
	// no pre-guard points
	breakpoint_begin = breakpoint_list.begin();
	breakpoint_current = breakpoint_list.begin();
	// 1 end guard point, which acts as the end of the list of actual
	// breakpoints
	breakpoint_end = --breakpoint_list.end();

	set_interpolation(LINEAR);
	
	// set the average wavelength for 300 Hz at 44.1 kHz
	set_avg_wavelength(147);
	waveshape = FLAT;
	phase = 0;


	set_num_breakpoints(8);
	reset_breakpoints();
	move_breakpoints();
}

gendy_waveform::~gendy_waveform() {
}

//TODO: protect against this getting called before data
//strucutres are set up
void gendy_waveform::set_num_breakpoints(unsigned int new_size) {
	if(new_size == 0) {
		print_log("gendy~: Cannot resize to 0, resizing to 1", LOG_INFO);
		new_size = 1;
	}

	unsigned int target_length = new_size + get_num_guardpoints();

	while(breakpoint_list.size() < (target_length))
		add_breakpoint();
	while(breakpoint_list.size() > (target_length))
		remove_breakpoint();
	center_breakpoints();
}

void gendy_waveform::set_avg_wavelength(float new_wavelength) {
	average_wavelength = new_wavelength;
	center_breakpoints();
}


unsigned int gendy_waveform::get_num_guardpoints() const {
	return breakpoint_list.size() - get_num_breakpoints();
}

unsigned int gendy_waveform::get_num_breakpoints() const {
	list<breakpoint>::const_iterator i = breakpoint_begin;
	unsigned int num_breakpoints = 0;
	while(i != breakpoint_end) {
		++i;
		++num_breakpoints;
	}
	return num_breakpoints;
}

void gendy_waveform::set_pre_guardpoints(unsigned int guardpoints) {
	list<breakpoint>::iterator src = breakpoint_end;
	list<breakpoint>::iterator i = breakpoint_begin;
	//work backwards from the first breakpoint until:
	// a) we get to the beginning of the list (more guard points needed)
	// b) guardpoints goes to 0 (we need to remove guardpoints)
	// c) both (we have the proper number of guardpoints)
	//we keep track of src because that's where we'll start copying
	//guard points from
	while(guardpoints && i != breakpoint_list.begin()) {
		--guardpoints;
		--i;
		--src;
	}
	//if we're not at the beginning of the list yet, delete guard points
	while(i != breakpoint_list.begin())
		breakpoint_list.pop_front();
	//if guardpoints hasn't run out yet, we need some more. copy them.
	while(guardpoints--)
		breakpoint_list.push_front(*(src--));
}

void gendy_waveform::set_post_guardpoints(unsigned int guardpoints) {
	list<breakpoint>::iterator src = breakpoint_begin;
	// move breakpoint_end to point at the last breakpoint instead
	// of the first node after the last breakpoint, so it doesn't
	// get clobbered
	--breakpoint_end;
	list<breakpoint>::iterator i = breakpoint_end;
	//work forwards from the last breakpoint until:
	// a) we get to the end of the list (more guard points needed)
	// b) guardpoints goes to 0 (we need to remove guardpoints)
	// c) both (we have the proper number of guardpoints)
	//we keep track of src because that's where we'll start copying
	//guard points from
	while(guardpoints && i != --breakpoint_list.end()) {
		--guardpoints;
		++i;
		++src;
	}
	//if we're not at the beginning of the list yet, delete guard points
	while(i != --breakpoint_list.end())
		breakpoint_list.pop_back();
	//if guardpoints hasn't run out yet, we need some more. copy them.
	while(guardpoints--)
		breakpoint_list.push_back(*(src++));
	//now we put breakpoint_end back where it's supposed to be
	++breakpoint_end;
}

void gendy_waveform::set_interpolation(interpolation_t new_interpolation) {
	if(new_interpolation == LINEAR) {
		interpolation_type = LINEAR;
		set_pre_guardpoints(0);
		set_post_guardpoints(1);
	}
	else if(new_interpolation == CUBIC) {
		interpolation_type = CUBIC;
		set_pre_guardpoints(1);
		set_post_guardpoints(2);
	}
	else {
		print_log("gendy~: unimplemented interpolation. defaulting to linear",
				LOG_ERROR);
	}
}

/*
void gendy_waveform::set_interpolation_type(t_symbol *new_interpolation) {
	if(strcmp(GetString(new_interpolation), "linear") == 0)
		interpolation_type = LINEAR;
	else
		post("So far only linear interpolation implemented.");
} */

void gendy_waveform::set_waveshape(waveshape_t new_waveshape) {
	waveshape = new_waveshape;
	center_breakpoints();
}

void gendy_waveform::set_step_width(float new_width) {
	step_width = new_width;
}

void gendy_waveform::set_step_height(float new_height) {
	step_height = new_height;
}

void gendy_waveform::set_duration_pull(float new_pull) {
	duration_pull = new_pull;
}

void gendy_waveform::set_amplitude_pull(float new_pull) {
	amplitude_pull = new_pull;
}

void gendy_waveform::set_constrain_endpoints(bool constrain) {
	constrain_endpoints = constrain;
}

float gendy_waveform::get_wavelength() const {
	list<breakpoint>::iterator i;
	float wavelength = 0;
	for(i = breakpoint_begin; i != breakpoint_end; i++)
		wavelength += i->get_duration();
	return wavelength;
}

// set new positions for all the breakpoints 
void gendy_waveform::move_breakpoints() {
	int breakpoint_count;
	
	// first we copy the last breakpoints of the current cycle into 
	// the pre guard points, which represent the past
	
	list<breakpoint>::iterator i = breakpoint_begin;
	list<breakpoint>::iterator j = breakpoint_end;
	while(i != breakpoint_list.begin())
		*(--i) = *(--j);

	// now we copy the post guard points (which represent the first
	// breakpoints of the next cycle) into the beginning of this cycle
	i = breakpoint_begin;
	j = breakpoint_end;
	while(j != breakpoint_list.end())
		*(i++) = *(j++);
	// i now points to the first breakpoint that needs to be newly calculated.
	// we'll also continue into the post guard points and recalculate those,
	// they are the future

	
	// calculate new values for all the rest of the breakpoints
	while(i != breakpoint_list.end()) {
		i->elastic_move(step_width, step_height, duration_pull, amplitude_pull);
		i++;
	}
}

// adds a breakpoint by splitting the longest breakpoint into two
void gendy_waveform::add_breakpoint() {
	gendydur_t new_duration; 
	gendyamp_t new_amplitude;

	gendydur_t longest_dur = 0;
	list<breakpoint>::iterator longest_dur_breakpoint;
	list<breakpoint>::iterator breakpoint_iter;

	// find the longest breakpoint
	for(breakpoint_iter = breakpoint_begin; 
			breakpoint_iter != breakpoint_end;
			breakpoint_iter++) {
		if((breakpoint_iter->get_duration()) > longest_dur) {
			longest_dur = breakpoint_iter->get_duration();
			longest_dur_breakpoint = breakpoint_iter;
		}
	} 

	// set the duration of the new breakpoint to be half the previous
	new_duration = longest_dur_breakpoint->get_duration() / 2;
	// halve the duration of the previous breakpoint
	longest_dur_breakpoint->set_duration(new_duration);
	// set the new amplitude to be the average of the 2 adjacent breakpoints
	// and move longest_dur_breakpoint iterator to the next breakpoint 
	// for insertion
	new_amplitude = (longest_dur_breakpoint->get_amplitude() + 
			(++longest_dur_breakpoint)->get_amplitude()) / 2;
	// insert the breakpoint in the list
	breakpoint_list.insert(longest_dur_breakpoint, 
			breakpoint(new_duration, new_amplitude));
	//TODO: copy data to guard points if need be
}

// remove the breakpoint closest to its neighbors. breakpoints centers
// should be reset after.
void gendy_waveform::remove_breakpoint() {
	// start smallest_space to be the largest it can be
	gendydur_t smallest_space = numeric_limits<gendydur_t>::max();
	gendydur_t space;
	list<breakpoint>::iterator smallest_space_position;
	
	// find the breakpoint closest to the adjacent breakpoints
	list<breakpoint>::iterator breakpoint_iter = breakpoint_begin;
	list<breakpoint>::iterator breakpoint_last = breakpoint_end;
	--breakpoint_last;
	while(breakpoint_iter != breakpoint_last) {
		space = breakpoint_iter->get_duration() + 
			(++breakpoint_iter)->get_duration();
		if(space < smallest_space) {
			smallest_space = space;
			smallest_space_position = breakpoint_iter;
		}
	} 
	// erase returns the list element following the erased one
	smallest_space_position = breakpoint_list.erase(smallest_space_position);
	// add the erased breakpoint's duration to the previous breakpoint
	(--smallest_space_position)->set_duration(smallest_space);
	//TODO: update guard points if need be
}

// center_breakpoints() calculates center positions for all the breakpoints
// TODO: sawtooth and triangle
// TODO: crashes when called on empty breakpoint list
void gendy_waveform::center_breakpoints() {
	gendydur_t new_dur;
	gendyamp_t new_amp;

	list<breakpoint>::iterator breakpoint_iter = breakpoint_begin;
	unsigned int breakpoint_index = 0;
	unsigned int num_breakpoints = get_num_breakpoints();
	while(breakpoint_iter != breakpoint_end) {
		// evenly distribute the breakpoints along the waveform
		breakpoint_iter->set_center_duration(average_wavelength / num_breakpoints);
		breakpoint_iter->set_max_duration(average_wavelength * 10 / num_breakpoints);

		float t = (breakpoint_index) / (float)num_breakpoints;
		if(waveshape == FLAT)
			breakpoint_iter->set_center_amplitude(0);
		else if(waveshape == SINE)
			breakpoint_iter->set_center_amplitude(sin(2 * M_PI * t));
		else if(waveshape == SQUARE)
			breakpoint_iter->set_center_amplitude(t < 0.5 ? 1 : -1);
		breakpoint_iter++;
		breakpoint_index++;
	}
	
	// copy center data starting at the end of the actual breakpoints
	// into the beginning guard points
	list<breakpoint>::iterator i = breakpoint_begin;
	list<breakpoint>::iterator j = breakpoint_end;
	while(i != breakpoint_list.begin())
		*(--i) = *(--j);

	// copy center data starting at the beginning of the actual breakpoints
	// into the end guard points
	i = breakpoint_begin;
	j = breakpoint_end;
	while(j != breakpoint_list.end())
		*(j++) = *(i++);
}

// reset_breakpoints() sets all of the breakpoint positions to be the
// center positions
void gendy_waveform::reset_breakpoints() {
	list<breakpoint>::iterator current;
	for(current = breakpoint_list.begin(); 
			current != breakpoint_list.end(); current++)
		current->set_position(current->get_center_duration(),
				current->get_center_amplitude());
}

/* 
 * generates a block of gendy audio.
 * This function will take care of moving the breakpoints when it reaches
 * the end of a cycle.
 */
//TODO: should this really return the number of samples copied? it's always
//      bufsize
unsigned int gendy_waveform::get_block(gendysamp_t *dest, unsigned int bufsize) {
	if(interpolation_type == LINEAR) {
		// assert that we have no pre guard points and at least 1 post guard point
		assert(breakpoint_begin == breakpoint_list.begin());
		assert(breakpoint_end != breakpoint_list.end());
		// generate the endpoints for the current segment
		list<breakpoint>::iterator breakpoint_next = breakpoint_current;
		breakpoint_next++;
		gendydur_t current_dur = breakpoint_current->get_duration();
		gendydur_t current_amp = breakpoint_current->get_amplitude();
		gendydur_t next_dur = breakpoint_next->get_duration();
		gendydur_t next_amp = breakpoint_next->get_amplitude();

		for(unsigned int i = 0; i < bufsize; i++) {
			dest[i] = current_amp + phase / current_dur * (next_amp - current_amp);
			phase++;
			// if we've reached the end of the current segment
			if(phase > current_dur) {
				phase -= current_dur;
				current_dur = next_dur;
				current_amp = next_amp;
				// if we've reached the end of this cycle
				if(breakpoint_next == breakpoint_end) {
					move_breakpoints();
					// wrap around the current and next iterators to the beginning
					breakpoint_current = breakpoint_begin;
					breakpoint_next = breakpoint_begin;
					breakpoint_next++;
				}
				else {
					//just increment both breakpoint iterators
					breakpoint_current = breakpoint_next;;
					breakpoint_next++;
				}
				next_dur = breakpoint_next->get_duration();
				next_amp = breakpoint_next->get_amplitude();
			}
		}
	}
	else if(interpolation_type == CUBIC) {
		/*list<breakpoint>::const_iterator next = breakpoint_list.begin();
		gendydur_t dur[4];
		gendyamp_t amp[4];
		//collect the 4 points needed to interpolate in the first segment
		for(int i = 0; i < 4; i++) {
			dur[i] = next->get_duration();
			amp[i] = next->get_amplitude();
			++next;
		}
		*/
		static int samplecount = 0;
		samplecount += bufsize;
		if(samplecount > 512) {
			move_breakpoints();
			samplecount = 0;
		}
		for(int i = 0; i < bufsize; i++)
			dest[i] = 0;
	}
	else {
		print_log("gendy~: Unimplemeted Interpolation Type", LOG_ERROR);
		assert(0);
	}
	return bufsize;
}

//TODO:needs protection against buffer overrun
unsigned int gendy_waveform::get_cycle(gendysamp_t *dest, unsigned int bufsize) const {
	if(interpolation_type == LINEAR) {
		assert(get_num_guardpoints() == 1);

		// we'll be going through the waveform piecewise. next stores 
		// the endpoint of the current section
		list<breakpoint>::const_iterator next = breakpoint_list.begin();
		// keep track of how long before a sample boundry the current
		// segment started. the first segment will start at the beginning of the buffer
		gendydur_t segment_shift = 0;
		gendydur_t current_dur, next_dur;
		gendyamp_t current_amp, next_amp;
		double slope;
		current_dur = next->get_duration();
		current_amp = next->get_amplitude();

		unsigned int buffer_offset = 0;
		// for each breakpoint 
		while(++next != breakpoint_list.end()) {
			next_amp = next->get_amplitude();
			slope = (next_amp - current_amp) / current_dur;
			unsigned int i = 0;
			while(i + segment_shift < current_dur && i + buffer_offset < bufsize) {
				dest[i + buffer_offset] = current_amp + 
					slope * (i + segment_shift);
				i++;
			}
			buffer_offset += i;
			segment_shift = (gendydur_t)i + segment_shift - current_dur;
			current_dur = next->get_duration();
			current_amp = next_amp;
		}
		return buffer_offset;
	}
	else if(interpolation_type == CUBIC) {
		assert(get_num_guardpoints() == 3);
		unsigned int N = breakpoint_list.size();
		double *x = (double *)malloc(N * sizeof(double));
		assert(x != 0);
		double *y = (double *)malloc(N * sizeof(double));
		assert(y != 0);
		list<breakpoint>::const_iterator current = breakpoint_list.begin();
		double guardpoint_offset = 0;
		while(current != breakpoint_begin) {
			guardpoint_offset += current->get_duration();
			current++;
		}
		current = breakpoint_list.begin();

		x[0] = -guardpoint_offset;
		y[0] = current->get_amplitude();
		for(int i = 1; i < N; i++) {
			x[i] = x[i-1] + current->get_duration();
			++current;
			y[i] = current->get_amplitude();
		}

		gsl_interp_accel *acc = gsl_interp_accel_alloc();
		gsl_spline *spline = gsl_spline_alloc(gsl_interp_cspline, N);
		gsl_spline_init(spline, x, y, N);

		int i = 0;
		while(i < x[N-1] && i < bufsize) {
			dest[i] = gsl_spline_eval(spline, i, acc);
			i++;
		}

		gsl_spline_free(spline);
		gsl_interp_accel_free(acc);

		return i;

	}
}

// return a uniformly distributed double-precision float between 0 and 1
double randf() {
	return rand() / static_cast<double>(RAND_MAX);
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
