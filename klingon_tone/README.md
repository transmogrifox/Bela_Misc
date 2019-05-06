# KLINGON-TONE #
## Guitar Stompbox Overdrive Effect Emulator ##

Modeled after the overdrive circuit found in the King of Tone analog pedal.

Derivation of the full tonestack is now complete (see analysis directory) and implemented in the overdrive virtual circuit model.

The render.cpp also includes processing on the Bela Analog Inputs for filtering pot inputs, detecting and processing settings changes.

Implemented higher sampling rate processing of nonlinear clipping function, currently first order up and down sampling rate conversion.

King of Tone has several dip switches for different configuration of the clipping stage between hard or soft clipping.  In this DSP implementation both stages are processed in parallel and then the "Mix" function allows blending between hard and soft rather than limiting the user to either/or.

Not yet implemented is the "all clean" setting.

Clipping functions implemented by exporting SPICE simulation of clipping stages to text file, then interpolate from look-up table to apply to input signal.  

Final output limit function added to gracefully keep output within range of -1.0 to 1.0 using a softer curve than simply limiting.  This can be abused by cranking gain and output level when mix is set to min (soft).
