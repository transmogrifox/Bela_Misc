// 
// Biquad filterbank
//

#ifndef FILTERBANK_H
#define FILTERBANK_H

#define LPF   0
#define HPF   1
#define BPF   2
#define APF   3

typedef struct filterbank_t
{
    float fs;

    // Coefficients
    float den[3];  // Denominator coefficients y[n-m]
    float num[3];  // Numerator coefficients x[n-m]

    // State variables
    float x[2];
    float y[2];
} filterank;

void
compute_filter_coeffs_1p(filterbank* fb, unsigned int type, float fs, float f0);

// Get frequency response, phase and magnitude
void
filterbankd_iir_get_response(filterbank* fb, float n, float fstart, float fstop, float* frq, float* mag, float* phase);

//
// Execute the 1rst-order IIR difference equation
//

inline float
filterbank_tick_filter_1p(iir_1p* f, float x)
{
    f->y[1] = f->num[0]*x + f->num[1]*f->x[1] + f->den[1]*f->y[1];
    f->x[1] = x;
    return f->y[1];  
}

//
// Execute the 2nd-order IIR difference equation
//

inline float
filterbank_tick_filter_biquad(iir_1p* f, float x)
{
    f->y[0] = f->num[0]*x + f->num[1]*f->x[1] + f->num[2]*f->x[2] + \
                         f->den[1]*f->y[1] + f->den[2]*f->y[2];
    f->x[2] = f->x[1];
    f->x[1] = x;
    f->y[2] = f->y[1];
    f->y[1] = f->y[0];
    return f->y[0];
}

//
// Compute bilinear transform on s-domain biquad
//
// Input s-domain biquad coefficients + filter gain
// Replaces num/den arrays with z-domain biquad coefficients
// Computed via Bilinear Transform
//
// sgain : Gain of s-domain transfer function or system linear gains to be
//         distributed into z-domain coefficients
// fs_   : DSP system sampling rate
// kz_   : Frequency warping coefficient, or 0.0 for default
// num   : Biquad numerator:    num[2]*s^2 + num[1]*s + num[0]
// den   : Biquad denominator:  den[2]*s^2 + den[1]*s + den[0]
//
// den[1] and den[2] are negated for difference equation implementation in
// MAC hardware (all operations summation).
//
// den[0] is always set to 0 for the z-domain biquad form.
//
// The intended difference equation implementation is of the following format:
// y[n] =   x[n-2]*num[2] + x[n-1]*num[1] + x[n]*num[0]
//        + y[n-2]*den[2] + y[n-1]*den[1]
//
// Default usage example:
//   s_biquad_to_z_biquad(sgain, fs, 0.0, num, den);
//

void
s_biquad_to_z_biquad(float sgain, float fs_, float kz_, float* num, float* den);

//
// Create s-domain coefficients for a biquad filter
//  For DSP approximation, use s_biquad_to_z_biquad() to convert
//  This pair of functions provides a design process in which 
//  analog filter design methods can be exploited for IIR filter 
//  design.
//
//  The generic function will populate num and den with appropriate coefficients
//  for the specified filter type.  
//
void
s_biquad_generic_coeffs(int type, float w0, float Q, float* num, float* den);

// When using s_biquad_to_z_biquad method
void
iir_init_struct(iir_1p* cf, float fs, float *num, float *den);

#endif //FILTERBANK_H
