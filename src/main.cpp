// Teensy Synth Pedal
//-----------------------------------------------------------------------------
// Written for the Teensy 3.6 microcontroller with the Teensy Audio Adaptor.
// Utilizes the teensy audio library to apply effects to instrument signals, 
//     primarily guitar and bass, but could also be used for any electric 
//     instrument.
// Requires a teensy 3.6, audio adaptor, an LCD, and various control 
//     potentiometers and buttons.
//-----------------------------------------------------------------------------
// For those who aren't aware, Arduino code is just C++ and the IDE 
//     automatically adds #include <Arduino.h> when compiling. You can save your
//     programs as .cpp files while adding the #include at the top. You also
//     need to put function headers at the top (above where you call them for
//     the first time) if you weren't already.
//-----------------------------------------------------------------------------
// Connections to remember:
// Effect1's and Effect2's channel 0 is bypass. e1/e2 indicates the effect, but
// are zero-indexed (e1=0 means square wave generator is selected ). When setting
// a channel to activate an effect, use e1+1 (or e2+1).
// 
// BitL and BitH (e1 = 1 and 2) use the same objects and thus use the same mixer
// connection, audio objects, and many of the same settings. The only difference
// is whether the granulizer should speed up or slow down its signal.

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <LiquidCrystal.h>

// _DEBUGMODE_ prints the status of the pedal over serial
// _EFFECTDETAILS_ provides in dept details on the status of specific effects
// #define _DEBUGMODE_
// #define _EFFECTDETAILS_

#ifdef _EFFECTDETAILS_
	#define _DEBUGMODE_
#endif

// More audio objects are created in debug mode like a peak detector,
//    so we need to #include this after the _DEBUGMODE_ definition
#include "AudioConnections.h"

// Variables and objects
//-----------------------------------------------------------------------------
int control = 0;


float WAVESHAPE[WAVESHAPE_LENGTH] = {};
int16_t granularMem[GRANULAR_MEMORY_SIZE];
float deviations[] = {1, 2};

bool ctrlChange = false;
bool e1Active, e2Active = false;

// if true, knobs adjust effect1 parameters. if false, knobs adjust low/high pass frequencies
bool e1EfctCtrls = true;


// Strings to print on the display and the numbers of effects
#define NUM_EFFECTS_E1 3
#define NUM_EFFECTS_E2 2
String effect1Types[] = {"  Sqr", "Bit-L", "Bit-H"};
String effect2Types[] = {"Revrb", "LFO  "};

// Variables to store the state of the pedal (what effect is active, where is this knob, etc.)
int e1, e2 = 0;
int ctrl1A = analogRead(CONTROL1A);
int ctrl1B = analogRead(CONTROL1B);
int last1A, last1B;
int ctrl2A = analogRead(CONTROL2A);
int ctrl2B = analogRead(CONTROL2B);
int last2A, last2B;

// value used for mixer volume on the effect. Bypass channel is set to 1.0 - wet
float wet1, wet2 = 1.0;
float dry1, dry2 = 0.0;

// Objects for the LCD display and buttons
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);
byte solid[8] = { B11111, B11111, B11111, B11111,
				B11111, B11111, B11111, B11111 };
Bounce cycle1F = Bounce(CYCLE_1F, 5);
Bounce cycle1B = Bounce(CYCLE_1B, 5);
Bounce cycle2F = Bounce(CYCLE_2F, 5);
Bounce cycle2B = Bounce(CYCLE_2B, 5);
Bounce e2Stomp = Bounce(E2_STOMP, 5);
Bounce e1Stomp = Bounce(E1_STOMP, 5);

// function declerations for initializing the audio objects
void setupLFO();
void setupHighLowPass();
void setupOverdrive();
void setupMixers();
void setupCombine();
void setupReverb();

// function declerations for adjusting effect parameters
void overdriveGain(float controlA, float controlB);
void lowHighFilters(float controlA, float controlB);
void lfoAdjust(float controlA, float controlB);
void combineAdjust(float controlB);
void reverbAdjust(float controlB);


void setup() {

	// standard arduino setup for pins and LCD
	lcd.begin(16, 2);
    lcd.createChar(0, solid);
	for (unsigned int i = 0; i < (sizeof(buttonPins)/sizeof(int)); i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
	}
	for (unsigned int i = 0; i < (sizeof(knobPins)/sizeof(int)); i++) {
		pinMode(knobPins[i], INPUT);
	}

	// Audio library initialization
	AudioMemory(20);
	sgtl5000.enable();
	sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
	sgtl5000.lineInLevel(LINE_IN_LEVEL);
	sgtl5000.volume(OUTPUT_VOLUME);

	// Call all effect setup functions
	setupLFO();
	setupHighLowPass();
	setupOverdrive();
    setupMixers();
    setupCombine();
    setupReverb();

	#ifdef _DEBUGMODE_
		Serial.begin(9600);
	#endif
}

