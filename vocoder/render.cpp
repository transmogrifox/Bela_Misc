#include <Bela.h>
#include <Scope.h>
#include <stdlib.h>
#include <math.h>

#include "envelope_detector.h"
#include "eq.h"
#include "fb_compressor.h"
#include "vocoder.h"

Scope scope;

vocoder *vcd;
float *ch0, *ch1, *vox;
float *a, *b;

eq_coeffs* vox_lpf;

int gAudioFramesPerAnalogFrame;
int gAnalogVoxChannel = 6;     // Set ADC channel to be used for microphone

//DC removal filter parameters
typedef struct dc_remover_t {
	float fs;
	float ifs;
	
	//filter coefficient
	float a;
	
	//State Variables
	float x1, y1;
} dc_remover;

dc_remover *vox_dcr;

// Extract a single channel from BelaContext and format for flanger code input
void format_audio_buffer(BelaContext* context, float *outbuffer, int channel)
{
	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{
		// Read the audio input
		outbuffer[n] = audioRead(context, n, channel);
		
	}
}

bool setup(BelaContext *context, void *userData)
{
	float fs = context->audioSampleRate;
	int nsamps = context->audioFrames;
	
	ch0 = (float*) malloc(nsamps*sizeof(float));
	ch1 = (float*) malloc(nsamps*sizeof(float));
	vox = (float*) malloc(nsamps*sizeof(float));
	a =  (float*) malloc(nsamps*sizeof(float));
	b =  (float*) malloc(nsamps*sizeof(float));
	
	int bands = 16;
    float fstart = 150.0;
    float fstop = 4000.0;
    vcd = make_vocoder(vcd, fs, nsamps, bands,  fstart, fstop);
    
    if(context->analogFrames)
		gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	rt_printf("Audio Frames Per Analog Frame: %d", gAudioFramesPerAnalogFrame);
		
	vox_dcr = (dc_remover*) malloc(sizeof(dc_remover));
	vox_dcr->fs = context->audioSampleRate;
	vox_dcr->ifs = 1.0/context->audioSampleRate;
	vox_dcr->a = expf(-vox_dcr->ifs*2.0*M_PI*150.0); //set high pass cut-off
	vox_dcr->x1 = 0.0;
	vox_dcr->y1 = 0.0;
	
	vox_lpf = make_eq_band(LPF, vox_lpf, fs, 4000.0, 0.707, 1.0);
    
	scope.setup(3, context->audioSampleRate);
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	float x0 = 0.0;
	float y0 = 0.0;
	
	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{
		vox[n] = analogRead(context, n/gAudioFramesPerAnalogFrame, gAnalogVoxChannel);
		x0 = vox[n];
		a[n] = x0;
		y0 = vox_dcr->y1 + x0 - vox_dcr->x1;
		y0 *= vox_dcr->a;
		vox_dcr->x1 = vox[n];
		vox_dcr->y1 = y0;
		vox[n] = tick_eq_band(vox_lpf, y0);
		b[n] = vox[n];
	}

	format_audio_buffer(context, ch0, 0);
	format_audio_buffer(context, ch1, 1);
	
	vocoder_tick_n(vcd, ch1, vox);
	
	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{
		audioWrite(context, n, 0, ch0[n]);
		audioWrite(context, n, 1, ch1[n]);
		//scope.log(a[n], b[n], ch0[n]);
	}
	
}

void cleanup(BelaContext *context, void *userData)
{

}