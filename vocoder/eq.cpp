#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "eq.h"

void
eq_compute_coeffs(eq_coeffs* cf, int type, float fs, float f0, float Q, float G)
{
    float A = powf(10.0, G/40.0);
    float w0 = 2.0*M_PI*f0/fs;
    float c = cosf(w0);
    float s = sinf(w0);

    float alpha = s/(2.0*Q);

    cf->fs = fs;
    cf->f0 = f0;
    cf->Q = Q;
    cf->G = G;
    cf->type = type;

    cf->A = A;
    cf->w0 = w0;
    cf->c = c;
    cf->s = s;
    cf->alpha = alpha;
    eq_update_gain(cf, G);

}

void
eq_update_gain(eq_coeffs* cf, float G)
{
    float A = powf(10.0, G/40.0);
    float Q = cf->Q;
    float sqrtA2 = 0.0;
    cf->G = G;

    float c = cf->c;
    float alpha = cf->alpha;
    float iA = 1.0/A;
    float a0, a1, a2;
    float b0, b1, b2;
    a0 = a1 = a2 = 0.0;
    b0 = b1 = b2 = 0.0;

    switch(cf->type)
    {
        default:
        case LPF:
            b0 =  (1.0 - c)/2.0;
            b1 =   1.0- c;
            b2 =  (1.0 - c)/2.0;
            a0 =   1.0 + alpha;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha;
            break;

         case HPF:
            b0 =  (1.0 + c)/2.0;
            b1 =   -(1.0 + c);
            b2 =  (1.0 + c)/2.0;
            a0 =   1.0 + alpha;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha;
            break;

         case BPF:
            b0 =   Q*alpha;
            b1 =   0.0;
            b2 =  -Q*alpha;
            a0 =   1.0 + alpha;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha;
            break;

        case PK_EQ:
            b0 =   1.0 + alpha*A;
            b1 =  -2.0*c;
            b2 =   1.0 - alpha*A;
            a0 =   1.0 + alpha*iA;
            a1 =  -2.0*c;
            a2 =   1.0 - alpha*iA;
            break;

        case LOW_SHELF:
            sqrtA2 = 2.0*sqrtf(A)*alpha;
            b0 =  A*( (A+1.0) - (A-1.0)*c + sqrtA2);
            b1 =  2.0*A*( (A-1.0) - (A+1.0)*c);
            b2 =  A*( (A+1.0) - (A-1.0)*c - sqrtA2);
            a0 =  (A+1.0) + (A-1.0)*c + sqrtA2;
            a1 =  -2.0*( (A-1.0) + (A+1.0)*c);
            a2 =  (A+1.0) + (A-1.0)*c - sqrtA2;
            break;

        case HIGH_SHELF:
            sqrtA2 = 2.0*sqrtf(A)*alpha;
            b0 = A*( (A+1.0) + (A-1.0)*c + sqrtA2 );
            b1 = -2.0*A*( (A-1.0) + (A+1.0)*c);
            b2 = A*( (A+1.0) + (A-1.0)*c - sqrtA2 );
            a0 = (A+1.0) - (A-1.0)*c + sqrtA2;
            a1 = 2.0*( (A-1.0) - (A+1.0)*c);
            a2 = (A+1.0) - (A-1.0)*c - sqrtA2;
            break;
    }

		float ia0 = 1.0/a0;
        b0 *=  ia0;
        b1 *=  ia0;
        b2 *=  ia0;
        a1 *=  ia0;
        a2 *=  ia0;

        cf->b0 = b0;
        cf->b1 = b1;
        cf->b2 = b2;
        //negate 'a' coefficients so addition can be used in filter
        cf->a1 = -a1;
        cf->a2 = -a2;

}

eq_coeffs*
make_eq_band(int type, eq_coeffs* cf, float fs, float f0, float Q, float G)
{

    cf = (eq_coeffs*) malloc(sizeof(eq_coeffs));
    eq_compute_coeffs(cf, type, fs, f0, Q, G);
    cf->y1 = 0.0;
    cf->y2 = 0.0;
    cf->x1 = 0.0;
    cf->x2 = 0.0;

    //printf("TYPE: %d\nFS: %f\nf0: %f\nQ: %f\nG: %f\n\n", cf->type, cf->fs, cf->f0, cf->Q, cf->G);
    return cf;

}

