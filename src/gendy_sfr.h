#include <list>

#include <flext.h>

//using namespace std;

#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define LOG_LEVEL LOG_DEBUG

extern void print_log(const char *msg, int level);
extern void print_log(const char *msg, int arg1, int level);
extern void print_log(const char *msg, unsigned int arg1, int level);
extern void print_log(const char *msg, float arg1, int level);

// define interpolation types
enum interpolation_t{ LINEAR, CUBIC, SPLINE, SINC };

// define center waveform shapes
enum waveshape_t { FLAT, SINE, SQUARE, TRIANGLE, SAWTOOTH };

// define types
typedef float gendydur_t;
typedef float gendyamp_t;
typedef float gendysamp_t;

// class defs
class breakpoint
{
	// store current breakpoint location
	// duration is in samples
	gendyamp_t amplitude;
	gendydur_t duration;

	// store the center point the breakpoint gravitates to
	gendyamp_t center_amp;
	gendydur_t center_dur, max_duration;

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
	void set_max_duration(gendydur_t new_max);
	gendydur_t get_duration();
	gendyamp_t get_amplitude();
	gendydur_t get_center_duration();
	gendyamp_t get_center_amplitude();
}; //end breakpoint class def

class gendy_waveform
{
	// buffer containing the current cycle of the waveform
	gendysamp_t *wave_samples;
	unsigned int wave_buffer_size;
	// keep track of how much of the waveform we've copied out
	unsigned int copy_index;
	// keep track of where we are in the waveform (in samples)
	gendydur_t phase;
	// average wavelength in samples. 	
	float average_wavelength;
	// the wavelength(in samples) of the current cycle of waveform. The wave
	// grows and shrinks cycle to cycle depending on the motion of the
	// breakpoints
	gendydur_t current_wavelength;
	// list of breakpoints
	std::list<breakpoint> breakpoint_list;
	// we store the first breakpoint of the next cycle for continuity
	breakpoint next_first;
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

#ifdef FLEXT_VERSION
	// buffer to copy to for waveform display
	flext::buffer *display_buf;
	// waveform display is written every display_rate cycles
	int display_rate;
	bool display;
#endif

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
	void set_avg_wavelength(float new_wavelength);
	void set_interpolation(interpolation_t new_interpolation);
	void set_waveshape(waveshape_t new_waveshape);
	void set_step_width(float new_width);
	void set_step_height(float new_height);
	void set_amplitude_pull(float new_pull);
	void set_duration_pull(float new_pull);
	void set_constrain_endpoints(bool constrain);
	unsigned int fill_buffer(float *buffer, unsigned int n);
#ifdef FLEXT_VERSION
	// display_toggle with no args flips state
	void display_toggle();
	void display_toggle(bool state);
	void set_display_rate(int rate);
	void set_display_buffer(flext::buffer *buf);
	void display_waveform();
#endif
}; //end gendy_waveform class def

// misc utility functions

// return a uniformly distributed double-precision float between 0 and 1
double randf();

// returns gaussian random variable with mu 0 and sigma 1
// From the GNU Scientific Library, src/randist/gauss.c, released under GPL
double gauss();

// returns nearest integer. X.5 always rounded to X+1, so it's non-symmetrical
int round(float num);
