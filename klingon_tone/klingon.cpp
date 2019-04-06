#include <math.h>
#include <stdlib.h>

#include "klingon.h"

//
// First order high-pass and low-pass filters.
//

void compute_filter_coeffs(iir_1p* cf, unsigned int type, float fs, float f0)
{
    float w0 = 2.0*M_PI*f0/fs;
    float a1;
    float b0, b1;
    float g = 1.0;  // This could be brought out into a user-configurable param

    switch(type){
    	 default:
         case LPF1P:
            //1-pole high pass filter coefficients
            // H(z) = g * (1 - z^-1)/(1 - a1*z^-1)
            // Direct Form 1:
            //    h[n] = g * ( b0*x[n] - b1*x[n-1] ) - a1*y[n-1]
            // In below implementation gain is redistributed to the numerator:
            //    h[n] = gb0*x[n] - gb1*x[n-1] - a1*y[n-1]
            a1 = -expf(-w0);
            g = (1.0 + a1)/1.12;
            b0 = g;
            b1 = 0.12*g; //0.12 zero improves RC filter emulation at higher freqs.
            break;
         case HPF1P:
            //1-pole high pass filter coefficients
            // H(z) = g * (1 - z^-1)/(1 - a1*z^-1)
            // Direct Form 1:
            //    h[n] = g * ( b0*x[n] -  b1*x[n-1] ) - a1*y[n-1]
            // In below implementation gain is redistributed to the numerator:
            //    h[n] = g*x[n] - g*x[n-1] - a1*y[n-1]
            a1 = -expf(-w0);
            g = (1.0 - a1)*0.5;
            b0 = g;
            b1 = -g;
            break;

    }


    cf->b0 = b0;
    cf->b1 = b1;
    cf->a1 = -a1;  // filter implementation uses addition instead of subtraction

    cf->y1 = 0.0;
    cf->x1 = 0.0;

}

//
// Execute the 1rst-order IIR difference equation
//

inline float tick_filter_1p(iir_1p* f, float x)
{
    f->y1 = f->b0*x + f->b1*f->x1 + f->a1*f->y1;
    f->x1 = x;
    return f->y1;
}

// Allocate the klingon struct and set default values
klingon* make_klingon(klingon* kot, unsigned int oversample, unsigned int bsz, float fs)
{
    kot = (klingon*) malloc(sizeof(klingon));
    kot->procbuf = (float*) malloc(sizeof(float)*bsz*oversample);

    for(int i=0; i<bsz; i++)
    {
        kot->procbuf[i] = 0.0;
    }
    kot->xn1 = 0.0;
    kot->xc1 = 0.0;

    kot->blksz = bsz;
    kot->oversample = oversample;
    kot->fs = fs;
    kot->clipper_fs = ((float) oversample)*fs;
    kot->inverse_oversample_float = 1.0/((float) oversample);

    // Set defaults
    kot->gain = 30.0;
    kot->tone = 0.5;
    kot->level = 0.5;
    kot->bypass = true;

    // Setup EQ stages

    // Anti-aliasing filter:
    //   1/10th fs assuming 44.1k rate -- increasing filter order will help
    //   down-play any aliasing artefacts, but increases CPU usage
    compute_filter_coeffs(&(kot->anti_alias), LPF1P, kot->clipper_fs, 4410.0);

    // First gain stage mids-boost filters
    compute_filter_coeffs(&(kot->pre_emph482), HPF1P, kot->fs, 482.29);
    compute_filter_coeffs(&(kot->pre_emph589), HPF1P, kot->fs, 589.46);

    // Second gain stage low-cut filter
    compute_filter_coeffs(&(kot->pre_emph589), HPF1P, kot->fs, 159.15);

    // First gain stage HF reject filter, hard-coded to setting at max gain
    compute_filter_coeffs(&(kot->post_emph), LPF1P, kot->clipper_fs, 14500.0);

    //  High-cut tone control -- Currently assumes second BOOST control
    //  is cranked all the way up.
    compute_filter_coeffs(&(kot->tone_lp), LPF1P, kot->fs, 636.0);

    // First stage gains
    // (g482*pre_emph482(x) + g589*pre_emph589(x))*gain + x
    // Resistor values * 1/1000 since it is relative and the 1k cancels
    kot->g482 = 1.0/33.0;  //1/33k resistor ratio
    kot->g589 = 1.0/27.0;  //1/27k resistor ratio
                          // 1rst stage gain will be 10 to 110
    //Second stage gain
    kot->g159 = 220.0/10.0;  // 220k/10k
    kot->gclip = 6.8/10.0;   // 6.8k/10k

    return kot;
}

void klingon_cleanup(klingon* kot)
{
    free(kot->procbuf);
    free(kot);
}

inline float sqr(float x)
{
    return x*x;
}

//
// Quadratic clipping function:
//   Linear between nthrs and thrs,
//   uses x - a*x^2 type of function above threshold
//

