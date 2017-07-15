Frequency Magnitude Response analyzer for Bela.

The main use case is to trace the frequency response of the analog (and digital) signal chain through Bela, particularly in validating the DC Removal filter configured in the CODEC and exploring the effect of ADC level on input impedance (high-pass cut-off rises with ADC gain).

This can also be used to trace out frequency (magnitude) response of any signal path you can put in the loop between Bela output and input.

Basic usage case is to loop channel 0 output to channel 0 input, and likewise with channel 1.  

This program generates a log-linear sine chirp which is sent to the Bela Audio outputs and analyzes incoming signal from Audio inputs with a peak hold detector and reset logic to trace the frequency response at pre-defined intervals along the spectrum.

Things this is NOT:
>Not an RTA (Real-Time Analyzer)
>Not a spectrum analyzer
>Currently does not detect phase (but may be implemented in the future)

This application requires a sweep time/rate slow enough to approximate steady-state sinusoidal behavior throughout the sweep.  This sweep rate is dependent upon the system being measured (for example a highly resonant system would require longer sweep rates for improved accuracy).
