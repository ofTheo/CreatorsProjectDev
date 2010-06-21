#pragma once

#include "ofMain.h"
#include "ofxStructuredLight.h"
#include "ofxAutoControlPanel.h"
#include "ofxThreadedVideoGrabber.h"
#include "ofxImageSaver.h"
#include "ofxFileHelper.h"
#include "ofxVideoGrabber.h"
#include "faceFinder.h"
#include "decodeAndExport.h"

typedef enum{
	CAMERA_CLOSED,
	CAMERA_NEEDS_OPENING,
	CAMERA_OPEN,
	CAMERA_NEEDS_CLOSING
}cameraState;

typedef enum{
	CAP_STATE_WAITING,
	CAP_STATE_PERSON_DETECTED,
	CAP_STATE_COUNTDOWN,
	CAP_STATE_CAPTURE,
	CAP_STATE_SAVING,
	CAP_STATE_NOTIFY,
	CAP_STATE_OUTPUT_AND_DISPLAY
}CapAppState;

typedef enum{
	CAP_DEBUG,
	CAP_NORMAL
}CapDebugState;

class captureApp : public ofBaseApp, public ofxThread{

public:
	void update1394Cam();
	void frameReceived(ofVideoGrabber& grabber);
	

	void drawCloud();
	void setup();
	void update();
	void draw();
		
	void threadedFunction();

	void startCapture();
	void endCapture();

	void handleFaceTrigger();
	void handleProjection();
	void handleCamera();
	
	//--------------------------------------------------------------
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);	
	void mouseReleased(int x, int y, int button);
	void keyPressed(int key);

	
	ofxAutoControlPanel panel;

	ThreePhaseGenerator threePhase;
	int lastWavelength;

	GrayCodeGenerator grayCode;
	int lastSubdivisions;
	float timeToEndCapture;

	TwoPlusOneGenerator twoPlusOne;

	GradientGenerator gradient;

	PatternGenerator* curGenerator;
	int lastPatternType;
	int lastOrientation;
	bool lastFullscreen;
	
	cameraState camState;
	cameraState prevCamState;
	
	CapAppState state;
	CapDebugState debugState;

	faceFinder face;	
	float prevFaceCheckTimeF;
	bool bNeedsToLeaveFrame;
	
	ofxEasyCam camera3D;

	decodeAndExport decoder;
	int saveIndex;
	
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
