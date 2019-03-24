
#ifndef OVERDRIVE_H
#define OVERDRIVE_H

#define LPF1P   0
#define HPF1P   1

typedef struct iir_1p_t
{
    // Coefficients
    float a1;
    float b0;
    float b1;

    // State variables
    float x1;
    float y1;
} iir_1p;

typedef struct overdrive_t
{
    float fs;  // Sampling frequency
    unsigned int oversample;  // Oversampling rate
    unsigned int blksz;

    // User control settings
    float gain;   // Distortion amount -- 1.0 ... 1000.0
    float tone;   // Tone control -- 0.0 ... 1.0
    float level;  // Output level -- 0.0 ... 1.0
    bool bypass;

    // Processing buffers
    float *procbuf;

    // Pre and post emphasis EQ
    iir_1p pre_emph;
    iir_1p post_emph;
    iir_1p tone_lp;
    iir_1p tone_hp;

} overdrive;

// Allocate the overdrive struct and set default values
overdrive* make_overdrive(overdrive* od, unsigned int oversample, unsigned int bsz, float fs);
void overdrive_cleanup(overdrive* od);

// Set EQ parameters to non-default
// Usually for configuring the overdrive flavor at initialization,
// but these can be user-configurable in real-time
void od_set_cut_pre_emp(overdrive* od, float fc);
void od_set_cut_post_emp(overdrive* od, float fc);
void od_set_cut_tone_lp(overdrive* od, float fc);
void od_set_cut_tone_hp(overdrive* od, float fc);

// Typical real-time user-configurable parameters
void od_set_drive(overdrive* od, float drive_db);   // 0 dB to 45 dB
void od_set_tone(overdrive* od, float hf_level_db); // high pass boost/cut, +/- 12dB
void od_set_level(overdrive* od, float outlevel_db); // -40 dB to +0 dB
bool od_set_bypass(overdrive* od, bool bypass);

// Run the overdrive effect
void overdrive_tick(overdrive* od, float* x);

#endif //OVERDRIVE_H