void loop() {

	// We don't need to update the controls as fast as possible since most of 
	// the time they aren't changing at all. Also saves some resources.
	if (control < CONTROL_CHECK) {
		control++;
		delay(20);
    }
	else {
		control = 0;

// UPDATE CONTROLS AND EFFECT STATE VARIABLES
// Updates the stomp switches as well as the effect selection buttons and their
// corresponding variables.
//-----------------------------------------------------------------------------
		cycle1F.update();
	    cycle1B.update();
	    cycle2F.update();
	    cycle2B.update();
	    e1Stomp.update();
	    e2Stomp.update();

		// cycle effect1 type
		// only 1 button for changing, so it can't underflow
		if (cycle1F.fallingEdge()) {
    		e1++;
    		if (e1 >= NUM_EFFECTS_E1) e1 = 0; // overflow
    	}

		// toogle if knobs should control effect1 or high/low pass filters
    	else if (cycle1B.fallingEdge()) {
    		e1EfctCtrls = !e1EfctCtrls;
    	}

		// cycle effect2 type. effect2 has 2 buttons, so it can under and overflow
    	if (cycle2F.fallingEdge()) {
    		e2++;
    		if (e2 >= NUM_EFFECTS_E2) { e2 = 0;} // overflow
    	}
    	else if (cycle2B.fallingEdge()) {
    		e2--;
    		if (e2 <= -1) {e2 = NUM_EFFECTS_E2-1;} // underflow
    	}

		// activate or deactivate effects based on stomp switches
    	if (e1Stomp.fallingEdge()) {
	    	e1Active = false;
	    }
	    else if (e1Stomp.risingEdge()) {
	    	e1Active = true;
	    }

	    if (e2Stomp.fallingEdge()) {
	    	e2Active = false;
            
	    }
	    else if (e2Stomp.risingEdge()) {
	    	e2Active = true;
	    }


// UPDATE MIXERS BASED ON STATE VARIABLES & MIX CONTROL
// changes the mixer's inputs based on what effect is selected. For effects
// with a wet/dry mix, this section reads the knob and calculates the proper
// mix to apply.
//-----------------------------------------------------------------------------

	    if (e1Active) {
			if (e1EfctCtrls) {
				wet1 = analogRead(CONTROL1A) / 1023.0;
				dry1 = 1.0 - wet1;
			}

	    	switch(e1) {
	    		case 0: // square wave, stop granular to save resources
					granular.stop();
	    			effect1.gain(0, dry1);
	    			effect1.gain(1, wet1);
	    			effect1.gain(2, 0.0);
	    		case 1:
	    		case 2: // BitL/BitH both use the same audio object
					granular.beginPitchShift(GRANULAR_LENGTH);
	    			effect1.gain(0, dry1);
	    			effect1.gain(1, 0.0);
	    			effect1.gain(2, wet1);
	    			break;
	    		default:
	    			break;
	    	}
	    }
	    else { // effect1 off, signal bypasses audio objects
			granular.stop();
	    	effect1.gain(0, 1.0);
	    	effect1.gain(1, 0.0);
	    	effect1.gain(2, 0.0);
	    }

	    if (e2Active) {
	    	switch(e2) {
                case 0: // reverb
					wet2 = analogRead(CONTROL2A) / 1023.0;
					dry2 = 1.0 - wet2;
                    effect2.gain(0, dry2);
                    effect2.gain(1, wet2);
                    effect2.gain(2, 0.0);
                    break;
                case 1: // LFO
                    effect2.gain(0, 0.0);
                    effect2.gain(1, 0.0);
                    effect2.gain(2, 1.0);
                    break;
                default:
                    break;
            }
	    }
	    else { // effect2 off, signal bypasses audio objects
	    	effect2.gain(0, 1.0);
            effect2.gain(1, 0.0);
            effect2.gain(2, 0.0);
	    }

// ADJUST PARAMETERS BASED ON CONTROL VARIABLES
//-------------------------------------------
		// update effect parameters if effect1 is active, or if knobs are set
		// to adjust high/low pass filters. These are active even if no effect
		// is active
		if (e1Active || !e1EfctCtrls) {
			ctrl1A = analogRead(CONTROL1A);
			ctrl1B = analogRead(CONTROL1B);

			// check if value has changed significantly. If it has, set last1x
			// this way turning the knob slowly will still trigger ctrlChange
			if (ctrl1A < (last1A - CTRL_SENS) || ctrl1A > (last1A + CTRL_SENS)) {
				last1A = ctrl1A;
				ctrlChange = true;
			}
			if (ctrl1B < (last1B - CTRL_SENS) || ctrl1B > (last1B + CTRL_SENS)) {
				last1B = ctrl1B;
				ctrlChange = true;
			}

			// if value has changed significantly, then modify parameters of the
			// effect
			if (ctrlChange) {
				if (e1EfctCtrls) {
					switch (e1) {
						case 0: // square wave
							break;
						case 1: // BitL
						case 2: // BitH
							combineAdjust((float)ctrl1B);
							break;
						default: // panic
							break; 
					}
				}
				else {
					lowHighFilters((float)ctrl1A, (float)ctrl1B);
				}
			}
			ctrlChange = false;
		}

		// do the same thing as above for effect2
		if (e2Active) {
			ctrl2A = analogRead(CONTROL2A);
			ctrl2B = analogRead(CONTROL2B);
			if (ctrl2A < (last2A - CTRL_SENS) || ctrl2A > (last2A + CTRL_SENS)) {
				last2A = ctrl2A;
				ctrlChange = true;
			}
			if (ctrl2B < (last2B - CTRL_SENS) || ctrl2B > (last2B + CTRL_SENS)) {
				last2B = ctrl2B;
				ctrlChange = true;
			}
			if (ctrlChange) {
				switch (e2) {
					case 0: // Reverb
						reverbAdjust((float)ctrl2B);
						break;
					case 1: // LFO
						lfoAdjust((float)ctrl2A, (float)ctrl2B);
						break;
					default: // panic
						break; 
				}
			}
			ctrlChange = false;
		}

// PRINT TO LCD
// currently the effect names are from the global array but control names are
// hardcoded here.
//-----------------------------------------------------------------------------
		lcd.setCursor(0, 0);
		lcd.print(effect2Types[e2]); // names are 5 characters long
		if (e2Active) lcd.write(byte(0)); else lcd.print(" ");
		if (e2Active) lcd.write(byte(0)); else lcd.print(" ");
		lcd.print("||");
		if (e1Active) lcd.write(byte(0)); else lcd.print(" ");
		if (e1Active) lcd.write(byte(0)); else lcd.print(" ");
		lcd.print(effect1Types[e1]);
        lcd.setCursor(0,1);

		// 2nd line
		switch(e2) {
			case 0: // Reverb
				lcd.print("Mix Sze");
				break;
			case 1: // LFO
				lcd.print("Rng Spd");
				break;
			default:
				lcd.print("       ");
				break;
		}
        lcd.print("||");
		if (e1EfctCtrls) {
			switch(e1) {
				case 0: // Square wave
					lcd.print("Mix    ");
					break;
				case 1: // BitL
				case 2: // BitH
					lcd.print("Mix Dev");
					break;
				default:
					break;
			}
		}
		else {
			lcd.print("Hi  Low");
		}

	} //close the else statement for control updates

// CODE FOR DEBUGGING
// print details about the pedal state to serial output. This may not print
// data that is helpful to you in its current config. You should change this
// code to show the data that is helpful to you before debugging
//-----------------------------------------------------------------------------
	#ifdef _DEBUGMODE_
		#ifdef _EFFECTDETAILS_
			Serial.print(" || Input: ");
			if (peakInput.available()) Serial.print(peakInput.read());
			else Serial.print("x.xx");
			Serial.print(" || Granulizer Output: ");
			if (peakGran.available()) Serial.print(peakGran.read());
			else Serial.print("x.xx");
			Serial.print(" || Combine Output: ");
			if (peakCombine.available()) Serial.print(peakCombine.read());
			else Serial.print("x.xx");
			Serial.println();
		#endif
    	#ifndef _EFFECTDETAILS_
    		Serial.print("Effect 1 Active: ");
    		Serial.print(e1Active);
    		Serial.print(" || Effect 2 Active: ");
    		Serial.print(e2Active);
    		Serial.print(" || Current E2: ");
    		Serial.print(effect2Types[e2]);
    		Serial.print(" || Current E1: ");
    		Serial.print(effect1Types[e1]);
    		Serial.print(" || Memory Usage Max: ");
    		Serial.print(AudioMemoryUsageMax());
			Serial.print("|| Mix (w1, d1, w2, d2): ");
			Serial.print(wet1);
			Serial.print(" | ");
			Serial.print(dry1);
			Serial.print(" | ");
			Serial.print(wet2);
			Serial.print(" | ");
			Serial.print(dry2);
			Serial.print(" || ");
			Serial.print("Input Amplitude: ");
			if (peak.available()) Serial.print(peak.read());
			else Serial.print("     ");
            Serial.println();
		#endif
	#endif
}

