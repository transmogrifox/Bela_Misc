
#ifndef BIQUAD_H
#define BIQUAD_H

#define LPF     0
#define HPF     1
#define BPF     2
#define HPF1P   3
#define LPF1P   4
#define WAH     5

typedef struct biquad_t {
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;

    float y1;
    float y2;
    float x1;
    float x2;

} biquad_coeffs;

biquad_coeffs*
make_biquad(int, biquad_coeffs*, float, float, float);
//make_biquad(int type, biquad_coeffs* cf, float fs, float f0, float Q);

void compute_filter_coeffs(int, biquad_coeffs*, float, float, float);

void biquad_reset_state_variables(biquad_coeffs* );

//run_filter(float x, biquad_coeffs* cf);
float run_filter(float, biquad_coeffs*);

#endif
