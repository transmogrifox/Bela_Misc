#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "iir_1pole.h"

#define SAMPLE_RATE 44100
#define FSTART      10
#define FSTOP       (SAMPLE_RATE/2)
#define FSTEP       10
#define FPOINTS     ((FSTOP - FSTART)/FSTEP)

iir_1p* bpf0;
iir_1p* bpf1;

int main(void)
{
    float frq[FPOINTS];
    float mag1[FPOINTS];
    float phase1[FPOINTS];
    float mag2[FPOINTS];
    float phase2[FPOINTS];
    float magout[FPOINTS];
    float phaseout[FPOINTS];

    float num0[3];
    float den0[3];
    float num1[3];
    float den1[3];

    bpf0 = (iir_1p*) malloc(sizeof(iir_1p));
    bpf1 = (iir_1p*) malloc(sizeof(iir_1p));

    float m = powf(2.0, 1.0/6.0);
    float f0 = 500.0;
    float f1 = m*f0;
    //float m=powf(2.0,((logf(f1/f0)/logf(2.0f))/(nb-1.0)));
    float Q = 0.5 * (m + 1.0)/ (m - 1.0);
    float w0 = 2.0*M_PI*f0;
    float k = w0/tanf(w0/(2.0*SAMPLE_RATE)); //frequency warping

    // First stagger-tuned filter
    s_biquad_bandpass_coeffs(w0, Q, num0, den0);
    s_biquad_to_z_biquad(1.0, SAMPLE_RATE, k,  num0, den0);
    iir_init_struct(bpf0, SAMPLE_RATE, num0, den0);

    // Second stagger-tuned filter
    w0 *= m;
    k = w0/tanf(w0/(2.0*SAMPLE_RATE)); //frequency warping
    s_biquad_bandpass_coeffs(w0, Q, num1, den1);
    s_biquad_to_z_biquad(1.0, SAMPLE_RATE, k,  num1, den1);
    iir_init_struct(bpf1, SAMPLE_RATE, num1, den1);

    iir_get_response(bpf0, FPOINTS, FSTART, FSTOP, frq, mag1, phase1);
    iir_get_response(bpf1, FPOINTS, FSTART, FSTOP, frq, mag2, phase2);
    //return 1;

    for( int i=0; i < FPOINTS; i++)
    {
        magout[i] = mag1[i]+mag2[i];
        phaseout[i] = phase1[i] + phase2[i];
        printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\n", frq[i], magout[i], mag1[i], mag2[i], phaseout[i], phase1[i], phase2[i]);
    }

    return 0;
}
