## OVERDRIVE Guitar Stompbox Effect Emulator ##

Pre-emphasis and post-emphasis modeled after Ibanez Tubescreamer overdrive circuit.
Tone is a 1rst-order shelving function, but does not implement the sweeping cut-off frequency like the TS-9 Tone uses.

The render.cpp also includes processing on the Bela Analog Inputs for filtering pot inputs, detecting and processing settings changes.

Implemented higher sampling rate processing of nonlinear clipping function:
* Linear interpolation upsampling
* Zero-order hold donwnsampling after band-limiting with de-emphasis filter and first-order anti-aliasing filter.