// SETUP FUNCTIONS
// these initialize the audio objects with some default values and some measured
// ones. the point is to get everything running when the pedal turns on, rather
// than when an effect is selected
//-----------------------------------------------------------------------------
void setupLFO() {
	LFO.amplitude(0.9);
	LFO.frequency(0.1);
	LFOFilter.frequency(640);
	LFOFilter.resonance(0.7);
	LFOFilter.octaveControl(5);
}

void setupHighLowPass() {
	lowPass.frequency(LOWPASSMAX);
	lowPass.resonance(0.7);
	highPass.frequency(HIGHPASSMIN);
	highPass.frequency(0.7);
}

void setupOverdrive() {
	// this effect is still referred to as overdive, even though it is setup to
	// turn the input signal into a square wave
	for (int i = 0; i < (WAVESHAPE_LENGTH-1)/2; i++) {
		WAVESHAPE[i] = -0.95;
		WAVESHAPE[WAVESHAPE_LENGTH - 1 - i] = 0.95;
	}
	WAVESHAPE[(WAVESHAPE_LENGTH-1)/2] = 0.0;

	waveshape.shape(WAVESHAPE, WAVESHAPE_LENGTH);
    amp.gain(0.3);
}

void setupMixers() {
	// turn all mixer inputs off, then activate the one that corresponds to
	// the active effect. 
    effect1.gain(0, 0.0);
    effect1.gain(1, 0.0);
    effect2.gain(0, 0.0);
    effect2.gain(1, 0.0);
    effect2.gain(2, 0.0);
    effect2.gain(3, 0.0);

    if (digitalRead(E1_STOMP) == LOW) {
        effect1.gain(0, 1.0);
    }
    else {
    	switch (e1) {
    		case 0:
    			effect1.gain(1, 1.0);
    			break;
    		case 1:
    		case 2:
    			effect1.gain(2, 1.0);
    			break;
    		default:
    			break;
    	}
    }

    if (digitalRead(E2_STOMP) == HIGH) {
        effect2.gain(e2+1, 1.0);
    }
    else {
        effect2.gain(0, 1.0);
    }
}

