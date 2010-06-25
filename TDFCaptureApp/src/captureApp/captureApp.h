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
#include "ofxTimeStamp.h"
#include "appConstants.h"
#include "ofxOsc.h"
#include "midiLight.h"

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
	CAP_STATE_FADEIN,
	CAP_STATE_CAPTURE,
	CAP_STATE_START_DECODE,
	CAP_STATE_DECODING,
	CAP_STATE_END_DECODE,
	CAP_STATE_SAVING,
	CAP_STATE_NOTIFY,
	CAP_STATE_OUTPUT_AND_DISPLAY
}CapAppState;

enum{
	POST_CAPTURE_SAVE,
	POST_CAPTURE_DECODE,
	POST_CAPTURE_DECODE_EXPORT,
	POST_CAPTURE_ALL_AND_NOTIFY
};

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
		
	void setupOsc();
	
	void threadedFunction();

	void startCapture();
	void handleCapture();
	void endCapture(bool cancelSave = false);
	
	void startFadeIn();
	
	void prepareTransferFramesToVizApp();
	void prepareExportFramesToDisk();
	
	void startDecode();
	void handleDecode();
	void endDecode(bool bCancelSave = false);

	void handleFaceTrigger();
	void handleProjection();
	void updateGenerator();
	void handleCamera();
	
	//--------------------------------------------------------------
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);	
	void mouseReleased(int x, int y, int button);
	void keyPressed(int key);

	void getClipping(ofImage& img, ofImage& clipping);
	
	ofxAutoControlPanel panel;

	ThreePhaseGenerator threePhase;
	float timeToEndCapture;
	PatternGenerator* curGenerator;
	
	cameraState camState;
	cameraState prevCamState;
	
	CapAppState state;
	CapDebugState debugState;

	faceFinder face;	
	float prevFaceCheckTimeF;
	bool bNeedsToLeaveFrame;
	
	bool bSetup;
	ofxEasyCam camera3D;
	
	ofSoundPlayer scanningSound;
	ofImage spotLightImage;
	float spotLightAlpha;

	decodeAndExport decoder;
	int saveIndex;
	int saveCount;
	float timeToDecode;
	float fadeInStartTime;
	
	string currentTimestamp;
	string currentDecodeFolder;
	string currentDecodePath;
	string currentCaptureFolder;
	string currentCity;
	
	bool bDoThreadedRSync;
	bool bDoThreadedFrameSave;
	string executeStr;
	
	bool bEnableOsc;
	bool bOscSetup;
	ofxOscSender oscTx;
	midiLight light;
	
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
