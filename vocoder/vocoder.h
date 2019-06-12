#ifndef VOCODER_H
#define VOCODER_H

#include "envelope_detector.h"
#include "eq.h"
#include "fb_compressor.h"

typedef struct vocoder_t
{
    int N;
    eq_filters* filterbank_m;   // analysis
    eq_filters* filterbank_c;   // carrier
    envelope_detector** evd;
    float* envelope_m;
    float* envelope_raw;
    float* bank_gains;

    feedback_compressor* vcomp;
    float *envelope; // fb comp outputs envelope
    float *output;

    // User params
    float gate;
    float out_volume;
    float duck_atk;
    float duck_rls;
    float duck_sns;
    float duck_sv;
} vocoder;

vocoder*
make_vocoder(vocoder* vcd, float fs, int N, int bands, float fstart, float fstop);

void
vocoder_tick_n(vocoder* vcd, float *carrier, float *modulator);

#endif //VOCODER_H
