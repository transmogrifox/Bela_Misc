//
// EQ filter definitions
//

#ifndef EQ_H
#define EQ_H

#define LPF         0
#define HPF         1
#define BPF         2
#define PK_EQ       3
#define LOW_SHELF   4
#define HIGH_SHELF  5

typedef struct cx_t
{
    float r;  //magnitude or //real
    float i;  //phase or //imaginary
} cx;

typedef struct eq_t
{
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;

    float y1;
    float y2;
    float x1;
    float x2;

    //Current settings
    int type;
    float fs;
    float f0;
    float Q;
    float G;

    //pre-computed values
    float c, s, alpha, w0, A;

    //Filterbank channel gain
    float gfb;

} eq_coeffs;

typedef struct equalizer_t
{
    size_t nbands;
    size_t shelf;  // 0 to 2 (1 run low shelf, 2 run low and high shelf)
    eq_coeffs** band;
} eq_filters;

// Typical peaking EQ array, number of bands logarithmically spaced from fstart to fstop
eq_filters*
make_equalizer(eq_filters* eq, size_t nbands, float fstart_, float fstop_, float sample_rate);

eq_coeffs*
make_eq_band(int type, eq_coeffs* cf, float fs, float f0, float Q, float G);

// Filter bank, cluster of band-pass filters logarithmically spaced from fstart to fstop,
// Lowest filter in range is low-pass, highest filter is high-pass
eq_filters*
make_filterbank(eq_filters* eq, size_t nbands, float fstart_, float fstop_, float sample_rate);

float
eq_get_filterbank_f0(eq_filters* eq, size_t band);

// De-allocate all dynamically allocated memory on an EQ
void
destroy_equalizer(eq_filters* eq);

void
eq_compute_coeffs(eq_coeffs* cf, int type, float fs, float f0, float Q, float G);

void
eq_update_gain(eq_coeffs* cf, float G);

inline float
tick_eq_band(eq_coeffs* cf, float x)
{

    float y0 = cf->b0*x + cf->b1*cf->x1 + cf->b2*cf->x2
                        + cf->a1*cf->y1 + cf->a2*cf->y2;
    cf->x2 =cf->x1;
    cf->x1 = x;
    cf->y2 = cf->y1;
    cf->y1 = y0;
    return y0;
    //return y0*cf->gfb;

}

inline float
geq_tick(eq_filters* eq, float x_)
{
    int i;
    float x = x_;
    for(i=0; i < (eq->nbands + eq->shelf); i++)
    {
        x = tick_eq_band(eq->band[i], x);
    }
    return x;
}

void
geq_tick_n(eq_filters* eq, float *xn, size_t N);

void plot_response(float , float , int , eq_coeffs* , float , cx*);
//void plot_response(float f1, float f2, int pts, eq_coeffs* cf, float fs, cx *r)
#endif
