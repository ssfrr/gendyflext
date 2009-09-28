#include <list>
#include <climits>
#include <ctime>
#include <cstdlib>
#include <cmath>

using namespace std;

// define interpolation types
enum { LINEAR, CUBIC, SPLINE, SINC };

// define center waveform shapes
enum { FLAT, SINE, SQUARE, TRIANGLE, SAWTOOTH };

// class defs
class breakpoint
{
	// store current breakpoint location
	float amplitude;
	unsigned int duration;

	// store the center point the breakpoint gravitates to
	float center_amp;
	unsigned int center_dur;
	unsigned int max_duration;

	public:
	breakpoint();
	breakpoint(unsigned int duration, float amplitude);
	breakpoint(unsigned int duration, float amplitude,
			unsigned int center_dur, float center_amp);
	void elastic_move(float h_step, float v_step, 
			float h_pull, float v_pull);
	void set_duration(unsigned int new_duration);
	void set_amplitude(float new_amplitude);
	void set_position(unsigned int new_duration, float new_amplitude);
	void set_center_duration(unsigned int new_duration);
	void set_center_amplitude(float new_amplitude);
	void set_center(unsigned int new_duration, float new_amplitude);
	void set_max_duration(unsigned int new_max);
	unsigned int get_duration();
	float get_amplitude();
	//TODO: do we need these accessor fuctions?
	unsigned int get_center_duration();
	float get_center_amplitude();
}; //end breakpoint class def

class gendy_waveform
{
	// buffer containing the current cycle of the waveform
	float *wave_samples;
	unsigned int wave_buffer_size;
	// average wavelength in samples. the wave grows and shrinks cycle to
	// cycle depending on the motion of the breakpoints
	unsigned int average_wavelength;
	// the wavelength(in samples) of the current cycle of waveform
	unsigned int current_wavelength;
	// list of breakpoints
	list<breakpoint> breakpoint_list;
	// we store the first breakpoint of the next cycle for continuity
	breakpoint next_first;
	// set the type of interpolation(see defines at top)
	unsigned int interpolation_type;
	// the waveshape that the breakpoints will gravitate to
	unsigned int waveshape;
	// constrain endpoints to 0
	bool constrain_endpoints;
	// std. dev. of the normal distribution that determines the
	// motion of the breakpoints. width is in samples.
	float step_width, step_height;
	// the extent to which the duration and amplitude are pulled to match
	// the waveshape. ranges from 0 to 1
	float duration_pull, amplitude_pull;
	bool debug;

	void move_breakpoints();
	void generate_from_breakpoints();
	void add_breakpoint();
	void remove_breakpoint();
	void center_breakpoints();
	void reset_breakpoints();

	public:
	gendy_waveform();
	~gendy_waveform();
	//gendy_waveform(float freq);
	void set_num_breakpoints(unsigned int new_size);
	void set_avg_wavelength(unsigned int new_wavelength);
	void set_interpolation(int new_interpolation);
	void set_waveshape(unsigned int new_waveshape);
	void set_step_width(float new_width);
	void set_step_height(float new_height);
	void set_amplitude_pull(float new_pull);
	void set_duration_pull(float new_pull);
	void set_constrain_endpoints(bool constrain);
	unsigned int get_wave_data(float *buffer, unsigned int n);


}; //end gendy_waveform class def

// misc utility functions

// return a uniformly distributed double-precision float between 0 and 1
double randf();

// returns gaussian random variable with mu 0 and sigma 1
// From the GNU Scientific Library, src/randist/gauss.c
double gauss();

// returns nearest integer. X.5 always rounded to X+1, so it's non-symmetrical
int round(float num);
