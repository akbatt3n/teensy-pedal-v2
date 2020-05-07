// Teensy Synth Pedal
//-----------------------------------------------------------------------------
// Written for the Teensy 3.6 microcontroller with the Teensy Audio Adaptor.
// Utilizes the teensy audio library to apply effects to instrument signals, 
// 		primarily guitar and bass, but could also be used for any electric 
// 		instrument.
// Requires a teensy 3.6, audio adaptor, an LCD, and various control 
// 		potentiometers/switches.
//-----------------------------------------------------------------------------
// Connections to remember:
// Effect1's and Effect2's channel 0 is bypass. e2 indicates the effect, but is
// zero-indexed. When setting a channel to activate an effect, use e2+1
//
// TODO:
//

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <LiquidCrystal.h>

#include "AudioConnections.h"


// debug mode prints the status of the pedal over serial
//#define _DEBUGMODE_
//#define _EFFECTDETAILS_
#ifdef _EFFECTDETAILS_
	#define _DEBUGMODE_
#endif


// Variables and objects
//-----------------------------------------------------------------------------
int control = 0;
float WAVESHAPE[WAVESHAPE_LENGTH] = {};

bool ctrlChange = false;
bool e1Active, e2Active = false;

// if true, knobs adjust effect. if false, knobs adjust low/high pass
bool e1EfctCtrls = true;

#define NUM_EFFECTS_E1 3
#define NUM_EFFECTS_E2 2

// Strings to print on the display
String effect1Types[] = {"  Sqr", "  AND", "   OR"};
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

// true when an effect is first selected in order to adjust mixers. Otherwise, mixers are controlled by
//   effect parameter functions
//bool e1First, e2First = true;

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


void setupLFO();
void setupHighLowPass();
void setupOverdrive();
void setupMixers();
void setupCombine();
void setupReverb();

void overdriveGain(float controlA, float controlB);
void lowHighFilters(float controlA, float controlB);
void lfoAdjust(float controlA, float controlB);
void combineAdjust(float controlB);
void reverbAdjust(float controlB);

