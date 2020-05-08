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

#define COMBINE_MODE (AudioEffectDigitalCombine::AND)
#define GRANULAR_MEMORY_SIZE 6615
#define GRANULAR_LENGTH (GRANULAR_MEMORY_SIZE/44100)

#define LINE_IN_LEVEL 9
#define OUTPUT_VOLUME 0.3
#define WAVESHAPE_LENGTH 65
#define LFO_MAX_FREQ 5
#define HIGHPASSMAX 7000
#define HIGHPASSMIN 10.25
#define LOWPASSMAX 7000
#define LOWPASSMIN 10.25
#define COMBINE_FREQ_MIN 656
#define COMBINE_FREQ_MAX 5000
#define CONTROL_CHECK 15
#define CTRL_SENS 20

// GUItool: begin automatically generated code
//-------------------------------------------------------------------
	AudioControlSGTL5000     sgtl5000;     //xy=365,52
	AudioInputI2S            in;           //xy=73,116
	AudioSynthWaveformSine   LFO;          //xy=91,436
	AudioEffectGranular      granular;      //xy=148,226
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
	AudioConnection          patchCord5(granular, 0, combine, 1);
	AudioConnection          patchCord6(waveshape, amp);
	AudioConnection          patchCord7(LFOFilter, 1, effect2, 2);
	AudioConnection          patchCord8(combine, 0, effect1, 2);
	AudioConnection          patchCord9(freeverb, 0, effect2, 1);
	AudioConnection          patchCord10(amp, 0, effect1, 1);
	AudioConnection          patchCord11(lowPass, 0, highPass, 0);
	AudioConnection          patchCord12(effect2, 0, lowPass, 0);
	AudioConnection          patchCord13(effect1, 0, LFOFilter, 0);
	AudioConnection          patchCord14(effect1, 0, effect2, 0);
	AudioConnection          patchCord15(effect1, freeverb);
	AudioConnection          patchCord16(highPass, 2, out, 0);
	AudioConnection          patchCord17(highPass, 2, out, 1);
	#ifdef _DEBUGMODE_
		AudioAnalyzePeak         peak;          //xy=332,113
		AudioConnection          patchCord18(in, 1, peak, 0);
	#endif
	
// GUItool: end automatically generated code
//-------------------------------------------------------------------