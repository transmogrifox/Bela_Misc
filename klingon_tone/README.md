# KLINGON-TONE #
## Guitar Stompbox Effect Emulator ##

Pre-emphasis and post-emphasis modeled after an overdrive circuit named after something to do with a king and tone.

Tone control is just a high-frequency roll-off approximating something in the first tone knob in the analog archetype.  Derivation of the full tonestack is now complete (see analysis directory) but not yet implemented in the overdrive virtual circuit.

The render.cpp also includes processing on the Bela Analog Inputs for filtering pot inputs, detecting and processing settings changes.

Implemented higher sampling rate processing of nonlinear clipping function:
* Linear interpolation upsampling
* Biquatdratic waveshaping function
* Zero-order hold donwnsampling after band-limiting with first-order anti-aliasing filter.
