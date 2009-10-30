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

	new_duration = old_duration + round(h_step * 
			(h_pull * (center_dur - old_duration) + 
			(1.0 - h_pull) * gauss() * center_dur));
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

gendydur_t breakpoint::get_duration() {
	return duration;
}

gendyamp_t breakpoint::get_amplitude() {
	return amplitude;
}

gendydur_t breakpoint::get_center_duration() {
	return center_dur;
}

gendyamp_t breakpoint::get_center_amplitude() {
	return center_amp;
}

// gendy_waveform class constructor with all default arguments
gendy_waveform::gendy_waveform() {
	//initialize variables to defaults
	wave_samples = new gendysamp_t[WAVE_BUFFER_INIT_SIZE];
	wave_buffer_size = WAVE_BUFFER_INIT_SIZE;
	step_width = 30;
	step_height = 0.1;
	duration_pull = 0.7;
	amplitude_pull = 0.4;
	constrain_endpoints = true;
	// set the average wavelength for 300 Hz at 44.1 kHz
	set_avg_wavelength(147);
	set_interpolation(LINEAR);
	set_waveshape(FLAT);
	copy_index = 0;
#ifdef FLEXT_VERSION
	display_buf = NULL;
	display_rate = 5;
	display = false;
#endif

	// start with a single breakpoint that spans the whole wavelength
	breakpoint_list.push_front(breakpoint(147,0,147,0));
	next_first.set_position(147,0);
	next_first.set_center(147,0);

	set_num_breakpoints(8);
	reset_breakpoints();
	move_breakpoints();
}

gendy_waveform::~gendy_waveform() {
	delete[] wave_samples;
}

void gendy_waveform::set_num_breakpoints(unsigned int new_size) {
	if(new_size == 0) {
		print_log("gendy~: Cannot resize to 0, resizing to 1", LOG_ERROR);
		new_size = 1;
	}
	while(breakpoint_list.size() < (new_size))
		add_breakpoint();
	while(breakpoint_list.size() > (new_size))
		remove_breakpoint();
	center_breakpoints();
}

void gendy_waveform::set_avg_wavelength(float new_wavelength) {
	// wave buffer needs to be at least 10 times the length of the average
	// waveform. When frequency changes, re-allocate if the current buffer
	// is too small
	if(wave_buffer_size < 10 * new_wavelength) {
		delete[] wave_samples;
		wave_samples = new gendysamp_t[10 * wave_buffer_size];
		wave_buffer_size = 10 * wave_buffer_size;
	}
	average_wavelength = new_wavelength;
	center_breakpoints();
}

