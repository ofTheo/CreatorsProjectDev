#pragma once

#include "ofMain.h"
#include "ofxStructuredLight.h"
#include "ofxAutoControlPanel.h"
#include "ofxThreadedVideoGrabber.h"
#include "ofxImageSaver.h"
#include "ofxFileHelper.h"
#include "ofxVideoGrabber.h"

typedef enum{
	CAMERA_CLOSED,
	CAMERA_NEEDS_OPENING,
	CAMERA_OPEN,
	CAMERA_NEEDS_CLOSING,
}cameraState;

class captureApp : public ofBaseApp{

public:
	void update1394Cam();
	void frameReceived(ofVideoGrabber& grabber);

	void handleCamera();

	void setup();
	void update();
	void draw();
	
	//--------------------------------------------------------------
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);	
	void mouseReleased(int x, int y, int button);
	void keyPressed(int key);


	bool hidden;
	ofxAutoControlPanel panel;

	ThreePhaseGenerator threePhase;
	int lastWavelength;

	GrayCodeGenerator grayCode;
	int lastSubdivisions;

	TwoPlusOneGenerator twoPlusOne;

	GradientGenerator gradient;

	PatternGenerator* curGenerator;
	int lastPatternType;
	int lastOrientation;
	bool lastFullscreen;
	
	cameraState camState;
	cameraState prevCamState;

	int lastMinBrightness;

	ofxThreadedVideoGrabber camera;
	ofxImageSaver imageSaver;
	
	ofxVideoGrabber camera1394;
	Libdc1394Grabber * sdk;
	ofxIIDCSettings * settings;	

	int cameraWidth, cameraHeight;
	ofImage cameraImage;
	vector<ofImage> recent;
	vector<bool> needsUpdate;
	int lastCameraRate;
	int cameraFrameNum;
	int patternFrame;
};
