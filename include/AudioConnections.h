#include <Arduino.h>
#include <Audio.h>

#define E1_STOMP 31
#define E2_STOMP 32
#define CYCLE_1F 27
#define CYCLE_1B 29
#define CYCLE_2F 28
#define CYCLE_2B 30
#define CONTROL1A A14 // pin 33
#define CONTROL1B A15 // pin 34
#define CONTROL2A A17 // pin 36
#define CONTROL2B A18 // pin 37
const int buttonPins[] = {E1_STOMP, E2_STOMP, CYCLE_2B, CYCLE_2F, CYCLE_1B, CYCLE_1F};
const int knobPins[] = {CONTROL1A, CONTROL1B, CONTROL2A, CONTROL2B};

#define GRANULAR_MEMORY_SIZE 6615
// GRANULAR_MEMORY_SIZE*100 / 44,100
#define GRANULAR_LENGTH 150
#define GRANULAR_ATTENUATION 0.05

// constants for the digital combine object
#define OR 0
#define XOR 1
#define AND 2
#define MODULO 3

#define LINE_IN_LEVEL 9
#define OUTPUT_VOLUME 0.3

#define WAVESHAPE_LENGTH 65
#define LFO_MAX_FREQ 5

#define HIGHPASSMAX 7000
#define HIGHPASSMIN 20
#define LOWPASSMAX 7000
#define LOWPASSMIN 20

#define CONTROL_CHECK 15
#define CTRL_SENS 20

// GUItool: begin automatically generated code
//-------------------------------------------------------------------
	AudioControlSGTL5000     sgtl5000;     //xy=365,52
	AudioInputI2S            in;           //xy=73,116
	AudioSynthWaveformSine   LFO;          //xy=91,436
	AudioEffectGranular      granular;      //xy=148,226
	AudioAmplifier           granAtt;
	AudioEffectWaveshaper    waveshape;     //xy=253,140
	AudioFilterStateVariable LFOFilter;        //xy=254,431
	AudioEffectDigitalCombine combine;       //xy=255,199
	AudioEffectFreeverb      freeverb;      //xy=265,357
	AudioAmplifier           amp;           //xy=404,135
	AudioFilterStateVariable lowPass;        //xy=433,510
	AudioMixer4              effect2;         //xy=566,392
	AudioMixer4              effect1;         //xy=577,128
	AudioFilterStateVariable highPass; //xy=587,510
	AudioOutputI2S           out;           //xy=759,518
	AudioConnection          patchCord1(in, 1, effect1, 0);
	AudioConnection          patchCord2(in, 1, waveshape, 0);
	AudioConnection          patchCord3(in, 1, combine, 0);
	AudioConnection          patchCord4(LFO, 0, LFOFilter, 1);
	AudioConnection          patchCord5(in, 1, granular, 0);
	AudioConnection          patchCord6(granular, granAtt);
	AudioConnection          patchCord7(waveshape, amp);
	AudioConnection          patchCord8(LFOFilter, 1, effect2, 2);
	AudioConnection          patchCord9(combine, 0, effect1, 2);
	AudioConnection          patchCord10(freeverb, 0, effect2, 1);
	AudioConnection          patchCord11(amp, 0, effect1, 1);
	AudioConnection          patchCord12(lowPass, 0, highPass, 0);
	AudioConnection          patchCord13(effect2, 0, lowPass, 0);
	AudioConnection          patchCord14(effect1, 0, LFOFilter, 0);
	AudioConnection          patchCord15(effect1, 0, effect2, 0);
	AudioConnection          patchCord16(effect1, freeverb);
	AudioConnection          patchCord17(highPass, 2, out, 0);
	AudioConnection          patchCord18(highPass, 2, out, 1);
	AudioConnection          patchCord19(granAtt, 0, combine, 1);

#ifdef _DEBUGMODE_
	AudioAnalyzePeak         peakInput;
	AudioConnection          patchCord30(in, 1, peakInput, 0);
	AudioAnalyzePeak         peakGran;
	AudioConnection			 patchCord31(granular, peakGran);
	AudioAnalyzePeak		 peakCombine;
	AudioConnection			 patchCord32(combine, peakCombine);
#endif	
// GUItool: end automatically generated code
//-------------------------------------------------------------------