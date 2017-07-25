
#include <Bela.h>
#include <cmath>
#include <string.h>

#include <Scope.h>

#include "freq_resp_anal.h"

Scope scope;

float *rx0, *tx0;
float *rx1, *tx1;

frequency_response_analyzer* fra0;
frequency_response_analyzer* fra1;

//For file output
AuxiliaryTask WriteOutTaskCh0;
AuxiliaryTask WriteOutTaskCh1;
void *arg;
int gSweepnum0, gSweepnum1;

void commit_to_file(float *frq_buf, float *mag_buf, int N, int ch, int run)
{
	FILE *outfile = NULL;
	
	//filename
	char fname[100];
	memset(fname, '\0', 100);
	sprintf(fname, "plot_frequency_response_ch%d_run_%d.log", ch, run);
	printf("Ch%d Filename: %s\n", ch, fname);
	
	outfile = fopen (fname, "w");
	if(outfile == NULL)
	{
		printf("Error opening file %s for output.\n", fname);
		return;
	}

	for( int i = 0; i < N; i++)
	{
		fprintf(outfile, "%f\t%f\n", frq_buf[i], mag_buf[i]);
	}
	
	fclose(outfile);	
}

void write_frequency_response0(void*)
{
	printf("Writing out ch 0 frequency response to file.\n");
	float *frq_buf = (float*) malloc(sizeof(float)*fra0->npoints);
	float *mag_buf = (float*) malloc(sizeof(float)*fra0->npoints);
	
	fra_get_spectrum(fra0, frq_buf, mag_buf);
	commit_to_file(frq_buf, mag_buf, fra0->npoints, 0, gSweepnum0);
	
	free(frq_buf);
	free(mag_buf);
	
	gSweepnum0++;
}

void write_frequency_response1(void*)
{
	printf("Writing out ch 1 frequency response to file.\n");
	float *frq_buf = (float*) malloc(sizeof(float)*fra1->npoints);
	float *mag_buf = (float*) malloc(sizeof(float)*fra1->npoints);
	
	fra_get_spectrum(fra1, frq_buf, mag_buf);
	commit_to_file(frq_buf, mag_buf, fra1->npoints, 1, gSweepnum1);
	
	free(frq_buf);
	free(mag_buf);
	
	gSweepnum1++;
}

bool setup(BelaContext *context, void *userData)
{
	//Frequency response analyzer for each channel
	fra0 = make_fra(fra0, context->audioSampleRate);
	fra1 = make_fra(fra1, context->audioSampleRate);

	//RX and TX buffers
	rx0 = (float*) malloc(sizeof(float)*context->audioFrames);
	tx0 = (float*) malloc(sizeof(float)*context->audioFrames);
	rx1 = (float*) malloc(sizeof(float)*context->audioFrames);
	tx1 = (float*) malloc(sizeof(float)*context->audioFrames);
	
	for( int i = 0; i < context->audioFrames; i++)
	{
		rx0[i] = 0.0;
		tx0[i] = 0.0;
		rx1[i] = 0.0;
		tx1[i] = 0.0;
	}
	
	//
	// DEBUG (scope)
	//
	
	scope.setup(2, context->audioSampleRate);

	//File output
	WriteOutTaskCh0 = Bela_createAuxiliaryTask(&write_frequency_response0, 50, "bela-WFR0", arg);
	WriteOutTaskCh1 = Bela_createAuxiliaryTask(&write_frequency_response1, 50, "bela-WFR1", arg);
	gSweepnum1 = 0;
	gSweepnum0 = 0;

	return true;
}


void render(BelaContext *context, void *userData)
{

	for(unsigned int n = 0; n < context->audioFrames; n++) {

		rx0[n] = audioRead(context, n, 0);
		rx1[n] = audioRead(context, n, 1);

	}
	
	//tick analyzes rx path and overwrites new samples onto tx path
	fra_tick_n(fra0, rx0, tx0, context->audioFrames);
	fra_tick_n(fra1, rx1, tx1, context->audioFrames);

	for(unsigned int n = 0; n < context->audioFrames; n++) 
	{

		audioWrite(context, n, 0, tx0[n]);
		audioWrite(context, n, 1, tx1[n]);
		scope.log(tx0[n], rx0[n]);
		
		if(fra0->sweep_finished)
		{
			Bela_scheduleAuxiliaryTask(WriteOutTaskCh0);
			fra0->sweep_finished = false;
		}
		if(fra1->sweep_finished)
		{
			Bela_scheduleAuxiliaryTask(WriteOutTaskCh1);
			fra1->sweep_finished = false;
		}
		
	}

}


void cleanup(BelaContext *context, void *userData)
{

}



