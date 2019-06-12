#ifndef ENV_DETECTOR_H
#define ENV_DETECTOR_H

typedef struct envelope_detector_t
{
    //System Parameters
    float fs;  //Sample rate
    float ifs; //Inverse sample rate
    int N;     //Processing block size

    //User settings
    bool reset;
    float ratio;
    float t_attack;
    float t_release;
    float out_gain;

    //Internal variables
    float atk, atk0, rls;  //attack and release coefficients

    //Peak detector
    float pkrls;
    float pk_hold_time;

    //state variables
    float gain;
    float y1;
    float pk;
    float rr[4];
    int rr_cyc;
    int pk_timer;

} envelope_detector;

//Allocate and initialize struct with sane values
//fs = Sampling Frequency
//N = processing block size
envelope_detector*
make_envelope_detector(envelope_detector* edt, float fs, int N);

//This is where it happens
//x[] is expected to be of length edt->N
void
envelope_detector_tick_n(envelope_detector* edt, float *x, float *envelope);

//Settings

//input unitless: 1 to 20
void
envelope_detector_set_ratio(envelope_detector* edt, float r_);

//input units in ms:  1 ms to 1000 ms
void
envelope_detector_set_attack(envelope_detector* edt, float a_);

//input units in ms:  10 ms to 1000 ms
void
envelope_detector_set_release(envelope_detector* edt, float r_);

//Peak detector hold time: 100us to 100 ms
void
envelope_detector_set_pkhold(envelope_detector* edt, float h_);

//deallocate memory when finished using the struct
void
envelope_detector_destructor(envelope_detector* edt);

#endif //ENV_DETECTOR_H
