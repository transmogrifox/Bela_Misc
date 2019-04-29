#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "read_vi_trace.h"

vi_trace trace;

int main(void)
{
    const char* fname = "export_clipper_vi_curve.txt";
    load_vi_data(&trace, (char*) fname);

    printf("N\tAmp\tVolt\tAmpI\tVoltI\n");

    float dx = trace.di;
    float x = dx*0.5 + trace.minamp;

    for(int i = 0; i < trace.cnt; i++)
    {
        //printf("%d\t%f\t%f\t%f\n", i, trace.amp[i], trace.volt[i], vi_trace_interp(&trace, x));
        printf("%d\t%f\t%f\t%f\t%f\n", i, trace.amp[i], trace.volt[i], x, vi_trace_interp(&trace, x));
        //printf("%d\t%f\t%f\t%f\n", i, trace.amp[i], trace.volt[i], x);
        x += dx;
    }

    return 0;
}