eq_filters*
make_equalizer(eq_filters* eq, size_t nbands, float fstart_, float fstop_, float sample_rate)
{
    float G = 0.0;
    float f1 = fstop_;
    float f0 = fstart_;
    if(f1 > sample_rate/2.0) f1 = sample_rate/2.0;
    float nb = (float) nbands;
    float m=powf(2.0,((logf(f1/f0)/logf(2.0f))/(nb-1)));
    float Q = 0.75 * (m + 1.0)/ (m - 1.0);

    int i = 0;
    eq = (eq_filters*) malloc(sizeof(eq_filters));
    eq->band = (eq_coeffs**) malloc((nbands+2)*sizeof(eq_coeffs*));
    eq->nbands = nbands;
    for(i=0; i<nbands; i++)
    {
        //printf("f0 = %f\tQ = %f\tfl=%f\tfh=%f\n", f0, Q, f0-0.5*f0/Q, f0+0.5*f0/Q);
        eq->band[i] = make_eq_band(PK_EQ, eq->band[i], sample_rate, f0, Q, G);
        eq->band[i]->f0 = f0;
        eq->band[i]->Q = Q;
        eq->band[i]->G = G;
        eq->band[i]->fs = sample_rate;
        eq->band[i]->type = PK_EQ;

        f0 *= m;
    }
    //Create low shelf and high shelf filters
    eq->band[nbands] = make_eq_band(LOW_SHELF, eq->band[nbands], sample_rate, 336.0, 0.5, G);
    eq->band[nbands+1] = make_eq_band(HIGH_SHELF, eq->band[nbands+1], sample_rate, 1418.0, 0.5, G);

    eq->band[nbands]->f0 = 336.0;
    eq->band[nbands]->Q = Q;
    eq->band[nbands]->G = G;
    eq->band[nbands]->fs = sample_rate;
    eq->band[nbands]->type = LOW_SHELF;

    eq->band[nbands+1]->f0 = 1418.0;
    eq->band[nbands+1]->Q = Q;
    eq->band[nbands+1]->G = G;
    eq->band[nbands+1]->fs = sample_rate;
    eq->band[nbands+1]->type = HIGH_SHELF;

    eq->shelf = 2; //Default run both high & low shelf filters
    return eq;
}

