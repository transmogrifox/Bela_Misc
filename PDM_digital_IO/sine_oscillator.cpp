#include "sine_oscillator.h"
#include <stdlib.h>
#include <cmath>

void set_sine_oscillator_phase(sinosc* osc, float phase)
{
     //LFO
     osc->s = sin(osc->phase);
     osc->c = cos(osc->phase);
}

void set_sine_oscillator_amp(sinosc* osc, float amp)
{
     osc->amp = amp;
}

void set_sine_oscillator_offset(sinosc* osc, float offset)
{
    osc->offset = offset;
}

void set_sine_oscillator_frequency(sinosc* osc, float f)
{
	osc->k =  2.0*M_PI*f*osc->Ts;	
}

sinosc* make_sine_oscillator(sinosc* osc, float fsw, float frq, float phase, float offset, float amp)
{
	osc = (sinosc*) malloc(sizeof(sinosc));

	osc->Ts = 1.0/fsw;
	
	set_sine_oscillator_offset(osc, offset);
	set_sine_oscillator_amp(osc, amp);
	set_sine_oscillator_frequency(osc, frq);
	set_sine_oscillator_phase(osc, phase);
	
	return osc;
}

float sine_oscillator_tick(sinosc* osc)
{
    osc->s += osc->c*osc->k;
    osc->c -= osc->s*osc->k;
	return osc->s*osc->amp + osc->offset;
}
