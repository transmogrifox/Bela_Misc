#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "read_vi_trace.h"

vi_trace trace;

int main(void)
{
    const char* fname = "export_clipper_vi_curve.txt";
    load_vi_data(&trace, (char*) fname);

    printf("Amp\tVolt\n");

    float dx = (trace.maxamp - trace.minamp)/500.0;
    float x = trace.minamp;

    for(int i = 0; i < 500; i++)
    {
        printf("%e\t%f\n", x, vi_trace_interp(&trace, x));
        x += dx;
    }

    return 0;
}
