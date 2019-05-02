//
// klingon-tone sketch,
//


#include <Bela.h>
#include <stdlib.h>
#include <math.h>

#include "klingon.h"

#define N_KNOBS         5
#define DRIVE           0
#define TONE            1
#define LEVEL           2
#define DRY		3
#define BOOST           4

float *ch0, *ch1;
int gAudioFramesPerAnalogFrame;
int gStartup_Control_Delay;

// klingon-tone implementation based conceptually on typical guitar stompbox OD like King of Tone
klingon* kot;

//
//Control functions related struct and processing
//

typedef struct knobs_t
{
    float fs;
    float ifs;
    int N;
    int channel;

    bool active;

    int active_timer;
    int active_time;

    float *frame;
    float b0;
    int scan_time;
    float dk_thrs;

    float y1, y2;
    float s0;
    int scan_timer;
} knobs;

knobs* make_knob(knobs* k, int channel, int N, float fs, float scan_time, float active_time, float tau, float thrs)
{
    k = (knobs*) malloc(sizeof(knobs));
    k->frame = (float*) malloc(sizeof(float)*N);
    k->fs = fs;
    k->ifs = 1.0/fs;
    k->N = N;
    k->channel = channel;

    for(int i = 0; i < N; i++)
        k->frame[i] = 0.0;

    k->b0 =  expf(-k->ifs/tau);
    k->scan_time = ((int) (fs*scan_time)) / N;
    k->active_time = ((int) (fs*active_time) ) / N;
    k->dk_thrs = thrs;

    k->y1 = k->y2 = 0.0;
    k->scan_timer = 0;
    k->active = false;
    k->active_timer = 0;

    return k;
}

//
//Filter analog inputs
// Detect changes in position
//

void knob_filter_run(knobs* k)
{
    float x = 0.0;
    //2 first order low-pass filters on analog inputs to smooth knob controls
    for(int i = 0; i < k->N; i++)
    {
        x = k->frame[i];
        k->y1 = x + (k->y1 - x) * k->b0;
        x = k->y1;
        k->y2 = x + (k->y2 - x) * k->b0;
    }

    //then check for changes
    if(++k->scan_timer >= k->scan_time)
    {
        k->scan_timer = 0;
        float dk = fabs(k->y2 - k->s0);
        if(dk > k->dk_thrs)
        {
            k->active_timer = k->active_time;
            //rt_printf("KNOB dk: %d, %f\n", k->channel, k->y2);
        }

        k->s0 = k->y2;
    }
    if(k->active_timer > 0)
    {
        if(k->active == false)
            rt_printf("KNOB is ACTIVE: %d, %f\n", k->channel, k->y2);
        k->active = true;
        k->active_timer--;
    }
    else
    {
        if(k->active == true)
        {
            rt_printf("KNOB %d is INACTIVE.  Value: %f\n", k->channel, k->y2);
        }
        k->active = false;
        k->active_timer = 0;
    }
}

void
knob_destructor(knobs* k)
{
	free(k);
}

//Control knobs:
knobs **kp;

// Extract a single audio channel from BelaContext and format for effect code input
void format_audio_buffer(BelaContext* context, float *outbuffer, int channel)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) {
        // Read the audio input and half the amplitude
        outbuffer[n] = audioRead(context, n, channel);
    }
}

// Extract a single analog channel from BelaContext and format for flanger code input
void format_analog_buffer(BelaContext* context, knobs *k)
{
    float ain = k->y1; //starts out with previous value -- just to initialize with something sane and make the compiler happy
    for(unsigned int n = 0; n < k->N; n++)
    {
        if(!(n % gAudioFramesPerAnalogFrame))
        {
             ain = analogRead(context, n/gAudioFramesPerAnalogFrame, k->channel);
        }
        // double-stuff it
        k->frame[n] = ain;
    }
    knob_filter_run(k);

    if(gStartup_Control_Delay <= 0)
	{
		if(gStartup_Control_Delay == 0)
		{
			rt_printf("Startup Delay Expired\n");
			gStartup_Control_Delay = -1;
		}

	    if(k->active)
	    {
	        switch(k->channel)
	        {
	            case DRIVE:
	                kot_set_drive(kot, map(k->y2, 0.0, 1.0, 12.0, 45.0) );
	                break;
	            case TONE:
	                kot_set_tone(kot, map(k->y2, 0.0, 1.0, -60.0, 0.0) );
	                break;
	            case LEVEL:
	                kot_set_level(kot, map(k->y2, 0.0, 1.0, -40.0, 0.0) );
	                break;
	            case BOOST:
	            	kot_set_boost(kot, k->y2);
	                break;
	            case DRY:
	            	kot_set_mix(kot, powf(10.0, map(k->y2, 0.0, 1.0, -60.0, 0.0)/20.0) );
	                break;
	            default:
	                break;
	        }
	    }
	}
	else
		gStartup_Control_Delay--;
}

bool setup(BelaContext *context, void *userData)
{
    ch0 = (float*) malloc(sizeof(float)*context->audioFrames);
    ch1 = (float*) malloc(sizeof(float)*context->audioFrames);

    // Create the klingon : make_klingon() initializes sane (usable) default settings
    kot = make_klingon(kot, 1, context->audioFrames, context->audioSampleRate);

    //Bypass is the least intuitive function:
    // If second argument is true, it forces bypass state.
    // If second argument is false, it toggles bypass state.
    // The second call below is all that is needed after make_klingon(), since the constructor function
    // sets bypass state to true.  A call to kot_set_bypass(kot, false) would then toggle to active-state
    // since it is already initialized to bypass state (true).
    // However for clarity in the example it is explicitly set to false, then toggled to demonstrate the normal sequence when
    // the current bypass state is unknown.
    kot_set_bypass(kot, true);
    kot_set_bypass(kot, false);

    //Setup analog control input functions
    kp = (knobs**) malloc(sizeof(knobs*)*N_KNOBS);
    for(int i = 0; i < N_KNOBS; i++)
    {
//make_knob(knobs* k, int channel, int N, float fs, float scan_time, float active_time, float tau, float thrs)
        kp[i] = make_knob(kp[i], i, context->audioFrames, context->audioSampleRate, 0.15, 1.0 , 0.01, 0.005);

    }

    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
    gStartup_Control_Delay = (int) (context->audioSampleRate*2.0);

    return true;
}

void render(BelaContext *context, void *userData)
{
    //separate audio inputs into their own buffers
    format_audio_buffer(context, ch1, 1);
    format_audio_buffer(context, ch0, 0);

    //Scan analog inputs and set settings
    for(int i = 0; i < N_KNOBS; i++)
        format_analog_buffer(context, kp[i]);


    //Run the klingon on ch0
    klingon_tick(kot, ch0);

    //Send it to the output
    for(unsigned int n = 0; n < context->audioFrames; n++)
    {
        // Write klingon through ch0, pass-through ch1
        audioWrite(context, n, 0, ch0[n]);
        audioWrite(context, n, 1, ch1[n]);
    }
}

void cleanup(BelaContext *context, void *userData)
{
    //
    //  Deallocate memory allocated during program execution
    //

    free(ch0);
    free(ch1);
    klingon_cleanup(kot);

    //Setup analog control input functions
    for(int i = 0; i < N_KNOBS; i++)
    {
		//deallocate memory from knobs
        knob_destructor(kp[i]);
    }
    free(kp);
}
