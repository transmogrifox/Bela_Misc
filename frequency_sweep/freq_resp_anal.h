
#ifndef FREQ_ANAL_H
#define FREQ_ANAL_H

#define FREQ_ANAL_DEFAULT_SIZE    500
#define FREQ_ANAL_DEFAULT_TIME    30    //seconds
#define FREQ_ANAL_DEFAULT_START   10
#define FREQ_ANAL_DEFAULT_STOP    500

typedef struct frequency_response_analyzer_t {
    //sampling frequency representations
    float fs, ifs, hfs;

    //User config
    float start_freq, fstart;   //frequency where sweep starts
    float stop_freq;    //end sweep
    int npoints;        //how many measurement points to return
    float sweep_time;   //How long a whole sweep cycle takes
    bool normalize;     //Should the final frequency response be normalized?
                        //true = normalize
                        //false = raw

    //Internal parameters
    int frq_bin_ptr;        //Which frequency step we're detecting
    float inst_freq;        //tells the oscillator its frequency
    float freq_rate_coeff;  //Sets how fast frequency sweep changes
    float pk_filter_coeff;  //Peak detector filter to reject noise

    //State Variables
    float s, c;  //oscillator sine and cosine parts
    float y1;  //pre-signal filter
    float pk_detector;
    bool sweep_finished;
    bool is_binning;        //spinlock to prevent getting spectrum while it is being copied to thread-safe buffers

    //Output
    float* output_buffer_w;     //internal working buffer
    float* output_buffer_f;     //Frequency indexes
    float* output_buffer_a;     //amplitude (linear)
    float* output_buffer_dB;    //magnitude (dB)

} frequency_response_analyzer;

frequency_response_analyzer* make_fra(frequency_response_analyzer* f, float fs);

void fra_tick_n(frequency_response_analyzer* f, float* rx, float* tx, int n);

int fra_get_spectrum(frequency_response_analyzer* f, float* frq_buf, float* mag_buf);
void fra_set_start_frq(frequency_response_analyzer* f, float fs);
void fra_set_stop_frq(frequency_response_analyzer* f, float fe);
void fra_set_n_points(frequency_response_analyzer* f, int n);
void fra_set_n_sweep_time(frequency_response_analyzer* f, float t);

#endif //FREQ_ANAL_H
