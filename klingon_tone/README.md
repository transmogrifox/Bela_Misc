# KLINGON-TONE #
## Guitar Stompbox Overdrive Effect Emulator ##

Modeled after the overdrive circuit found in the King of Tone analog pedal.

Derivation of the full tonestack is now complete (see analysis directory) and implemented in the overdrive virtual circuit model.

The render.cpp also includes processing on the Bela Analog Inputs for filtering pot inputs, detecting and processing settings changes.

Implemented higher sampling rate processing of nonlinear clipping function, currently first order up and down sampling rate conversion.
