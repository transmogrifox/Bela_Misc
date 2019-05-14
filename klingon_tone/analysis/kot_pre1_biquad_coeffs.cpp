#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../iir_1pole.h"

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
// den   : Buquad denominator:  den[2]*s^2 + den[1]*s + den[0]
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

void s_biquad_to_z_biquad(float sgain, float fs_, float kz_, float* num, float* den)
{
    float kz = kz_;
    float fs = fs_;
    
    if(kz == 0.0) 
        kz = 2.0*fs;
    
    float kz2 = kz*kz;
        
    float B0 = num[2]*sgain;
    float B1 = num[1]*sgain;
    float B2 = num[0]*sgain;
    float A0 = den[2];
    float A1 = den[1];
    float A2 = den[0];
    
    num[2] = (B0*kz2 + B1*kz + B2)/(A0*kz2+A1*kz+A2);
    num[1] = (2.0*B2 - 2.0*B0*kz2)/(A0*kz2+A1*kz+A2);
    num[0] = (B0*kz2 - B1*kz + B2)/(A0*kz2+A1*kz+A2);

    den[2] = -(A0*kz2-A1*kz+A2)/(A0*kz2+A1*kz+A2);      // negated
    den[1] = -(2.0*A2-2.0*A0*kz2)/(A0*kz2+A1*kz+A2);    // negated
    den[0] = 0.0;  // should not be used
    
}

//
// King of Tone, Marshall Bluesbreaker, etc.
//  First pre-emphasis stage high-pass second-order filter response
//    (VIN-)-*--/\/\/\/---||---*---||---(Io->)--GND
//           |    R2      C2   |   C1
//           *-----/\/\/\/-----*
//                   R1
// Transfer function is Io(s)/Vin(s)
//
// Default usage example:
//   compute_s_biquad(r1, r2, c1, c2, num, den);
//
void compute_s_biquad(float r1, float r2, float c1, float c2, float* num, float* den)
{
    float ga = (r1+r2)/(r1*r2);
    float gs = 1.0/ga;
    float z0 = 1.0/(r2*c2);
    float p0 = 1.0/((r1+r2)*c2);
    
    num[0] = 0.0;
    num[1] = ga*p0;
    num[2] = ga;
    
    den[0] = p0/(c1*gs);
    den[1] = (c1*gs*z0+1.0)/(c1*gs);
    den[2] = 1.0;

}

const int sz = 2000;
int main(void)
{
    float frq[sz];
    float mag[sz];
    float phase[sz];
    
    iir_1p pre_emph;
    iir_1p* pre = &pre_emph;
    
    pre_emph.x1 = 0.0;
    pre_emph.x2 = 0.0;
    pre_emph.y1 = 0.0;
    pre_emph.y2 = 0.0;
    pre_emph.gain = 1.0;
    pre_emph.fs = 44100.0;
    
    
    float k = 1000.0;
    float n = 1e-9;
    
    float fs = pre_emph.fs;
    float sgain = 1.0;
    
    float r1 = 27*k;
    float r2 = 33*k;
    float c1 = 10*n;
    float c2 = 10*n;
    
    float num[3];
    float den[3];

    compute_s_biquad(r1, r2, c1, c2, num, den);
    
    printf("  B0 = %f\tB1 = %f\tB2 = %f\n", num[2], num[1], num[0]);
    printf("---------------------------------------------------------\n");
    printf("  A0 = %f\tA1 = %f\tA2 = %f\n\n", den[2], den[1], den[0]);
    
    s_biquad_to_z_biquad(sgain, fs, 0.0, num, den);
    
    printf("  zb0 = %f\tzb1 = %f\tzb2 = %f\n", num[2], num[1], num[0]);
    printf("---------------------------------------------------------\n");
    printf("  za0 = %f\tza1 = %f\tza2 = %f\n", den[2], den[1], den[0]);

    pre_emph.a1 = den[1];
    pre_emph.a2 = den[2];
    pre_emph.b0 = num[0];
    pre_emph.b1 = num[1];
    pre_emph.b2 = num[2];
    
    iir_get_response(pre, sz, 20.0, 20000.0, frq, mag, phase);
    
    for(int i = 0; i < sz; i++)
    {
        printf("%f\t%f\n", frq[i], mag[i]);
    }
    
    return 0;
}
