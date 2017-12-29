#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DUTY                            50
#define OUTPUT_SAMPLE_RATE              22050
#define NPTS                            OUTPUT_SAMPLE_RATE*2

int main()
{
    FILE* outfile;
    outfile = fopen ("plot_stair_step.txt","w");
    size_t i;
    double x;
    
    double dt = 1.0/OUTPUT_SAMPLE_RATE;
    double dt0 = dt - dt/DUTY;
    double dt1 = dt - dt0;
    
    double t = 0.0;
    double T = 1.0/OUTPUT_SAMPLE_RATE;
    double f0 = 2500.0;
    double w0 = 2.0*M_PI*f0;

    //fprintf(outfile, "Time Vadc\r\n");

    for(i=0; i < NPTS/2; i++)
    {
        x = cos(w0*t);
        fprintf(outfile, "%.12lf %lf\r\n", t, x);
        t += dt0;
        fprintf(outfile, "%lf %lf\r\n", t, x);
        t += dt1;
    }

    return 0;
}
