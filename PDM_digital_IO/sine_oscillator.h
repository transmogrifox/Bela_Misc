#ifndef SINE_OSCILLATOR_H
#define SINE_OSCILLATOR_H

typedef struct sinosc_t {
    float Ts;
    
    // oscillator coefficient
    float k;
    
    // oscillator output offset and amplitude
    float offset;
    float amp;
    float phase;
    
    // State variables
    float c;
    float s;
    
} sinosc;

// Allocate the struct and set up defaults
sinosc* make_sine_oscillator(sinosc* osc, float fsw, float frq, float phase, float offset, float amp);

// Set oscillator current phase
void set_sine_oscillator_phase(sinosc* osc, float phase);

// Amplitude
void set_sine_oscillator_amp(sinosc* osc, float amp);

// Offset
void set_sine_oscillator_offset(sinosc* osc, float offset);

// Oscillator frequency
void set_sine_oscillator_frequency(sinosc* osc, float f);

// Tick - run the oscillator each cycle
float sine_oscillator_tick(sinosc* osc);

#endif