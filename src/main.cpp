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
	private:	
		gendy_waveform *waveform;
		bool debug;
		// class-wide variable to keep track of how many objects exist
		static gendy_count = 0;
		// instance ID
		unsigned int id;
        
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
        FLEXT_CALLBACK_I(set_debug)

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
		void set_debug(int new_debug);

		// Internal class methods
        static void class_setup(t_classid thisclass);
		void set_interpolation(unsigned int interpolation);
};


// object class constructor(run at each gendy object creation)
gendy::gendy() {
	gendy_count++;
	id = gendy_count;
	if(debug)
		post("gendy~ #%d: Constructor initiated", id);
    AddInAnything("control input");    // control input
    AddOutSignal("audio out");          // audio output
	// TODO: error checking for memory allocation
	waveform = new gendy_waveform;
	debug = false;
	if(debug)
		post("gendy~ #%d: Constructor terminated", id);
}

gendy::~gendy() {
	if(debug)
		post("gendy~ #%d: Destructor initiated", id);
	gendy_count--;
	delete waveform;
	if(debug)
		post("gendy~ #%d: Destructor terminated", id);
}

void gendy::class_setup(t_classid thisclass) {
	// associate methods with incoming messages on inlet 0
	// numerical input is taken as frequency
    FLEXT_CADDMETHOD(thisclass, 0, set_frequency);
    FLEXT_CADDMETHOD_(thisclass, 0, "breakpoints", set_num_breakpoints);
    FLEXT_CADDMETHOD_(thisclass, 0, "h_step", set_h_step);
    FLEXT_CADDMETHOD_(thisclass, 0, "v_step", set_v_step);
    FLEXT_CADDMETHOD_(thisclass, 0, "h_pull", set_h_pull);
    FLEXT_CADDMETHOD_(thisclass, 0, "v_pull", set_v_pull);
    FLEXT_CADDMETHOD_(thisclass, 0, "interpolation", set_interpolation);
    FLEXT_CADDMETHOD_(thisclass, 0, "debug", set_debug);
    post("--- gendy~ by Spencer Russell ---");
}

// Now we define our DSP function. It gets these arguments:
// 
// n: length of signal vector.
// float *const *in, float *const *out: 
//  These are arrays of signal vectors(in is a pointer to const pointer to float)

void gendy::m_signal(int n, float *const *in, float *const *out) {
	
    // point outs to the output signal vector
	float *outs = out[0];
    int filled_index = 0;
    int retreived_samples = 0;
	
    // keep grabbing cycles of waveform until the output buffer is full
	while (filled_index < n) {
	    retreived_samples = waveform.get_wave_data(outs + filled_index, 
				n - filled_index);
		// move insertion index by the number of samples we just grabbed
        filled_index += retreived_samples;
	}
}

// Message handling functions

void gendy::set_frequency(float new_freq) {
	waveform.set_avg_wavelength(Samplerate() / new_freq);
}

void gendy::set_num_breakpoints(float num_breakpoints) {
	waveform.set_num_breakpoints(num_breakpoints);
}

void gendy::set_h_step(float new_stepsize) {
	waveform.set_h_step(new_stepsize);
}

void gendy::set_v_step(float new_stepsize) {
	waveform.set_v_step(new_stepsize);
}

void gendy::set_h_pull(float new_pull) {
	waveform.set_h_pull(new_pull);
}

void gendy::set_v_pull(float new_pull) {
	waveform.set_v_pull(new_pull);
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

// private class methods
void gendy::set_interpolation(int new_interpolation) {
	waveform.set_interpolation(new_interpolation);
}

//register the gendy class as a PD or Max object
FLEXT_NEW_DSP("gendy~", gendy)
