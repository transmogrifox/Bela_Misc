#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define NPTS 1000
#define FS 44100

    FILE* outfile;

//
// Lazy RMS detector
//  Inputs:
//  sz = processing block size
//  x = input signal (typically AC waveform)
//  y = detector state variable (current RMS measurement)
//  k = integration time constant, typically (1/RC)*(1/Fs)
//
float
lazy_rms_detector_tick_debug(float *x, float *y, float k)
{
    float error = x[0]*x[0] - y[0]*y[0];  // error from y^2
    y[0] += k*error; // Forward Euler integration, resolves to sqrt(x2)
    return error;
}

void
lazy_rms_detector_tick(int sz, float *x, float *y, float k)
{
    int i = 0;
    for(i=0; i < sz; i++)
    {
        y[i] += k*(x[i]*x[i] - y[i]*y[i]); // Forward Euler integration, resolves to sqrt(x2)
    }
}

int main()
{
    outfile = fopen ("plot_gate.txt","w");
    
    int i;
    float fs = 44100.0;
    float k = 2000.0/(fs); // Integration period
    float x = 0.75;
    float s = 0.0;
    float tt = 0.0;
    float T = 1.0/fs;
    float p = 2.0*M_PI*1000.0;
    float y = 0.0;
    float error = 0.0;

    
    for(i=0; i < NPTS; i++)
    {
        if(i > 200)
            x = 1.0;
        if(i > 400)
            x = 0.05;
        if(i <= 200)
            x = 0.25;
        s = sinf(p*tt);
        tt += T;
        if((p*tt) >= 2.0*M_PI)
            tt = 0.0;
        x *= s;
        
        
        // EVALUATE steady-state error
        // error = lazy_rms_detector_tick_debug(&x, &y, k);        
        // fprintf(outfile, "%d\t%f\t%f\t%f\t%f\t%f\n", i, x, x*x, y, y*y, 20.0*log10(fabs(error)) );
        
        // EVALUATE typical usage
        lazy_rms_detector_tick(1, &x, &y, k);
        fprintf(outfile, "%d\t%f\t%f\t%f\t%f\n", i, x, x*x, y, y*y );
    }

    return 0;
}
