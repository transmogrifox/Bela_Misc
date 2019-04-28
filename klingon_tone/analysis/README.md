# King of Tone Circuit Analysis #
## Derivation of digitized frequency response model ##

Here are files supporting analysis of the King of Tone circuit, used for implementing klingon_tone code.

### SPICE ###
LTSpice (and several other SPICE programs) include the "Laplace" function, which enables evaluation of digital filters using the 
expression:

z^-1 = e^-sT

Where T = 1/SAMPLE_RATE

The simulation includes the part of the King of Tone circuit analyzed with text comments stepping through the process.  The simulation also 
includes the IIR filter stages used to implement the filters throughout, implemented using behavioral voltage sources with the Laplace 
directive.

The set of IIR filters used for implementing the tonestack are not as straightforward, and for this it is recommended to look at the scanned 
derivation (pdf) and MATLAB (GNU Octave) file to follow the derivation process.

The simulation simply includes this to verify against the analog circuit frequency response and validate that neglecting the
effect of the diodes on the small signal response is a reasonable assumption (notice the ~0.1dB error).

LTSpice simulation and plot files included.

Output graph (.png file) represents frequency response at several "Tone" and "Boost" pot settings.

### Octave ###
GNU Octave was used to validate the derivation of the tonestack section of the circuit.  This file is included.

### Tonestack Derivation ###
The derivation of the tonestack (pencil and paper) is scanned and included in PDF format.  This traces steps through
circuit analysis and Bilinear Transformation of the s-domain transfer function.


