# Teensy Pedal v2
An upgraded version of the teensy-based guitar pedal. This version aims to make the following improvements:
- [ ] Additional effects
    - [ ] synth (bitwise functions, square wave generator)
    - [x] low and high pass filters
    - [x] reverb
    - [x] LFO
- [x] More informative display
- [x] Improved enclosure design
- [x] Cleaner internals - PCB instead of spaghetti wires

The pedal is still based on the Teensy 3.6 and audio adaptor from pjrc.com. Currently all circuitry is for controls and the display; no filters, pre-amps, etc. Audio in/out connects to the audio adaptor board.

# Near Future Plans:
  * improve the sound of the synth effects. As of 2.1.0 they are too dissonant and must be dialed way back for the output to sound ok.
  * refactor code - move setup and adjust functions to library, pass parameters through the function rather than assuming global variables of a specific name exist. This helps shrink the main file and makes it easier to read.
  * Create program flowchart to help with readability without having to refactor the entire project.

# Changelog
## v2.0.2
  * added synth code (granulizer + combine)
  * updated interface for synth effects
  * peak detectors are only instantiated if DEBUGMODE is active, saving a small amount of resources for normal usage
  * improved documentation in code

## v2.0.1
  * added wet/dry mix to most effects (all but LFO)
  * updated interface for wet/dry mix
  * moved project to PlatformIO
  * moved constants and audio objects to header file
  * reduced control sensitivity to account for analogRead() impercise measurements

## v2.0
  * PCB designed and added
  * cleaner enclosure 3D printed with base attached by screws
  * true synthesizer changed to bitwise combining of input and reference signal

  

