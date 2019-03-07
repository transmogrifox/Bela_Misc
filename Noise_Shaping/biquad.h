
#ifndef BIQUAD_H
#define BIQUAD_H

#define LPF     0
#define HPF     1
#define BPF     2
#define HPF1P   3
#define LPF1P   4
#define WAH     5

typedef struct cx_t {
    float r;  //magnitude or //real
    float i;  //phase or //imaginary
} cx;

typedef struct biquad_t {
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
    
    float y1;
    float y2;
    float x1;
    float x2;
    
} biquad_coeffs;

typedef struct wah_coeffs_t {

    float gf;
    float ax;
    float a0;
    
    
    
    //biquad filter coefficients
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
    
} wah_coeffs;


biquad_coeffs* 
make_biquad(int, biquad_coeffs*, float, float, float);
//make_biquad(int type, biquad_coeffs* cf, float fs, float f0, float Q);

void compute_filter_coeffs(int, biquad_coeffs*, float, float, float);

void biquad_reset_state_variables(biquad_coeffs* );


//run_filter(float x, biquad_coeffs* cf);
float run_filter(float, biquad_coeffs*);


//float run_wah(float x, wah_coeffs* bq)
float run_wah(float, wah_coeffs*);


//void plot_response(float f1, float f2, int pts, biquad_coeffs* cf, float fs, cx *r)
void plot_response(float , float , int , biquad_coeffs* , float , cx*);

#endif