eq_filters*
make_filterbank(eq_filters* eq, size_t nbands, float fstart_, float fstop_, float sample_rate)
{
    float G = 0.0;
    float f1 = fstop_;
    float f0 = fstart_;
    if(f1 > sample_rate/2.0) f1 = sample_rate/2.0;
    float nb = (float) nbands;
    float m=powf(2.0,((logf(f1/f0)/logf(2.0f))/(nb-1.0)));
    float Q = 0.5 * (m + 1.0)/ (m - 1.0);

    int i = 0;
    eq = (eq_filters*) malloc(sizeof(eq_filters));
    eq->band = (eq_coeffs**) malloc((nbands+2)*sizeof(eq_coeffs*)); // keep size compatible so same destructor can be used
    eq->nbands = nbands;

    // Low frequency band is LPF
    eq->band[i] = make_eq_band(LPF, eq->band[i], sample_rate, f0, Q, G);
    eq->band[i]->f0 = f0;
    eq->band[i]->Q = Q;
    eq->band[i]->G = G;
    eq->band[i]->fs = sample_rate;
    eq->band[i]->type = LPF;
    eq->band[i]->gfb = 1.0;

    f0 *= m;

    for(i=1; i< (nbands - 1); i++)
    {
        //printf("f0 = %f\tQ = %f\tfl=%f\tfh=%f\n", f0, Q, f0-0.5*f0/Q, f0+0.5*f0/Q);
        eq->band[i] = make_eq_band(BPF, eq->band[i], sample_rate, f0, Q, G);
        eq->band[i]->f0 = f0;
        eq->band[i]->Q = Q;
        eq->band[i]->G = G;
        eq->band[i]->fs = sample_rate;
        eq->band[i]->type = BPF;
        eq->band[i]->gfb = 1.0;

        f0 *= m;
    }

    // High frequency band is HPF
    i = nbands - 1;  // explicit
    eq->band[i] = make_eq_band(HPF, eq->band[i], sample_rate, f0, Q, G);
    eq->band[i]->f0 = f0;
    eq->band[i]->Q = Q;
    eq->band[i]->G = G;
    eq->band[i]->fs = sample_rate;
    eq->band[i]->type = HPF;
    eq->band[i]->gfb = 1.0;

    //Create low shelf and high shelf filters
    eq->band[nbands] = make_eq_band(LOW_SHELF, eq->band[nbands], sample_rate, 336.0, 0.5, G);
    eq->band[nbands+1] = make_eq_band(HIGH_SHELF, eq->band[nbands+1], sample_rate, 1418.0, 0.5, G);

    eq->band[nbands]->f0 = 336.0;
    eq->band[nbands]->Q = Q;
    eq->band[nbands]->G = G;
    eq->band[nbands]->fs = sample_rate;
    eq->band[nbands]->type = LOW_SHELF;

    eq->band[nbands+1]->f0 = 1418.0;
    eq->band[nbands+1]->Q = Q;
    eq->band[nbands+1]->G = G;
    eq->band[nbands+1]->fs = sample_rate;
    eq->band[nbands+1]->type = HIGH_SHELF;

    eq->shelf = 0; //Default don't run both high & low shelf filters
    return eq;
}

void
destroy_equalizer(eq_filters* eq)
{
    int i = 0;
    int bands = eq->nbands + 2;

    for(i=0; i<bands; i++)
        free(eq->band[i]);

    free(eq->band);
    free(eq);
}

void
geq_tick_n(eq_filters* eq, float *xn, size_t N)
{
    int i, k;
    int cnt = eq->nbands + 2;
    float x = 0.0;

    for(k=0; k<N; k++)
    {
        x = xn[k];
        for(i=0; i < cnt; i++)
        {
            x = tick_eq_band(eq->band[i], x);
        }
        xn[k] = x;
    }
}

inline float
sqr(float x)
{
    return x*x;
}

void
plot_response(float f1, float f2, int pts, eq_coeffs* cf, float fs, cx *r)
{
    float dp = 2.0*M_PI;
    // z^-1 = cos(w) - j*sin(w)
    // z^-2 = cos(2*w) - j*sin(2w)

    float num[2], den[2];
    num[0] = num[1] = 0.0;
    den[0] = den[1] = 0.0;

    float dw = dp*(f2-f1)/pts;
    dw /= fs;
    float w = dp*f1/fs;
    float magn = 0.0;
    float magd = 0.0;
    float an = 0.0;
    float ad = 0.0;

    int n = abs(pts) + 1;

    int i;
    for(i=0; i<n; i++)
    {
        num[0] = cf->b0 + cf->b1*cos(-w) + cf->b2*cos(-2.0*w); //numerator real part
        num[1] = cf->b1*sin(-w) + cf->b2*sin(-2.0*w);
        den[0] = 1.0 + cf->a1*cos(-w) + cf->a2*cos(-2.0*w); //denominator real part
        den[1] = cf->a1*sin(-w) + cf->a2*sin(-2.0*w);

        magn = sqrtf(sqr(num[0]) + sqr(num[1]));
        magd = sqrtf(sqr(den[0]) + sqr(den[1]));
        an = atanf(num[1]/num[0]); //angle of numerator
        ad = atanf(den[1]/den[0]); //angle of denominator

        r[i].r = magn/magd;
        r[i].i = an - ad;

        //printf("%f\t%f\t%f\n", fs*w/dp, 20.0*log10(r[i].r), 180.0*r[i].i/(M_PI));

        w += dw;
    }


}

//Function to get parameters
float
eq_get_filterbank_f0(eq_filters* eq, size_t band)
{
	return eq->band[band]->f0;
}