void setupCombine() {
	granular.begin(granularMem, GRANULAR_MEMORY_SIZE);
	granular.setSpeed(1.0);
	combine.setCombineMode(XOR);
	granAtt.gain(GRANULAR_ATTENUATION);
}

void setupReverb() {
	float controlB = analogRead(CONTROL2B) / 1023;
	freeverb.roomsize(controlB);
	freeverb.damping(1.0);
}

// PARAMETER ADJUSTMENT FUNCTIONS
// these are called when a knob has been turned and effect parameters need to 
// be changed accordingly
//-----------------------------------------------------------------------------
void overdriveGain(float controlA, float controlB) {
	// may be implemented in the future
}

void lowHighFilters(float controlA, float controlB) {
	controlA = map(controlA, 0, 1023, LOWPASSMIN, LOWPASSMAX);
	controlB = map(controlB, 0, 1023, HIGHPASSMIN, HIGHPASSMAX);

	lowPass.frequency(controlA);
	highPass.frequency(controlB);
}

void lfoAdjust(float controlA, float controlB) {
	controlA = map(controlA, 0.0, 1023.0, 0.01, 5);
	controlB = map(controlB, 0.0, 1023.0, 0.01, LFO_MAX_FREQ);
	LFOFilter.octaveControl(controlA);
	LFO.frequency(controlB);
}

void combineAdjust(float controlB) {
	// map controlB to 0 - 3 integers only
	controlB = (int) ((controlB / 1023.0) * 1.99);

    // dev should be 1, 2, 4, or 8 taken from deviations and using controlB as an index
    float dev = deviations[(int) controlB];

    // invert dev for bit-L effect to get octaves down
	if (e1 == 1) {
		dev = 1.0 / dev;
	}
	// set speedup/slowdown of granular object to value of controlB
	granular.setSpeed(dev);    
}

void reverbAdjust(float controlB) {
	controlB = controlB / 1023;
	freeverb.roomsize(controlB);
}