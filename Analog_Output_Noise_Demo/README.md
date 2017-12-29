This is a little bit of code that creates a PWL (Piece Wise Linear) file that can then be imported into LTSpice (or similar simulator) and evaluated with simulated filters.  The intention is to use the built-in FFT utility in the simulator to evaluate how much spectral noise remains in the audio spectrum before and/or after the simulated filter.

The end result is to synthesize a realizable filter that can be attached to a Bela DAC output for getting acceptable audio quality.
