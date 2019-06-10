# Vocoder - IIR Filterbank Model #
## Work in Progress ##
2nd-order filterbank implementation mirroring approach used in the traditional analog vocoder circuit.

## Current Capability ##
Basic vocoder works "plug & play".  No user-adjustable parameters, so any parameter changes must be hard-coded and re-compiled.

### MOD CH ### 

[Bela Cape Analog Pin 6: IN] -> [DC Removal Filter] -> [Dynamic Range Compressor]->(next line down)->

(cont'd)-> [N-Channel Filterbank] -> [N-Channel Envelope Detector] 

### CARRIER CH ### 

[Bela Cape Audio CH0: IN] -> [N-Channel Filterbank (modulated by MOD CH)] -> [Bela Cape Audio CH0: OUT] 

## Road Map ##
* Add dynamic range compressor to CARRIER CH also
* Finish implementing noise gate on MOD CH
* User-configurable controls:
  * Master Active/Bypass
  * MOD
    * Compressor Active/Bypass
    * Compressor ratio
    * Compressor Attack
    * Compressor Release
    * Compressor Threshold
    * Channel gain
    * Per-channel sensitivity
    * Detector bank attack time scaling control 
    * Detector bank release time scaling control
    * Detector bank resonance (Q) scaling control
  * CARRIER
    * Compressor Active/Bypass
    * Compressor ratio
    * Compressor Attack
    * Compressor Release
    * Compressor Threshold
    * Channel gain (master volume)
    * Mix MOD channel to filterbank input
    * Mix MOD channel to filterbank output
    * Master wet/dry mix


