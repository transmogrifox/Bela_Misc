#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "biquad.h"

void
compute_filter_coeffs(int type, biquad_coeffs* cf, float fs, float f0, float Q)
{
    float w0 = 2.0*M_PI*f0/fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s/(2.0*Q);
    float a0, a1, a2;
    float b0, b1, b2;
    float g;


    switch(type){
        case LPF:
            b0 =  (1.0 - c)/2.0;
            b1 =   1.0- c;
            b2 =  (1.0 - c)/2.0;
            a0 =   1.0 + alpha;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha;
            break;

         case HPF:
            b0 =  (1.0 + c)/2.0;
            b1 =   -(1.0 + c);
            b2 =  (1.0 + c)/2.0;
            a0 =   1.0 + alpha;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha;
            break;

         case BPF:
            b0 =   Q*alpha;
            b1 =   0.0;
            b2 =  -Q*alpha;
            a0 =   1.0 + alpha;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha;
            break;
         case LPF1P:
            //1-pole high pass filter coefficients
            // H(z) = g * (1 - z^-1)/(1 - a1*z^-1)
            // Direct Form 1:
            //    h[n] = g * ( b0*x[n] - b1*x[n-1] ) - a1*y[n-1]
            // In below implementation gain is redistributed to the numerator:
            //    h[n] = gb0*x[n] - gb1*x[n-1] - a1*y[n-1]
            a0 = 1.0;
            a2 = 0.0;
            a1 = -expf(-w0);
            g = (1.0 + a1)/1.12; //0.12 zero improves RC filter emulation at higher freqs.
            b0 = g;
            b1 = 0.12*g;
            b2 = 0.0;
            break;
         case HPF1P:
            //1-pole high pass filter coefficients
            // H(z) = g * (1 - z^-1)/(1 - a1*z^-1)
            // Direct Form 1:
            //    h[n] = g * ( x[n] - x[n-1] ) - a1*y[n-1]
            // In below implementation gain is redistributed to the numerator:
            //    h[n] = g*x[n] - g*x[n-1] - a1*y[n-1]
            a0 = 1.0;
            a2 = 0.0;
            a1 = -expf(-w0);
            g = (1.0 - a1)*0.5;
            b0 = g;
            b1 = -g;
            b2 = 0.0;
            break;

        default:
            break;
    }

    b0 /=  a0;
    b1 /=  a0;
    b2 /=  a0;
    a1 /=  a0;
    a2 /=  a0;

    cf->b0 = b0;
    cf->b1 = b1;
    cf->b2 = b2;
    cf->a1 = -a1;  // filter implementation uses addition instead of subtraction
    cf->a2 = -a2;  // so "a" coefficients must be negated to compensate


}

void
biquad_reset_state_variables(biquad_coeffs* cf)
{
    cf->y1 = 0.0;
    cf->y2 = 0.0;
    cf->x1 = 0.0;
    cf->x2 = 0.0;    
}

biquad_coeffs*
make_biquad(int type, biquad_coeffs* cf, float fs, float f0, float Q)
{

    cf = (biquad_coeffs*) malloc(sizeof(biquad_coeffs));
    compute_filter_coeffs(type, cf, fs, f0, Q);
    biquad_reset_state_variables(cf);
    return cf;

}

float*
make_butterworth_coeffs(int order, float* coeffs)
{
    coeffs = (float*) malloc(sizeof(float)*order);

    int k = 1;
    int n = order;
    float fn = (float) n;

    //printf("N, Coeff\n", k, coeffs[k-1]);

    if(n%2 == 0) {
        for(k=1; k<=n/2; k++)
        {
            float fk = (float) k;
            coeffs[k-1] = 1.0/(-2.0*cos( (2.0*fk + fn -1)/(2.0*fn) * M_PI)); //1/x returns filter stage Q factor
            //printf("%d, %f\n", k, coeffs[k-1]);
        }
    } else { //odd
        for(k=1; k<=(n-1)/2; k++)
        {
            float fk = (float) k;
            coeffs[k-1] = 1.0/( -2.0*cos( (2.0*fk + fn -1)/(2.0*fn) * M_PI) ); //1/x returns filter stage Q factor
            //printf("%d, %f\n", k, coeffs[k-1]);
        }
    }

    return coeffs;
}

float
run_filter(float x, biquad_coeffs* cf)
{

    float y0 = cf->b0*x + cf->b1*cf->x1 + cf->b2*cf->x2
                        + cf->a1*cf->y1 + cf->a2*cf->y2;
    cf->x2 =cf->x1;
    cf->x1 = x;
    cf->y2 = cf->y1;
    cf->y1 = y0;
    return y0;

}

float
run_filter_one_pole(float x, biquad_coeffs* cf)
{

    float y0 = cf->b0*x + cf->b1*cf->x1
                        + cf->a1*cf->y1;
    cf->x1 = x;
    cf->y1 = y0;
    return y0;

}

//1-pole all-pass filter takes HPF as input.
// Nonlinear distortion such as found in a FET phaser can be applied
// by modulating the a1 coefficient with signal level.
float
run_APF_one_pole(float x, biquad_coeffs* cf)
{

    //first run high-pass filter
    float y0 = cf->b0*x + cf->b1*cf->x1
                        + cf->a1*cf->y1;
    cf->x1 = x;
    cf->y1 = y0;

    //Subtract
    return 2.0*y0 - x; //converts to APF function

}


float sqr(float x)
{
    return x*x;
}


// int main(int argc, char** argv)
// {


    //construct filters
    // biquad_coeffs* f;
    // f = make_biquad(BPF, f, 44100.0, 800.0, 10.0);


    // for(j=0; j<maxtime; j++)
    // {
        // if(j>startstep)
            // x0 = 1.0;
        // y0 = run_APF_one_pole(x0, f[4]);
        // printf("%f\t%f\n", ((float) j)/fs, y0);
    // }

    // for(j=0;j<5;j++)
    // {
        // free(f[j]);
    // }
    // free(q);

    // return 0;
// }
