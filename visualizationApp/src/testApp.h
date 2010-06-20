
#pragma once

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxControlPanel.h"
#include "ofxDirList.h"
#include "ofxAutoControlPanel.h"
#include "scanPlayer.h"

class testApp : public ofBaseApp {

public:
	
	~testApp();
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	ofxEasyCam camera;
	ofxAutoControlPanel panel;
	simpleFileLister inputList;
	
	scanPlayer  SP;
	
};
