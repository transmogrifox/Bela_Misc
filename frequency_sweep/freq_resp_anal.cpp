
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "freq_resp_anal.h"

frequency_response_analyzer* make_fra(frequency_response_analyzer* f, float fs)
{
    f = (frequency_response_analyzer*) malloc(sizeof(frequency_response_analyzer));
    f->output_buffer_f = (float*) malloc(sizeof(float)*(FREQ_ANAL_DEFAULT_SIZE + 1));
    f->output_buffer_a = (float*) malloc(sizeof(float)*FREQ_ANAL_DEFAULT_SIZE);
    f->output_buffer_w = (float*) malloc(sizeof(float)*FREQ_ANAL_DEFAULT_SIZE);
    f->output_buffer_dB = (float*) malloc(sizeof(float)*FREQ_ANAL_DEFAULT_SIZE);

    //System operating conditions
    f->fs = fs;
    f->ifs = 1.0/fs;
    f->hfs = 0.5*fs;

    //User options
    f->start_freq = FREQ_ANAL_DEFAULT_START;
    f->stop_freq = FREQ_ANAL_DEFAULT_STOP;
    f->npoints = FREQ_ANAL_DEFAULT_SIZE;
    f->sweep_time = FREQ_ANAL_DEFAULT_TIME;
    f->normalize = true;

    //Internal parameters
    float step_rate = expf( logf(f->stop_freq/f->start_freq) / ((float) (f->npoints - 1) ));
    f->inst_freq = FREQ_ANAL_DEFAULT_START / step_rate;  //it will begin one interval below to stabilize steady state before measurement
    f->pk_filter_coeff = 1.0 - expf( -20.0 * M_PI * f->inst_freq * f->ifs ); //noise filter set a decade above instantaneous frequency
    f->freq_rate_coeff = expf( logf(f->stop_freq/f->start_freq) / (f->sweep_time*f->fs));
    f->fstart = f->inst_freq;
    f->frq_bin_ptr = 1;

    //State Variables
    f->pk_detector = 0.0;
    f->sweep_finished = false;
    f->s = 0.0;
    f->c = 1.0;

    float frq = f->start_freq;
    for(int i = 0; i < f->npoints; i++)
    {
        f->output_buffer_f[i] = frq;
        f->output_buffer_w[i] = 0.0;
        f->output_buffer_a[i] = 0.0;
        f->output_buffer_dB[i] = 0.0;
        frq *= step_rate;
    }
    f->output_buffer_f[f->npoints -1] = f->stop_freq;
    f->output_buffer_f[f->npoints] = frq;

    return f;
}

void fra_tick_n(frequency_response_analyzer* f, float* rx, float* tx, int n)
{
    float fosc = 0.0;
    float xi;
    for(int i = 0; i < n; i++)
    {
        //Excitation Signal Generator
        fosc = 2.0*M_PI*f->inst_freq*f->ifs;
        f->s = f->s + fosc * f->c;
        f->c = f->c - fosc * f->s;
        tx[i] = f->s;
        f->inst_freq *= f->freq_rate_coeff;

        //Detector
        xi = rx[i];
        f->y1 = f->y1 + f->pk_filter_coeff * (xi - f->y1);
        xi = f->y1;
        xi =  fabs(xi);

        if(f->inst_freq < f->start_freq)
        {
            f->pk_detector = 0.0;
        } else
        {
            if(xi > f->pk_detector)
                f->pk_detector = xi;
        }
        //increase filter cut-off continuously
        f->pk_filter_coeff = 1.0 - expf( -20.0 * M_PI * f->inst_freq * f->ifs );

        //Binning
        if(f->inst_freq < f->output_buffer_f[f->frq_bin_ptr])
        {
            f->output_buffer_w[f->frq_bin_ptr - 1] = f->pk_detector;
        } else
        {
            f->frq_bin_ptr += 1;
            f->pk_detector = 0.0;  //reset peak detector for next sweep segment
            if(f->frq_bin_ptr > f->npoints)
            {
                f->frq_bin_ptr = 1;
                f->inst_freq = f->fstart;
                f->s = 0.0;
                f->c = 1.0;
                f->sweep_finished = true;
                //copy over for a non-rt access;
                f->is_binning = true;
                for(int j = 0; j < f->npoints; j++)
                    f->output_buffer_a[j] = f->output_buffer_w[j];
                f->is_binning = false;
            }
        }
    }

}


 //Call this to get the frequency response in a thread-safe manner
int fra_get_spectrum(frequency_response_analyzer* f, float* frq_buf, float* mag_buf)
{
    float max = 0.0;
    float min = powf(10.0, -90.0 / 20.0);
    int n = f->npoints;

    while(f->is_binning)
    {
        usleep(10000);
    }

    if(f->normalize)
    {
        for(int i = 0; i < f->npoints; i++)
        {
            if(f->output_buffer_a[i] > max)
                max = f->output_buffer_a[i];
        }
        for(int i = 0; i < f->npoints; i++)
        {
            f->output_buffer_a[i] /= max;
        }
    }
    for(int i = 0; i < f->npoints; i++)
    {
        if(f->output_buffer_a[i] < min)
            f->output_buffer_a[i] = min;
        f->output_buffer_dB[i] = 20.0 * log10( f->output_buffer_a[i]);
    }

    for(int i = 0; i < n; i++)
    {
        mag_buf[i] = f->output_buffer_dB[i];
        frq_buf[i] = f->output_buffer_f[i];
    }
    return n;
}
