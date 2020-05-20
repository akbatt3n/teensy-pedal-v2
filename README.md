# Teensy Pedal v2
An upgraded version of the teensy-based guitar pedal. This version aims to make the following improvements:
- [ ] Additional effects - reverb and "synth"
- [ ] Improved functionality - overdrive that works, synth sound with no latency, etc.
- [ ] Better usability - more intuitive controls, more informative display
- [x] Improved enclosure design
- [x] Cleaner internals - PCB instead of spaghetti wires

The pedal is still based on the Teensy 3.6 and audio adaptor from pjrc.com. Currently all circuitry is for controls and the display; no filters, pre-amps, etc. Audio in/out connects to the audio adaptor board.

# Changelog
## v2.0.1
  * added wet/dry mix to most effects (all but LFO)
  * updated interface for wet/dry mix
  * moved project to PlatformIO (for uploading without PlatformIO, put all .cpp/.h files from src, include, and lib into a single folder and use the regular Arduino IDE upload process)
  * moved constants and audio objects to header file
  * reduced control sensitivity to account for analogRead() impercise measurements

## v2.0
  * PCB designed and added
  * cleaner enclosure 3D printed with base attached by screws
  * true synthesizer changed to bitwise combining of input and reference signal

  

