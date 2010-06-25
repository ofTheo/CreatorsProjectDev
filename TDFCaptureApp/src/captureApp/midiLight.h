/*
 *  midiLight.h
 *  TDFCaptureApp
 *
 *  Created by theo on 6/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxMidi.h"

class midiLight{
	public:
	midiLight(){
		isLightOn = false;
	}
	
	void setup(int whichPort){
		lightControl.listPorts();
		lightControl.openPort(whichPort);
	}
	
	void lightOn(){
		lightControl.sendNoteOn(1, 0, 127);
		lightControl.sendNoteOff(1, 0, 127);
		lightControl.sendNoteOn(1, 1, 127);
		lightControl.sendNoteOff(1, 1, 127);
		lightControl.sendNoteOn(1, 2, 127);
		lightControl.sendNoteOff(1, 2, 127);
		lightControl.sendNoteOn(1, 3, 127);
		lightControl.sendNoteOff(1, 3, 127);		
		isLightOn = true;
	}
	
	void lightOff(){
		lightControl.sendNoteOn(1, 0, 0);
		lightControl.sendNoteOff(1, 0, 0);
		lightControl.sendNoteOn(1, 1, 0);
		lightControl.sendNoteOff(1, 1, 0);
		lightControl.sendNoteOn(1, 2, 0);
		lightControl.sendNoteOff(1, 2, 0);
		lightControl.sendNoteOn(1, 3, 0);
		lightControl.sendNoteOff(1, 3, 0);
		isLightOn = false;
	}
	
	void update(){
			
	}
	
	bool bLightOn(){
		return isLightOn;
	}
	
	ofxMidiOut lightControl;
	bool isLightOn;
};