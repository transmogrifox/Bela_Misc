#include <stdlib.h>
#include <stdio.h>

#include "iir_1pole.h"
#include "kot_tonestack.h"

#define SAMPLE_RATE 44100
#define FSTART      10
#define FSTOP       (SAMPLE_RATE/2)
#define FSTEP       10
#define FPOINTS     ((FSTOP - FSTART)/FSTEP)

kot_stack stack;
kot_stack* pstack;

int main(void)
{
    float frq[FPOINTS];
    float mag1[FPOINTS];
    float phase1[FPOINTS];
    float mag2[FPOINTS];
    float phase2[FPOINTS];
    float magout[FPOINTS];
    float phaseout[FPOINTS];

    pstack = &stack;
    kotstack_init(pstack, SAMPLE_RATE);
    kotstack_set_tone(pstack, 1.0);
    kotstack_set_boost(pstack,0.25);

    iir_get_response(&(pstack->st1), FPOINTS, FSTART, FSTOP, frq, mag1, phase1);
    iir_get_response(&(pstack->st2), FPOINTS, FSTART, FSTOP, frq, mag2, phase2);

    for( int i=0; i < FPOINTS; i++)
    {
        magout[i] = mag1[i] + mag2[i];
        phaseout[i] = phase1[i] + phase2[i];
        printf("%f\t%f\t%f\n", frq[i], magout[i], phaseout[i]);
    }

    return 0;
}
