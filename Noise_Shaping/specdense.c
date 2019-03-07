#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <fftw3>  //if in your compiler search path

#include "../../../FFTW3/fftw-3.3.5-dll32/fftw3.h" //or path to libfftw3 header
#include "biquad.h"  //IIR Filters

// FFT output parameters
#define ROWS 10000          //Number of FFT points
#define COLS 11             //Number of gain scans to evaluate

// FILTERING PARAMETERS
#define AUDIO_FS    44100   //Audio sampling frequency
#define F_CENTER    1000     //Hz
#define F_RES       60      //Band-pass Filter Q

//FFTW variables
fftw_complex fftw_in[ROWS], fftw_out[ROWS];
fftw_plan ftPlanForward;

//Pulse generation
static  int cyc_cntr;
static  int duty_cntr; 
static  int duty_cnt;
static float r_output;
static float r_duty_f;

// Noise shaping filter
biquad_coeffs* nf;
    
// Shaped noise PWM function
float pwm_randseq_tick(float gain)
{
    r_duty_f = run_filter((float) (rand()%32), nf);
    cyc_cntr++;
    duty_cntr++;
    if(cyc_cntr >= 32)
    {
        cyc_cntr = 0;
        duty_cntr = 0;
        r_output = 1.0;  // set
        
        duty_cnt = (int) (gain*r_duty_f);
        if(duty_cnt > 30) duty_cnt = 30;
        else if (duty_cnt < 2) 
            duty_cnt = 2;
    }

    if(duty_cntr >= duty_cnt)
    {
        duty_cntr = 0;
        r_output = 0.0; // reset
    }
    
    //return r_output;
    if(r_duty_f > 0.0)
        return 1.0;
    else 
        return 0.0;
 
}

// Shaped noise, 1-bit sampled
// Produces prominent peak at filter cut-off
    // Gain parameter currently not implemented here
    // The implementation would be to strobe this function on
    // and off to make total output power adjustable
float randseq_tick(float gain)
{
    r_duty_f = run_filter((float) (rand()%2048)/2048.0, nf);
    if(r_duty_f > 0.0)
        return 1.0/ROWS;
    else 
        return 0.0;
 
}

int main(int argc, char** argv)
{
    FILE * pImpulse, *pFFT;
    pImpulse = fopen ("impulse_response.txt","w");
    pFFT = fopen ("frequency_response.txt","w");

    srand(16);
    size_t nfftFrameSize = ROWS;
    ftPlanForward = fftw_plan_dft_1d(nfftFrameSize, fftw_in, fftw_out, FFTW_FORWARD, FFTW_MEASURE);

    cyc_cntr = 0;
    duty_cntr = 0;
    duty_cnt = 0; 
    r_output = 0.0;
    r_duty_f = 0.0;
    nf = make_biquad(BPF, nf, AUDIO_FS, F_CENTER, 1.0);
    
    int k;
    for(k=0; k<nfftFrameSize; k++)
    {
        fftw_in[k][0] = 0.0;
        fftw_in[k][1] = 0.0;
    }

    //Sampling frequency
    float fs = 44100.0;


    //output filter pulse responses
    int i,j;
    float gstep = 0.1;
    float gp = 1.0/F_RES;
    float x = 0.0;
    float y = 0.0;
    unsigned int seq = 0;
    float impulse[ROWS+1][COLS+2];
    float fft[ROWS+1][COLS];
    float ftfreq[ROWS];
    float real = 0.0;
    float imag = 0.0;
    float fstep = fs/ROWS;

    for(i=0; i<COLS; i++)
    {
        for(j=0; j<ROWS; j++)
        {
                impulse[j][i] = 0.0;
        }
    }
    fprintf(pImpulse, "SEQ\tX\t");

    for(i=0; i<COLS; i++)
    {
        fprintf(pImpulse, "Y%d\t", i);
        compute_filter_coeffs(BPF, nf, AUDIO_FS, F_CENTER, F_RES*gp);
        for(j=0; j<ROWS; j++)
        {

            y = randseq_tick(gp);
            // if(y == 0.0)
                // y = 1.0/ROWS;
            // else y = 0.0;
            if(i == 0) {
                impulse[j][0] = x;
                impulse[j][1] = y;

                ftfreq[j] = ((float) j)*fstep;
            }
            else {
                impulse[j][i+1] = y;
            }

                fftw_in[j][0] = y;
                fftw_in[j][1] = 0.0;
        }

        fftw_execute(ftPlanForward);
        for(j=0; j<ROWS; j++)
        {
            real = fftw_out[j][0];
            imag = fftw_out[j][1];
            fft[j][i] = 20.0*log10( sqrtf(real*real + imag*imag) );

        }
        printf("Processing Step: %f\n", gp);
        gp += gstep;
        x = 0.0;
        
    }
    fprintf(pImpulse, "\n");


    for(j=0; j<ROWS; j++)
    {
        fprintf(pImpulse, "%d\t", j);

        if(ftfreq[j] > 100.0 && ftfreq[j] < 10000.0)
            fprintf( pFFT , "%f\t", ftfreq[j]);

        for(i=0; i<COLS; i++)
        {
            fprintf(pImpulse, "%f\t", impulse[j][i]);

            if(ftfreq[j] > 100.0 && ftfreq[j] < 10000.0)
                fprintf( pFFT , "%f\t", fft[j][i]);
        }
        fprintf(pImpulse, "\n");
        if(ftfreq[j] > 100.0 && ftfreq[j] < 10000.0)
            fprintf( pFFT , "\n");
    }

    fftw_destroy_plan(ftPlanForward);
    fclose(pImpulse);
    fclose(pFFT);

    return 0;
}
