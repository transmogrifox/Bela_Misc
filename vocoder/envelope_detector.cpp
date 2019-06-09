#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <math_neon.h>

#include "envelope_detector.h"

void
envelope_detector_update_parameters(envelope_detector* edt)
{
    envelope_detector_set_release(edt, edt->t_release);
    envelope_detector_set_attack(edt, edt->t_attack);
    envelope_detector_set_ratio(edt, edt->ratio);
}

envelope_detector*
make_envelope_detector(envelope_detector* edt, float fs, int N)
{
    edt = (envelope_detector*) malloc(sizeof(envelope_detector));

    edt->fs = fs;
    edt->ifs = 1.0/fs;
    edt->N = N;

    edt->t_attack = 30.0;
    edt->t_release = 50.0;

    edt->atk = expf(-edt->ifs/(edt->t_attack));
    edt->atk0 = edt->atk;
    edt->rls = expf(-edt->ifs/(edt->t_release));
    edt->pkrls = expf(-edt->ifs/(0.0081));
    edt->pk_hold_time = lrintf(0.0042*edt->fs);

    envelope_detector_update_parameters(edt);

    edt->y1 = 0.0;
    edt->pk = 0.0;
    edt->pk_timer = 0;
    edt->rr_cyc = 0;
    for(int n = 0; n < 4; n++)
    {
   		edt->rr[n] = 0.0;
    }

    edt->reset = true;  //everything has been initialized

    return edt;
}

void
envelope_detector_tick_n(envelope_detector* edt, float *x, float *envelope)
{

    float yn;
    for(int i = 0; i < edt->N; i++)
    {
        //Run Sidechain

        //Peak detector
        //  This is a "round-robin" style peak detector.
        //  General strategy inspired by Harry Bissel:
        //   http://www.edn.com/design/analog/4344656/Envelope-follower-combines-fast-response-low-ripple
        //   http://m.eet.com/media/1143121/18263-figure_1.pdf
        //  A circular buffer (edt-rr[]) is used to store
        //  peak values and it is clocked every pk_hold_time.
        //  The oldest peak value is reset and the maximum
        //  of the circular buffer is used to represent the
        //  envelope.
        //  The output is a square-stepped track & hold
        //  signal that closely follows the signal envelope.
        //  Finally the rapid edges must be smoothed, so
        //  a last stage is added to filter the peak detector.
        //  The end result is a very smooth approximation of the
        //  envelope.
        //  The key is matching the round-robin cycling rate
        //  to the expected type of signal and desired response times.
        //  For guitar and bass it should be set to make a full cycle at about
        //  50 Hz, and hold time is (1/2)*(1/4) of that period.
        //  Bass low E is around 38 Hz, so may want to go slower for bass
        //  If response ends up being too fast, but ballistics filtering
        //  is expected to make up the difference on that front
        yn = fabs(x[i]);
    	if(yn > edt->rr[edt->rr_cyc])
    	{
    		edt->rr[edt->rr_cyc] = yn;
    	}

        //Cycle Timer
        if (edt->pk_timer < edt->pk_hold_time)
        {
            edt->pk_timer++;
        }
        else
        {
            if(++edt->rr_cyc >= 4)
        		edt->rr_cyc = 0;
        	edt->pk_timer = 0;
        	edt->rr[edt->rr_cyc] = 0.0;
        }

        //Final peak value selection circuit
        float tmpk = 0.0;
        for(int n = 0; n < 4; n++)
        {
        	if(edt->rr[n] > tmpk)
        		tmpk = edt->rr[n];
        }
        //Smooth the stair-steps
        //edt->pk = yn*(1.0-edt->pkrls) + edt->pk*edt->pkrls;
        edt->pk = tmpk*(1.0-edt->pkrls) + edt->pk*edt->pkrls;

        //
        // End Peak Detector
        //

		//
		//Ballistics
		// 
        yn = edt->pk;
        if(yn > edt->y1) //Attack
            edt->y1 = yn + (edt->y1 - yn)*edt->atk0;
        else //Release
            edt->y1 = yn + (edt->y1 - yn)*edt->rls;
        //
        // End Ballistics
        //

        x[i] = edt->y1;
        envelope[i] = edt->pk;
    }
}

void
envelope_detector_set_ratio(envelope_detector* edt, float r_)
{
    float r = r_;
    if(r < 1.0) r = 1.0;
    else if(r > 20.0) r = 20.0;
    edt->ratio = r/20.0;

}

void
envelope_detector_set_attack(envelope_detector* edt, float a_)
{
    //Expects units are in ms
    float a = a_;

    if(a < 0.1) a = 0.1;  //practical lower limit
    else if ( a > 1000.0 ) a = 1000.0; //more than 1s attack is probably not useful and likely a bug.

    a *= 0.001;  //convert to units of seconds
    edt->t_attack = a;
    edt->atk = expf(-edt->ifs/(edt->t_attack));
    edt->atk0 = edt->atk;

}

void
envelope_detector_set_release(envelope_detector* edt, float r_)
{
    //Expects units are in ms
    float r = r_;

    if(r < 1.0) r = 1.0;  //less than this is probably not useful
    else if ( r > 2000.0 ) r = 2000.0; //more is probably not useful and likely a bug.

    r *= 0.001;  //convert to units of seconds
    edt->t_release = r;
    edt->rls = expf(-edt->ifs/(edt->t_release));

}

void
envelope_detector_destructor(envelope_detector* edt)
{
	free(edt);
}
