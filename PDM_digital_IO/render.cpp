/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
	Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
	Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include <Bela.h>
#include <cmath>
#include <stdlib.h>

#include "sine_oscillator.h"
#include "biquad.h"

// Sine oscillator struct
sinosc* pure_sine;
sinosc* lfo;

// Pulse-density modulator filters
biquad_coeffs* hpf1;
biquad_coeffs* hpf2;

biquad_coeffs* lpf1;
biquad_coeffs* lpf2;

// Pulse-density modulator output pin 
int gOutputPin = 5;

//
// Pulse Density Modulator Generation functions
//

// High frequncy noise generator (noise shaping)
//   Note: This does not need to be duplicated for every output channel.  This noise
//         signal can be fed into all of the pulse-density modulators
float hf_noise_gen(biquad_coeffs* hpf1, biquad_coeffs* hpf2)
{
    // Noise shaped for highest density at high frequencies
    return run_filter(run_filter( ((float) (rand()%65536))/65536.0, hpf1)/50.0, hpf2);
}

float pdm_tick(biquad_coeffs* lpf1, biquad_coeffs* lpf2, biquad_coeffs* hpf1, biquad_coeffs* hpf2, float x)
{
	// For more channels this should be refactored so hf_noise_gen() is executed only once per frame
    float input = x + hf_noise_gen(hpf1, hpf2);
    
    // The remainder of this would need to be duplicated for each output channel.
    float output = 0.0;
    if( (input - lpf1->y1) > 0.0)
        output = 1.0;
    run_filter(run_filter(output, lpf1), lpf2);
    return output;
    //return lpf2->y1;  // Just to evaluate the effect of external analog filtering
}

bool setup(BelaContext *context, void *userData)
{
	// Signal to be output on the PDM pin
	pure_sine = make_sine_oscillator(pure_sine, context->audioSampleRate, 150.0, 0.0, 0.5, 0.5);
	lfo = make_sine_oscillator(lfo, context->audioSampleRate, 0.25, 0.0, 800.0, 400.0);
	
	// PDM generator filters
    hpf1 = make_biquad(HPF, hpf1, context->audioSampleRate, 0.8*context->audioSampleRate/2.0, 10.0); //Noise shaping
    hpf2 = make_biquad(HPF, hpf2, context->audioSampleRate, 0.8*context->audioSampleRate/2.0, 10.0); //Noise shaping
    lpf1 = make_biquad(LPF, lpf1, context->audioSampleRate, 2000.0, 0.707); // Reconstruction
    lpf2 = make_biquad(LPF, lpf2, context->audioSampleRate, 2000.0, 0.707); // Reconstruction

	// Digital output pin from which to output the PDM 
	pinMode(context, 0, gOutputPin, OUTPUT); // Set gOutputPin as output
	
	// Just to let you know whether assumption of digitalFrames and audioFrames being the same number
	if(context->audioFrames != context->digitalFrames) 
		rt_printf("Digital frames not same as audio frames\n");
	else rt_printf("Digital Frames match Audio Frames\n");
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	bool stat = 0;
	for(unsigned int n = 0; n < context->digitalFrames; n++) 
	{
		// Intended output signal
		set_sine_oscillator_frequency(pure_sine, sine_oscillator_tick(lfo));
		
		// Modulate output signal onto PDM for output to 
		float out = pdm_tick(lpf1, lpf2, hpf1, hpf2, sine_oscillator_tick(pure_sine)) - 0.5;

		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) 
		{
			//audioWrite(context, n, channel, out); // Audio write just for proof of concept, but will be weird if digitalFrames != audioFrames
			
			// Float to bool interpretation
			if(out > 0.0)
			{
				stat = 1;
			}
			else
				stat = 0;
				
			// Write to specified digital I/O pin
			digitalWrite(context, n, gOutputPin, stat);
		}
	}

}

void cleanup(BelaContext *context, void *userData)
{
// This should be used to free memory used by filters and oscillators, but neglected for now
}

