#pragma once

#include "ofMain.h"
#include "ofxLibdc.h"

class testApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	void drawCapture();
	void drawPrep();
	
	int wait;
	ofxLibdc camera;
	bool captureMode;
	
	int pass, passCount;
	ofImage curImage;
	vector<ofImage> images;
	vector<float> shutter;
	bool saved;
};

