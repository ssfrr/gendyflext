// gendy~ - an implementation of Iannis Xenakis' Dynamic
// Stochastic Synthesis as an object for Pure Data.
//
// This project uses Thomas Grill's Flext framework, which
// is necessary to compile and run it.
//
// Released under the GPL v3, copyright (c) 2009 Spencer Russell
//

#include <math.h>
#include <flext.h>
#include "gendy_sfr.h"

using namespace std;
 
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

// gendy~ version 0.0.1:
const int GENDY_MAJ = 0;
const int GENDY_MIN = 0;
const int GENDY_REV = 1;

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
		void set_debug(int new_debug);
		void set_outbuf(short argc, t_atom *argv);

	private:	
		gendy_waveform waveform;
		static bool debug;
		// class-wide variable to keep track of how many objects exist
		static unsigned int gendy_count;
		// instance ID
		unsigned int id;

		// waveform display buffer variables
		buffer *display_buf;
		t_symbol *display_bufname;
		bool display;
		int display_refresh;
		
		// Internal class methods
		static void class_setup(t_classid thisclass);
		void set_interpolation(int interpolation);

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
		FLEXT_CALLBACK_I(set_debug)
		FLEXT_CALLBACK(set_outbuf)
		FLEXT_CALLBACK_F(set_outbuf)
		FLEXT_CALLBACK_S(set_outbuf)
};
unsigned int gendy::gendy_count = 0;
bool gendy::debug = true;

// object class constructor(run at each gendy object creation)
gendy::gendy() {
	gendy_count++;
	id = gendy_count;
	if(debug)
		post("gendy~ #%d: Constructor initiated", id);
	AddInAnything("control input");	// control input
	AddOutSignal("audio out");		  // audio output

	display_buf = NULL;
	display = false;
	//default to displaying every 5 cycles
	display_refresh = 5;

	if(debug)
		post("gendy~ #%d: Constructor terminated", id);
}

gendy::~gendy() {
	if(debug)
		post("gendy~ #%d: Destructor initiated", id);
	gendy_count--;
	if(debug)
		post("gendy~ #%d: Destructor terminated", id);
}

void gendy::class_setup(t_classid thisclass) {
	// associate methods with incoming messages on inlet 0
	// numerical input is taken as frequency
	if(debug)
		post("Class constructor beginning");
	FLEXT_CADDMETHOD(thisclass, 0, set_frequency);
	FLEXT_CADDMETHOD_(thisclass, 0, "breakpoints", set_num_breakpoints);
	FLEXT_CADDMETHOD_(thisclass, 0, "h_step", set_h_step);
	FLEXT_CADDMETHOD_(thisclass, 0, "v_step", set_v_step);
	FLEXT_CADDMETHOD_(thisclass, 0, "h_pull", set_h_pull);
	FLEXT_CADDMETHOD_(thisclass, 0, "v_pull", set_v_pull);
	FLEXT_CADDMETHOD_(thisclass, 0, "linear", set_interpolation_lin);
	FLEXT_CADDMETHOD_(thisclass, 0, "cubic", set_interpolation_cubic);
	FLEXT_CADDMETHOD_(thisclass, 0, "spline", set_interpolation_spline);
	FLEXT_CADDMETHOD_(thisclass, 0, "sinc", set_interpolation_sinc);
	FLEXT_CADDMETHOD_(thisclass, 0, "debug", set_debug);
	FLEXT_CADDMETHOD_(thisclass, 0, "display", set_outbuf);
	post("--- gendy~ by Spencer Russell ---");
	if(debug)
		post("Class constructor ending");
}

// Now we define our DSP function. It gets these arguments:
// 
// n: length of signal vector.
// float *const *in, float *const *out: 
//  These are arrays of signal vectors(in is a pointer to const pointer to float)

void gendy::m_signal(int n, float *const *in, float *const *out) {
	
	// point outs to the output signal vector
	waveform.get_wave_data(out[0], n);
}

// Message handling functions

void gendy::set_frequency(float new_freq) {
	waveform.set_avg_wavelength(Samplerate() / new_freq);
}

void gendy::set_num_breakpoints(float num_breakpoints) {
	waveform.set_num_breakpoints(num_breakpoints);
}

void gendy::set_h_step(float new_stepsize) {
	waveform.set_step_width(new_stepsize);
}

void gendy::set_v_step(float new_stepsize) {
	waveform.set_step_height(new_stepsize);
}

void gendy::set_h_pull(float new_pull) {
	waveform.set_amplitude_pull(new_pull);
}

void gendy::set_v_pull(float new_pull) {
	waveform.set_duration_pull(new_pull);
}

void gendy::set_interpolation_lin() {
	set_interpolation(LINEAR);
}

void gendy::set_interpolation_cubic() {
	set_interpolation(CUBIC);
}

void gendy::set_interpolation_spline() {
	set_interpolation(SPLINE);
}

void gendy::set_interpolation_sinc() {
	set_interpolation(SINC);
}

void gendy::set_debug(int new_debug) {
	if(new_debug)
		debug = true;
	else
		debug = false;
}

void gendy::set_outbuf() {
	// no arguement toggles waveform display
	display = !display;
}

void gendy::set_outbuf(int arg) {
	// zero turns display off
	if(arg == 0)
		display = false;
	// non-zero numerical argument sets display rate, turns display on
	else {
		display = true;
		display_refresh = arg;
	}
}

void gendy::set_outbuf(t_symbol *bufname) {
	// symbol argument, set output buffer
	// TODO: better error reporting
	display_bufname = bufname; 
	// delete existing buffer reference
	if(display_buf) {
		delete display_buf;
	}
	display_buf = new buffer(bufname);
	if(!display_buf->Ok()) {
		post("gendy~: buffer not valid");
		delete display_buf;
		display_buf = NULL;
		display_bufname = NULL;
	}
	else 
		display = true;
}

// private class methods
void gendy::set_interpolation(int new_interpolation) {
	waveform.set_interpolation(new_interpolation);
}

//register the gendy class as a PD or Max object
FLEXT_NEW_DSP("gendy~", gendy)