float thrs = 0.8;
float nthrs = -0.72;
float f=1.25;

void clipper_tick(klingon* kot, int N, float* x, float* clean)
{
	float xn = 0.0;
	float dx = 0.0;
	float dc = 0.0;
	float delta = 0.0;
	float deltac = 0.0;

    for(int i=0; i<N; i++)
    {
    	// Compute deltas for linear interpolation (upsampling)
    	dx = (x[i] - kot->xn1)*kot->inverse_oversample_float;
    	dc = (clean[i] - kot->xc1)*kot->inverse_oversample_float;
    	kot->xn1 = x[i];
    	kot->xc1 = clean[i];

    	// Run clipping function at higher sample rate
    	for(int n = 0; n < kot->oversample; n++)
    	{
    		xn = x[i] + delta; // Linear interpolation up-sampling
    		clean[i] = clean[i] + deltac; // Upsample clean signal for mix
    		delta += dx;
    		deltac += dc;
	        //Hard limiting
	        if(xn >= 1.2) xn = 1.2;
	        if(xn <= -1.12) xn = -1.12;

	        //Soft clipping
	        if(xn > thrs){
	            xn -= f*sqr(xn - thrs);
	        }
	        if(xn < nthrs){
	            xn += f*sqr(xn - nthrs);
	        }

	        // Pre-filter for down-sampling
	        // Run de-emphasis and anti-aliasing filters
	        xn = tick_filter_1p(&(kot->post_emph), (clean[i] + 2.0*xn));
	        xn = tick_filter_1p(&(kot->anti_alias), xn);
    	}

    	// Reset linear interpolator state variables
	    delta = 0.0;
	    deltac = 0.0;

	    // Zero-order hold downsampling assumes de-emphasis filter and
        // anti-aliasing filters sufficiently reject
        // harmonics > 1/2 base sample rate
        x[i] = xn;
    }
}

// Typical real-time user-configurable parameters
void kot_set_drive(klingon* kot, float drive_db)   // 0 dB to 45 dB
{
    float drv = drive_db;

    if (drv < 0.0)
        drv = 0.0;
    else if(drv > 45.0)
        drv = 45.0;

    // Convert gain given in dB to absolute value
    drv = powf(10.0,drv/20.0);

    // Work backward through gain stages to get pot value adjustment
    float gl = 33.0*27.0/(33.0 + 27.0);
    float gb = 22.0;

    // pot setting required to get requested gain from all
    // cascaded gain stages
    kot->gain =  (drv/gb - 1.0)*gl;
}

void kot_set_tone(klingon* kot, float hf_level_db) // high pass cut
{
    float tone = hf_level_db;

    // 60 dB far below RC roll-off
    if (tone < -60.0)
        tone = -60.0;
    else if (tone > 0.0)
        tone = 0.0;

    kot->tone = powf(10.0, tone/20.0);
}

void kot_set_level(klingon* kot, float outlevel_db) // -40 dB to +0 dB
{
    float vol = outlevel_db;

    if (vol < -40.0)
        vol = -40.0;
    if (vol > 0.0)
        vol = 0.0;
    kot->level = powf(10.0, vol/20.0);
}

bool kot_set_bypass(klingon* kot, bool bypass)
{

	if(!bypass)
	{
		if(kot->bypass)
			kot->bypass = false;
		else
            kot->bypass = true;
	}
	else
	{
		kot->bypass = true;
	}

	return kot->bypass;
}

//
// Run the klingon effect
//

void klingon_tick(klingon* kot, float* x)
{
    unsigned int n = kot->blksz;

    if(kot->bypass)
        return;

    // Run pre-emphasis filters
    for(int i = 0; i<n; i++)
    {
        // First gain stage frequency response
        kot->procbuf[i] = tick_filter_1p(&(kot->pre_emph589), kot->g589*x[i]);
        kot->procbuf[i] += tick_filter_1p(&(kot->pre_emph482), kot->g482*x[i]);
        kot->procbuf[i] *= kot->gain;
        kot->procbuf[i] += x[i];

        // Feed resistor and cap for the second gain stage, and gain
        kot->procbuf[i] += tick_filter_1p(&(kot->pre_emph159), kot->g159*x[i]);
        x[i] *= kot->gclip;  // This stores the clean portion of the mix
    }

    // Clipping applied with the higher head-room path (soft overdrive)
    clipper_tick(kot, n, kot->procbuf, x);  // Quadratic function: x - a*x^2
    // TODO:
    /*
     *  Second hard-clipping stage and associatede mode-select
     *  stomp-switch not yet implemented
     */

    // Output level and tone control
    for(int i = 0; i<n; i++)
    {
        // First-stage tonestack
        x[i] = kot->tone*kot->procbuf[i] + \
             (1.0 - kot->tone)*tick_filter_1p(&(kot->tone_lp), kot->procbuf[i]);

        // TODO:
        /* ... Full second-order tonestack not yet implemented... */

        // Output Volume
        x[i] *= kot->level;
    }
}
