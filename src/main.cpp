// gendy~ - an implementation of Iannis Xenakis' Dynamic
// Stochastic Synthesis as an object for Pure Data.
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
class gendy: 
	public flext_dsp
{
    // flext macro magic
	FLEXT_HEADER_S(gendy, flext_dsp, class_setup)

	public:
        gendy();	
	
	protected:
		// here we declare the virtual DSP function
		virtual void m_signal(int n, float *const *in, float *const *out);
	private:	
		gendy_waveform *waveform;
        float leftover_waveform[];
        int leftover_length;
        
        // register the callbacks
		FLEXT_CALLBACK_F(set_frequency)
        FLEXT_CALLBACK_I(set_num_breakpoints)
        FLEXT_CALLBACK_F(set_h_step)
        FLEXT_CALLBACK_F(set_v_step)
        FLEXT_CALLBACK_F(set_h_pull)
        FLEXT_CALLBACK_F(set_v_pull)
        FLEXT_CALLBACK_I(set_interpolation)

        void set_frequency(float new_freq);
        void set_num_breakpoints(float num_breakpoints);
        void set_h_step(float new_stepsize);
        void set_v_step(float new_stepsize);
        void set_h_pull(float new_pull);
        void set_v_pull(float new_pull);
        void set_interpolation(int new_interpolation);

        static void class_setup(t_classid thisclass);
}; // end of class declaration for gendy


// define the object class constructor(run at each gendy object creation)
gendy::gendy()
{
    // TODO: write constructor
    AddInAnything("control input");    // control input
    AddOutSignal("audio out");          // audio output
    
    // We're done constructing:
    post("-- gendy~ by Spencer Russell ---");
    
} // end of constructor

void gendy::class_setup(t_classid thisclass)
{
    FLEXT_CADDMETHOD(0,set_frequency);
    FLEXT_CADDMETHOD_(thisclass, 0, "breakpoints", set_num_breakpoints);
    FLEXT_CADDMETHOD_(thisclass, 0, "h_step", set_h_step);
    FLEXT_CADDMETHOD_(thisclass, 0, "v_step", set_v_step);
    FLEXT_CADDMETHOD_(thisclass, 0, "h_pull", set_h_pull);
    FLEXT_CADDMETHOD_(thisclass, 0, "v_pull", set_v_pull);
    FLEXT_CADDMETHOD_(thisclass, 0, "interpolation", set_interpolation);
}

// Now we define our DSP function. It gets this arguments:
// 
// int n: length of signal vector. Loop over this for your signal processing.
// float *const *in, float *const *out: 
//          These are arrays of the signals in the objects signal inlets rsp.
//          oulets. We come to that later inside the function.

void signal1::m_signal(int n, float *const *in, float *const *out)
{
	
    // outs points to the output signal vector
	float *outs = out[0];
    int filled_index = 0;
    int retreived_samples = 0;
	
    // keep grabbing cycles of waveform until the output buffer is full
	while (retreived_samples != BUFFULL)
	{
	    retreived_samples = get_wave_data(&outs[filled_index], n-filled_index);
        filled_index += retreived_samples;
	}
}  // end m_signal

//register the gendy class as a PD or Max object
FLEXT_NEW_DSP("gendy~", gendy)