void gendy_waveform::set_interpolation(interpolation_t new_interpolation) {
	interpolation_type = new_interpolation;
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

#ifdef FLEXT_VERSION
void gendy_waveform::display_toggle() {
	display = !display;
}

void gendy_waveform::display_toggle(bool state) {
	display = state;
}

void gendy_waveform::set_display_rate(int rate) {
	if(rate > 0)
		display_rate = rate;
}

void gendy_waveform::set_display_buffer(flext::buffer *buf) {
	display_buf = buf;
}

// conditionally copies waveform to Flext buffer and marks it dirty
void gendy_waveform::display_waveform() {
	static int display_count = 0;
	int n = 0;

	if(!display_buf || !display_buf->Ok()) {
		print_log("gendy-sfr: Invalid Buffer", LOG_ERROR);
		display = false;
		return;
	}

	if(++display_count >= display_rate) {
		flext::buffer::lock_t state = display_buf->Lock();
		display_buf->Update();
		// resize table to fit 1 wavelength
		//display_buf->Frames(current_wavelength, false, false);
		int bufsize = display_buf->Frames();
		while(n < current_wavelength && n < bufsize)
			display_buf->Poke(n, wave_samples[n++]);
		// zero out the rest of the buffer
		while(n < bufsize)
			display_buf->Poke(n++, 0);
		display_count = 0;
		display_buf->Dirty(true);
		display_buf->Unlock(state);
	}
}
#endif
// set new positions for all the breakpoints and update current_wavelength
// NB: could be easily modified and cleaned up a little if next_first was 
// stored in the last spot in the breakpoint list, instead of as a separate
// object
void gendy_waveform::move_breakpoints() {
	list<breakpoint>::iterator i;
	gendydur_t total_dur = 0;
	// first we copy in the previously calculated first breakpoint 
	breakpoint_list.begin()->set_duration(next_first.get_duration());
	breakpoint_list.begin()->set_amplitude(next_first.get_amplitude());
	total_dur += breakpoint_list.begin()->get_duration();
	// calculate new values for all the rest of the breakpoints
	for(i = ++breakpoint_list.begin(); i != breakpoint_list.end(); i++) {
		i->elastic_move(step_width, step_height, duration_pull, amplitude_pull);
		total_dur += i->get_duration();
	}
	current_wavelength = total_dur;

	// calculate new position for the first breakpoint in the next cycle
	next_first.elastic_move(step_width, step_height, duration_pull, amplitude_pull);
}

// fill the waveform buffer with one cycle of gendy waveform generated by
// interpolating the breakpoints
// NB: could also be cleaned up if next_first was just the last element in the
// breakpoint list
// TODO: could floating point errors cause the slope calculation to be off
// enough to create audible discontinuities between segments?
void gendy_waveform::generate_from_breakpoints() {

	if(interpolation_type == LINEAR) {
		// we'll be going through the waveform piecewise. next will store
		// the endpoint of the current section
		list<breakpoint>::iterator next = breakpoint_list.begin();
		// offset stores how many samples deep into the output buffer the
		// current segment starts
		unsigned int offset = 0;
		gendydur_t current_dur, next_dur;
		gendyamp_t current_amp, next_amp;
		double slope;
		current_dur = next->get_duration();
		current_amp = next->get_amplitude();
		// for each breakpoint 
		while(++next != breakpoint_list.end()) {
			next_amp = next->get_amplitude();
			slope = (next_amp - current_amp) / current_dur;
			for(unsigned int i = 0; i < current_dur; i++) {
				// slower but no discontuinities
				// wave_samples[i+offset] = current_amp + i / current_dur * 
				//		(next_amp - current_amp);
				wave_samples[i+offset] = current_amp + slope * i;
			}
			offset += current_dur;
			current_dur = next->get_duration();
			current_amp = next_amp;
		}
		// connect up with the first breakpoint of the next cycle
		next_amp = next_first.get_amplitude();
		slope = (next_amp - current_amp) / current_dur;
		for(unsigned int i = 0; i < current_dur; i++) {
			wave_samples[i+offset] = current_amp + slope * i;
		}
	}
	// TODO: implement other interpolations
#ifdef FLEXT_VERSION
	if(display)
		display_waveform();
#endif
}

// find the biggest space between breakpoints and add a new one
void gendy_waveform::add_breakpoint() {
	gendydur_t new_duration; 
	gendyamp_t new_amplitude;

	gendydur_t longest_dur = 0;
	list<breakpoint>::iterator longest_dur_breakpoint;
	list<breakpoint>::iterator i = breakpoint_list.begin();

	// find the largest space
	while(i != breakpoint_list.end()) {
		if((i->get_duration()) > longest_dur) {
			longest_dur = i->get_duration();
			longest_dur_breakpoint = i;
		}
		i++;
	} 

	// set the duration of the new breakpoint to be half the previous
	new_duration = longest_dur_breakpoint->get_duration() / 2;
	// halve the duration of the previous breakpoint
	longest_dur_breakpoint->set_duration(new_duration);
	// set the new amplitude to be the average of the 2 adjacent breakpoints
	// and move longest_dur_breakpoint iterator to the next breakpoint 
	// for insertion
	if(longest_dur_breakpoint == --breakpoint_list.end()) {
		new_amplitude = (longest_dur_breakpoint->get_amplitude() + 
				next_first.get_amplitude()) / 2;
		longest_dur_breakpoint++;
	}
	else
		new_amplitude = (longest_dur_breakpoint->get_amplitude() + 
				(++longest_dur_breakpoint)->get_amplitude()) / 2;
	// insert the breakpoint in the list
	breakpoint_list.insert(longest_dur_breakpoint, 
			breakpoint(new_duration, new_amplitude));
}

// remove the breakpoint closest to its neighbors. breakpoints centers
// should be reset after.
void gendy_waveform::remove_breakpoint() {
	// start smallest_space to be the largest integer as defined in limits.h
	gendydur_t smallest_space = numeric_limits<gendydur_t>::max();
	gendydur_t space;
	list<breakpoint>::iterator smallest_space_position;
	list<breakpoint>::iterator i = breakpoint_list.begin();

	// find the breakpoint closest to the adjacent breakpoints
	// NB: won't ever remove first or last breakpoint
	while(i != --breakpoint_list.end()) {
		if((space = i->get_duration() + (++i)->get_duration()) < smallest_space) {
			smallest_space = space;
			smallest_space_position = i;
		}
	} 
	// erase returns the list element following the erased one
	smallest_space_position = breakpoint_list.erase(smallest_space_position);
	// add the erased breakpoint's duration to the previous breakpoint
	(--smallest_space_position)->set_duration(smallest_space);
}

// center_breakpoints() calculates center positions for all the breakpoints
// TODO:
// sawtooth and triangle
void gendy_waveform::center_breakpoints() {
	list<breakpoint>::iterator current;
	unsigned int i = 0;
	gendydur_t new_dur;
	gendyamp_t new_amp;

	for(current = breakpoint_list.begin();
			current != breakpoint_list.end(); current++) {
		// evenly distribute the breakpoints along the waveform
		current->set_center_duration(average_wavelength / breakpoint_list.size());
		current->set_max_duration(int(average_wavelength * 10 / 
					breakpoint_list.size()));

		if(waveshape == FLAT)
			current->set_center_amplitude(0);
		else if(waveshape == SINE)
			current->set_center_amplitude(sin(2 * M_PI * i / breakpoint_list.size()));
		else if(waveshape == SQUARE)
			if((float(i) / breakpoint_list.size()) > 0.5)
				current->set_center_amplitude(1);
			else
				current->set_center_amplitude(-1);
		i++;
	}
	// copy new center info to the next_first breakpoint
	next_first.set_center(breakpoint_list.begin()->get_center_duration(),
			breakpoint_list.begin()->get_center_amplitude());
}

// reset_breakpoints() sets all of the breakpoint positions to be the
// center positions
void gendy_waveform::reset_breakpoints() {
	list<breakpoint>::iterator current;
	for(current = breakpoint_list.begin(); 
			current != breakpoint_list.end(); current++)
		current->set_position(current->get_center_duration(),
				current->get_center_amplitude());
	next_first.set_position(next_first.get_center_duration(),
			next_first.get_center_amplitude());
}

// copies the waveform into a buffer of size n until the buffer is full.  When
// the waveform is fully copied another cycle is generated.
unsigned int gendy_waveform::fill_buffer(gendysamp_t *buffer, unsigned int n) {
	unsigned int samples_copied = 0;
	// keep track of where in the waveform we're copying from
	while(samples_copied < n) {
		// if the waveform has been completely copied out, generate a new one
		if(copy_index == current_wavelength) {
			move_breakpoints();
			generate_from_breakpoints();
			copy_index = 0;
		}
		// copy samples until the output buffer or the waveform buffer runs out
		while(samples_copied < n && copy_index < current_wavelength)
			buffer[samples_copied++] = wave_samples[copy_index++];
	}
	return samples_copied;
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
