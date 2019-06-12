#include <math.h>
#include <math_neon.h>
#include <stdio.h>
#include <stdlib.h>

#include "vocoder.h"

vocoder*
make_vocoder(vocoder* vcd, float fs, int N, int bands, float fstart, float fstop)
{
    int i;
    vcd = (vocoder*) malloc(sizeof(vocoder));

    vcd->N = N;
    vcd->filterbank_m = make_filterbank(vcd->filterbank_m, bands, fstart, fstop, fs);
    vcd->filterbank_c = make_filterbank(vcd->filterbank_c, bands, fstart, fstop, fs);
    vcd->evd = (envelope_detector**) malloc(bands*sizeof(envelope_detector*));
    vcd->vcomp = make_feedback_compressor(vcd->vcomp, fs, N);
    vcd->envelope = (float*) malloc(N*sizeof(float));
    vcd->output = (float*) malloc(N*sizeof(float));
    vcd->envelope_m = (float*) malloc(N*sizeof(float));
    vcd->envelope_raw = (float*) malloc(N*sizeof(float));
    vcd->bank_gains = (float*) malloc(bands*sizeof(float));

    for(i=0; i < N; i++)
    {
        vcd->envelope[i] = 0.0;
        vcd->envelope_m[i] = 0.0;
        vcd->envelope_raw[i] = 0.0;
        vcd->output[i] = 0.0;
    }

	float fc = 0.0;
	float tatk = 0.0;
	float trls = 0.0;
    for(i = 0; i < bands; i++)
    {
    	// Experimental: set attack/release times according to center frequency
    	fc = eq_get_filterbank_f0(vcd->filterbank_m, i);
    	tatk = 1.0*1000.0/fc;
		if(tatk < 2.5)
			tatk = 2.5;
		trls = 5.0*tatk;

        vcd->evd[i] = make_envelope_detector(vcd->evd[i], fs, N);
        envelope_detector_set_pkhold(vcd->evd[i], 250.0/fc); // round-robin sampling time
        envelope_detector_set_attack(vcd->evd[i], tatk);
        envelope_detector_set_release(vcd->evd[i], trls);
        vcd->bank_gains[i] = 1.0;
    }
    
    // Set low-pass band 
    i = 0;
    envelope_detector_set_pkhold(vcd->evd[i], 10.0);
    envelope_detector_set_attack(vcd->evd[i], 50.0);
    envelope_detector_set_release(vcd->evd[i], 200.0);
    // Set high-pass band
    i = bands - 1;
    envelope_detector_set_pkhold(vcd->evd[i], 1.0);
    envelope_detector_set_attack(vcd->evd[i], 20.0);
    envelope_detector_set_release(vcd->evd[i], 50.0);
    
    // Configure gate and output volume
    vcd->gate = powf(10.0, -60.0/20); // Gate below this thresh
    vcd->out_volume = 0.5;
    
    // Pre-configure defaults for vox channel dynamic range compressor
    feedback_compressor_set_threshold(vcd->vcomp, -12.0);
    feedback_compressor_set_ratio(vcd->vcomp, 2.0);
    feedback_compressor_set_attack(vcd->vcomp, 25.0);
    feedback_compressor_set_release(vcd->vcomp, 50.0);
    feedback_compressor_set_out_gain(vcd->vcomp, 0.0);
    feedback_compressor_set_mix(vcd->vcomp, 1.0);
    feedback_compressor_set_knee(vcd->vcomp, true);
    feedback_compressor_set_transfer_function(vcd->vcomp, true);
    feedback_compressor_set_bypass(vcd->vcomp, true);
    feedback_compressor_set_bypass(vcd->vcomp, false);
    
    // Ducking attack & release times
    vcd->duck_atk = expf(-1.0/(fs*0.005));
    vcd->duck_rls = expf(-1.0/(fs*0.025));
    vcd->duck_sns = -9.22; //e^-9.22 ~ -80dB, signal 
    vcd->duck_sv = 0.0;
    
    return vcd;
}

//
// carrier <-- typically noise or instrument
// modulator <-- typically voice (microphone channel)
// 
void
vocoder_tick_n(vocoder* vcd, float *carrier, float *modulator)
{
    int i = 0;
    int j = 0;
    float chgain = 0.0;
    float duck =  0.0;

    for(i=0; i < vcd->N; i++)
    {
        vcd->output[i] = 0.0;
    }

    // Run compressor on modulator (voice) channel
    feedback_compressor_tick_n(vcd->vcomp, modulator, vcd->envelope);

    // Process each filter band
    for(j=0; j < vcd->filterbank_m->nbands; j++)
    {
        // First run signal analysis on the modulator channel
        for(i=0; i < vcd->N; i++)
        {
            // Not envelope yet, but using memory allocated and envelope detector
            // will overwrite this buffer with envelope data
            vcd->envelope_m[i] = tick_eq_band(vcd->filterbank_m->band[j], modulator[i]);
        }
        // Get envelope from the modulator
        envelope_detector_tick_n(vcd->evd[j], vcd->envelope_m, vcd->envelope_raw);  //envelope_raw unused, but dummy memory to dump envelope without ballistics

        // Run filterbank on carrier, apply envelope from modulator
        for(i=0; i < vcd->N; i++)
        {
            // Apply gate and channel gains
            // TODO: Implement gate with natural release rather than hard cut-off
            //if(vcd->envelope[i] > vcd->gate) // Evaluate gate with incoming signal raw detected envelope
                chgain = vcd->envelope_m[i]*vcd->bank_gains[j];
            //else
              //  chgain = 0.0;

            // Sum processed filterbank channel to output
            vcd->output[i] += chgain*tick_eq_band(vcd->filterbank_c->band[j], carrier[i]);
        }
    }

    // Overwrite carrier array with processed output
    for(i=0; i < vcd->N; i++)
    {
    	duck = 10.0*vcd->envelope[i];  //-20dB pick-up on modulator envelope

    	if(duck > vcd->duck_sv) //Attack
            vcd->duck_sv = duck + (vcd->duck_sv - duck)*vcd->duck_atk;
        else //Release
            vcd->duck_sv = duck + (vcd->duck_sv - duck)*vcd->duck_rls;

        duck = expf_neon(vcd->duck_sns*vcd->duck_sv);
		
		// TODO: Make ducking user configurable, enable/disable, sensitivity, atk, rls
		// Enable ducking with this line
        carrier[i] = vcd->output[i]*vcd->out_volume*(1.0 - duck) + duck*carrier[i];
        
        // No ducking with this line.
        //carrier[i] = vcd->output[i]*vcd->out_volume;
    }


}