void setup() {

	lcd.begin(16, 2);
    lcd.createChar(0, solid);

	for (unsigned int i = 0; i < (sizeof(buttonPins)/sizeof(int)); i++) {
		pinMode(buttonPins[i], INPUT_PULLUP);
	}
	for (unsigned int i = 0; i < (sizeof(knobPins)/sizeof(int)); i++) {
		pinMode(knobPins[i], INPUT);
	}

	AudioMemory(25);
	sgtl5000.enable();
	sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
	sgtl5000.lineInLevel(9);
	sgtl5000.volume(0.3);

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

	if (control < CONTROL_CHECK) {
		control++;
		delay(20);
    }
	else {
		control = 0;

// UPDATE CONTROLS AND CONTROL VARIABLES
//---------------------------------------
		cycle1F.update();
	    cycle1B.update();
	    cycle2F.update();
	    cycle2B.update();
	    e1Stomp.update();
	    e2Stomp.update();

		// cycle effect1 type
		if (cycle1F.fallingEdge()) {
    		e1++;
    		if (e1 >= NUM_EFFECTS_E1) e1 = 0; // overflow

			// set combine mode for e1 = 1 or 2 (e1=0 is square wave)
            switch(e1) {
                case 1: // AND
                    combine.setCombineMode(AudioEffectDigitalCombine::AND);
                    break;
                case 2: // OR
                    combine.setCombineMode(AudioEffectDigitalCombine::OR);
                    break;
                default:
                    break;
            }
    	}

		// toogle if knobs should control effect1 or high/low pass filters
    	else if (cycle1B.fallingEdge()) {
    		e1EfctCtrls = !e1EfctCtrls;
    	}

    	if (cycle2F.fallingEdge()) {
    		e2++;
    		if (e2 >= NUM_EFFECTS_E2) { e2 = 0;} // overflow
    	}
    	else if (cycle2B.fallingEdge()) {
    		e2--;
    		if (e2 <= -1) {e2 = NUM_EFFECTS_E2-1;} // underflow
    	}


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


// UPDATE MIXERS BASED ON CONTROL VARIABLES
//------------------------------------------

	    if (e1Active) {
			if (e1EfctCtrls) {
				wet1 = analogRead(CONTROL1A) / 1023.0;
				dry1 = 1.0 - wet1;
			}

	    	switch(e1) {
	    		case 0: // square wave
	    			effect1.gain(0, dry1);
	    			effect1.gain(1, wet1);
	    			effect1.gain(2, 0.0);
	    		case 1:
	    		case 2:
	    			effect1.gain(0, dry1);
	    			effect1.gain(1, 0.0);
	    			effect1.gain(2, wet1);
	    			break;
	    		default:
	    			break;
	    	}
	    }
	    else {
	    	effect1.gain(0, 1.0);
	    	effect1.gain(1, 0.0);
	    	effect1.gain(2, 0.0);
	    }

	    if (e2Active) {
	    	switch(e2) {
                case 0: // reverb
					wet2 = analogRead(CONTROL2A);
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
	    else {
	    	effect2.gain(0, 1.0);
            effect2.gain(1, 0.0);
            effect2.gain(2, 0.0);
	    }

// ADJUST PARAMETS BASED ON CONTROL VARIABLES
//-------------------------------------------
		if (e1Active || !e1EfctCtrls) {
			ctrl1A = analogRead(CONTROL1A);
			ctrl1B = analogRead(CONTROL1B);

			// check if value has changed significantly
			if (ctrl1A < (last1A - CTRL_SENS) || ctrl1A > (last1A + CTRL_SENS)) {
				last1A = ctrl1A;
				ctrlChange = true;
			}
			if (ctrl1B < (last1B - CTRL_SENS) || ctrl1B > (last1B + CTRL_SENS)) {
				last1B = ctrl1B;
				ctrlChange = true;
			}

			// if value has changed significantly, then modify parameters of the effect
			if (ctrlChange) {
				if (e1EfctCtrls) {
					switch (e1) {
						case 0: // square wave
							break;
						case 1: // AND
						case 2: // OR
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

// PRINT STATUS TO LCD
//--------------------
		lcd.setCursor(0, 0);
		lcd.print(effect2Types[e2]); // names are 4 characters long
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
				case 1: // AND
				case 2: // OR
					lcd.print("Mix Tone");
					break;
				default:
					break;
			}
		}
		else {
			lcd.print("Hi  Low");
		}

	} //close the else statement for controls

// CODE FOR DEBUGGING
//-------------------
	#ifdef _DEBUGMODE_
    	#ifndef _EFFECTDETAILS_
    		Serial.print("Effect 1 Active: ");
    		Serial.print(e1Active);
    		Serial.print(" || Effect 2 Active: ");
    		Serial.print(e2Active);
    		Serial.print(" || Current E2: ");
    		Serial.print(effect2Types[e2]);
    		Serial.print(" || Current E1: ");
    		Serial.print(effect1Types[e1]);
    		Serial.print(" || Control status: ");
    		Serial.print(ctrl2A);
    		Serial.print(" | ");
    		Serial.print(ctrl2B);
    		Serial.print(" | ");
    		Serial.print(ctrl1A);
    		Serial.print(" | ");
    		Serial.print(ctrl1B);
    		Serial.print(" || Memory Usage Max: ");
    		Serial.print(AudioMemoryUsageMax());
            Serial.println();
		#endif
	#endif
}

// SETUP FUNCTIONS
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
	for (int i = 0; i < (WAVESHAPE_LENGTH-1)/2; i++) {
		WAVESHAPE[i] = -0.95;
		WAVESHAPE[WAVESHAPE_LENGTH - 1 - i] = 0.95;
	}
	WAVESHAPE[(WAVESHAPE_LENGTH-1)/2] = 0.0;

	waveshape.shape(WAVESHAPE, WAVESHAPE_LENGTH);
    amp.gain(0.3);
}

void setupMixers() {
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
	float controlB = analogRead(CONTROL1B);
	float controlA = analogRead(CONTROL1A);
	controlB = map(controlB, 0, 1023, COMBINE_FREQ_MIN, COMBINE_FREQ_MAX);
	waveform.begin(WAVEFORM_SINE);
	waveform.frequency(controlB);;
    waveform.amplitude(0.0001);
	combine.setCombineMode(AudioEffectDigitalCombine::AND);
}

void setupReverb() {
	float controlB = analogRead(CONTROL2B) / 1023;
	freeverb.roomsize(controlB);
	freeverb.damping(1.0);
}

// PARAMETER ADJUSTMENT FUNCTIONS
//-----------------------------------------------------------------------------
void overdriveGain(float controlA, float controlB) {}

void lowHighFilters(float controlA, float controlB) {
	controlA = map(controlA, 0, 1023, LOWPASSMIN, LOWPASSMAX);
	controlB = map(controlB, 0, 1023, HIGHPASSMIN, HIGHPASSMAX);

    #ifdef _EFFECTDETAILS_
        Serial.print("LowPass Freq: ");
        Serial.print(controlA);
        Serial.print("HighPass Freq: ");
        Serial.print(controlB);
        Serial.println();
    #endif

	lowPass.frequency(controlA);
	highPass.frequency(controlB);
}

void lfoAdjust(float controlA, float controlB) {
	controlA = map(controlA, 0.0, 1023.0, 0.01, 5);
	controlB = map(controlB, 0.0, 1023.0, 0.01, LFO_MAX_FREQ);
	LFOFilter.octaveControl(controlA);
	LFO.frequency(controlB);
    
    #ifdef _EFFECTDETAILS_
        Serial.print("CtrlB: ");
        Serial.print(controlB);
    #endif
}

void combineAdjust(float controlB) {
	controlB = map(controlB, 0, 1023, COMBINE_FREQ_MIN, COMBINE_FREQ_MAX);
	waveform.frequency(controlB);
   #ifdef _EFFECTDETAILS_
        Serial.print(" | Combine Freq: ");
        Serial.print(controlB);
        Serial.println();
   #endif
    
}

void reverbAdjust(float controlB) {
	controlB = controlB / 1023;

	freeverb.roomsize(controlB);
}