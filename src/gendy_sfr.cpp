// TODO:
// check constructors/destructors/copy constructors/assignment
// pd object constructor
// pd object destructor
// check DSP loop
// add license stuff to header/implementation files
// add max_duration to each breakpoint

breakpoint::breakpoint(unsigned int duration, float amplitude,
        unsigned int center_dur, float center_amp)
{
    this.duration = duration;
    this.amplitude = amplitude;
    this.center_dur = center_dur;
    this.center_amp = center_amp;
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
// durations are mirrored across 0 to become positive
void breakpoint::elastic_move(float h_step, float v_step, 
        float h_pull, float v_pull)
{
    long int new_duration;
    float new_amplitude;

    new_duration = duration + (int)((1 - h_pull) * h_step * gauss() +
        h_pull * (float)(center_dur - duration));
    // set mirror boundaries on new_duration
    do
    {
        if(new_duration > max_duration)
            new_duration = (2 * max_duration) - new_duration;
        if(new_duration < 0)
            new_duration = -new_duration;
    } while (new_duration < 0 || new_duration > max_duration);
    
    new_amplitude = amplitude + (1 - v_pull) * v_step * gauss() +
        v_pull * (center_amp - amplitude);
    // set mirror boundaries on new_amplitude
    do
    {
        if(new_amplitude > 1)
            new_amplitude = 2 - new_amplitude;
        if(new_amplitude < -1)
            new_amplitude = -2 - new_amplitude;
    } while (new_amplitude < -1 || new_amplitude > 1);

    duration = new_duration;
    amplitude = new_amplitude;
}

void breakpoint::set_duration(unsigned int new_duration)
{
    duration = new_duration;
}

void breakpoint::set_amplitude(float new_amplitude)
{
    amplitude = new_amplitude;
}

void breakpoint::set_position(unsigned int new_duration, float new_amplitude)
{
    //TODO: argument sanitization
    duration = new_duration;
    amplitude = new_amplitude;
}

// set_center
// accessor function to set the amplitude and duration of a breakpoint's
// center position

void breakpoint::set_center(unsigned int new_duration, float new_amplitude)
{
    //TODO: argument sanitization
    center_dur= new_duration;
    center_amp = new_amplitude;
}

void breakpoint::set_max_duration(unsigned int new_max)
{
    max_duration = new_max;
}

unsigned int breakpoint::get_duration()
{
    return duration;
}

float breakpoint::get_amplitude()
{
    return amplitude;
}

unsigned int breakpoint::get_center_duration()
{
    return center_dur;
}

float breakpoint::get_center_amplitude()
{
    return center_amp;
}

// gendy_waveform class constructor with all default arguments
gendy_waveform::gendy_waveform()
{
    //initialize variables to defaults
    step_width = 30;
    step_height = 0.1;
    duration_pull = 0.7;
    amplitude_pull = 0.4;
    constrain_endpoints = true;
    // set the average wavelength for 300 Hz at 44.1 kHz
    set_avg_wavelength(147);
    set_interpolation_type(LINEAR);
    set_waveshape(SINE);
    copy_index = 0;

    // start with a buffer that's 10 times as long as it needs to be for the
    // average wavelength
    wave_samples = new float[average_wavelength * 10];

    // start with a single breakpoint that spans the whole wavelength
    breakpoint initial(147,0,147,0);
    breakpoint_list.push_front(initial);
    next_first = new breakpoint(147,0,147,0);

    set_num_breakpoints(8);
    reset_breakpoints();
    move_breakpoints();
}

gendy_waveform::~gendy_waveform()
{
    delete[] wave_samples;
    delete next_first;
}

void gendy_waveform::set_num_breakpoints(unsigned int new_size)
{
    while(breakpoint_list.size() < (new_size))
        add_breakpoint();
    while(breakpoint_list.size() > (new_size))
        remove_breakpoint();
    center_breakpoints();
}

void gendy_waveform::set_avg_wavelength(unsigned int new_wavelength)
{
    average_wavelength = new_wavelength;
    center_breakpoints();
}

void gendy_waveform::set_interpolation_type(unsigned int new_interpolation)
{
    interpolation_type = new_interpolation;
}

/*
void gendy_waveform::set_interpolation_type(t_symbol *new_interpolation)
{
    if(strcmp(GetString(new_interpolation), "linear") == 0)
        interpolation_type = LINEAR;
    else
        post("So far only linear interpolation implemented.");
} */

void gendy_waveform::set_waveshape(unsigned int new_waveshape)
{
    waveshape = new_waveshape;
}

void gendy_waveform::set_step_width(float new_width)
{
    step_width = new_step_width;
}

void gendy_waveform::set_step_height(float new_height)
{
    step_height = new_step_height;
}

// set new positions for all the breakpoints and update current_wavelength
void gendy_waveform::move_breakpoints()
{
    list<breakpoint>::iterator i;
    unsigned int total_dur = 0;
    // first we copy in the previously calculated first breakpoint 
    breakpoint_list.begin()->set_duration(next_first->get_duration());
    breakpoint_list.begin()->set_amplitude(next_first->get_amplitude());
    total_dur += breakpoint_list.begin()->get_duration();
    // calculate new values for all the rest of the breakpoints
    for(i = ++breakpoint_list.begin(); i != breakpoint_list.end(); i++)
    {
        i->elastic_move(step_width, step_height, 
                duration_pull, amplitude_pull);
        total_dur += i->get_duration();
    }
    // calculate new position for the first breakpoint in the next cycle
    next_first->elastic_move(step_width, step_height, 
            duration_pull, amplitude_pull);
    current_wavelength = total_dur;
}

// fill the waveform buffer with one cycle of gendy waveform generated
// by interpolating the breakpoints
void gendy_waveform::generate_from_breakpoints()
{
    list<breakpoint>::iterator next = breakpoint_list.begin();

    if(interpolation_type == LINEAR)
    {
        unsigned int offset = 0;
        unsigned int current_dur, next_dur;
        float current_amp, next_amp;
        float slope;
        next_dur = next->duration;
        next_amp = next->amplitude;
        // for each breakpoint 
        while(++next != breakpoint_list.end())
        {
            // the previous loop's next values become the current ones
            current_dur = next_dur;
            current_amp = next_amp;
            next_dur = next->duration;
            next_amplitude = next->amplitude;
            slope = (next_amp - current_amp) / (next_dur - current_dur);
            for(unsigned int i = 0; i < current_dur; i++)
            {
                wave_samples[i+offset] = current_amp + slope * i;
            }
            offset += current_dur;
        }
        // connect up with the first breakpoint of the next cycle
        current_dur = next_dur;
        current_amp = next_amp;
        next_dur = next_first->duration;
        next_amp = next_first->amplitude;
        slope = (next_amp - current_amp) / (next_dur - current_dur);
        for(unsigned int i = 0; i < current_dur; i++)
        {
            wave_samples[i+offset] = current_amp + slope * i;
        }
    }
    else
        post("I'm lazy and have only implemented linear interpolation");
}

// find the biggest space between breakpoints and add a new one
void gendy_waveform::add_breakpoint()
{
    unsigned int new_duration; 
    float new_amplitude;

    unsigned int largest_space = 0;
    list<breakpoint>::iterator largest_space_position;
    list<breakpoint>::iterator i = breakpoint_list.begin();
    breakpoint new_breakpoint;

    // find the largest space
    while(i != breakpoint_list.end())
    {
        if((i->duration) > largest_space)
        {
            largest_space = i->duration;
            largest_space_position = i;
        }
        i++;
    } 

    // set the duration of the new breakpoint to be half the previous
    new_duration = i->get_duration() / 2;
    // halve the duration of the previous breakpoint
    i.set_duration(new_duration);
    // set the new amplitude to be the average of the 2 adjacent breakpoints
    // and move i to the next breakpoint for insertion
    if(i == --breakpoint_list.end())
    {
        new_amplitude = (i->get_amplitude() + next_first->get_amplitude()) 
            / 2;
        i++;
    }
    else
        new_amplitude = (i->get_amplitude() + (++i)->get_amplitude()) / 2;
    new_breakpoint.set_duration(new_duration);
    new_breakpoint.set_amplitude(new_amplitude);
    // insert the breakpoint in the list
    breakpoint_list.insert(i, new_breakpoint);
}

// remove the breakpoint closest to its neighbors. breakpoints centers
// should be reset after.
void gendy_waveform::remove_breakpoint()
{
    // start smallest_space to be the largest integer as defined in limits.h
    unsigned int smallest_space = UINT_MAX;
    unsigned int space;
    list<breakpoint>::iterator smallest_space_position;
    list<breakpoint>::iterator i = breakpoint_list.begin();

    // find the breakpoint furthest from the adjacent breakpoints
    while(i != --breakpoint_list.end())
    {
        if((space = i->duration + (++i)->duration) < smallest_space)
        {
            smallest_space = space;
            smallest_space_position = i;
        }
    } 
    // erase returns the list element following the erased one
    smallest_space_position = breakpoint_list.erase(smallest_space_position);
    (--smallest_space_position)->set_duration(smallest_space);
}

// center_breakpoints() calculates center positions for all the breakpoints
// TODO:
// sawtooth and triangle
void gendy_waveform::center_breakpoints()
{
    list<breakpoint>::iterator current;
    unsigned int i = 0;
    unsigned int new_dur;
    float new_amp;

    if(waveshape == SINE)
        for(current = breakpoint_list.begin(); 
                current != breakpoint_list.end(); current++)
        {
            // evenly distribute the breakpoints along the waveform
            new_dur = average_wavelength / num_breakpoints;
            // set amplitudes to match a single cycle of sine 
            new_amp = sin(2 * M_PI * i / num_breakpoints);
            current.set_center(new_dur, new_amp);
            current.set_max_duration(average_wavelength * 10 
                    / num_breakpoints);
            i++;
        }
    else if(waveshape == SQUARE)
        for(current = breakpoint_list.begin(); 
                current != breakpoint_list.end(); current++)
        {
            // evenly distribute the breakpoints along the waveform
            new_dur = average_wavelength / num_breakpoints;
            // set amplitudes to match a single cycle of square wave
            {
                if(((float)i / num_breakpoints) > 0.5)
                    new_amp = 1;
                else
                    new_amp = -1;
            }
            current.set_center(new_dur, new_amp);
            current.set_max_duration(average_wavelength * 10 
                    / num_breakpoints);
            i++;
        }
    // copy new center info to the next_first breakpoint
    next_first.set_center(breakpoint_list.begin()->get_center_duration(),
            breakpoint_list.begin()->get_center_amplitude());
}

// reset_breakpoints() sets all of the breakpoint positions to be the
// center positions
void gendy_waveform::reset_breakpoints()
{
    list<breakpoint>::iterator current;
    for(current = breakpoint_list.begin(); 
            current != breakpoint_list.end(); current++)
        current->set_position(current->get_center_duration(),
                current->get_center_amplitude());
    next_first->set_position(next_first->get_center_duration(),
            next_first->get_center_amplitude());
}

// copy as much of the wave data as possible into a buffer of size n,
// returns number of samples copied or BUFFULL if the buffer has been
// filled. The next time it is called the remainder of the waveform is
// copied
int gendy_waveform::get_wave_data(float *buffer, unsigned int n)
{
    int samples_copied = 0;
    // if the waveform has been completely copied out, generate a new one
    if(copy_index == current_wavelength)
    {
        move_breakpoints();
        generate_from_breakpoints();
        copy_index = 0;
    }
    // copy samples until the output buffer or the waveform buffer runs out
    while(samples_copied++ < n && copy_index < current_wavelength)
        *(buffer++) = wave_samples[copy_index++];
    if(samples_copied == n)
        return BUFFULL;
    else
        return samples_copied;
}

// return a uniformly distributed double-precision float between 0 and 1
double randf()
{
    return rand() / static_cast<double>(RAND_MAX);
}

// returns gaussian random variable with mu 0 and sigma 1
// From the GNU Scientific Library, src/randist/gauss.c

double gauss()
{
    double x, y, r2;
    do
    {
        /* choose x,y in uniform square (-1,-1) to (+1,+1) */ 
        x = -1 + 2 * randf();
        y = -1 + 2 * randf();

        /* see if it is in the unit circle */
        r2 = x * x + y * y;
    } while (r2 > 1.0 || r2 == 0);

    /* Box-Muller transform */
    return y * sqrt (-2.0 * log(r2) / r2);
}
