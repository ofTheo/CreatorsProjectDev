
#pragma once

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxControlPanel.h"
#include "ofxDirList.h"
#include "ofxControlPanel.h"
#include "scanPlayer.h"
#include "scanNotifier.h"

class testApp : public ofBaseApp {

public:
	
	~testApp();
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	ofxEasyCam camera;
	ofxControlPanel panel;
	simpleFileLister inputList;
	
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased();
	
	scanNotifier SN;
	scanPlayer  SP;
	
};
