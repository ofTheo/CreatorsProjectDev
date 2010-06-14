#include "captureApp.h"

static bool do1394 = false;
string capturePrefix = "input/";

static float lastTime = 0.0;
static float camFps = 60.0f;
int preCamMode = 0;

void measureCamFps(){
	float timeDiff = ofGetElapsedTimef()-lastTime;
	lastTime = ofGetElapsedTimef();
		
	float fps = 1.0/ofClamp(timeDiff, 0.01, 1000.0);
	camFps *= 0.95;
	camFps += 0.05 * fps;
}

void captureApp::frameReceived(ofVideoGrabber& grabber) {
	if(&grabber == &camera) {
		int cameraRate = panel.getValueI("cameraRate");
		int cameraOffset = panel.getValueI("cameraOffset");
		int sequenceSize = curGenerator->size();
		int curFrame = cameraFrameNum + cameraOffset;
		// the size is 0 when we're preparing it in another thread
		if(sequenceSize != 0) {
			if(curFrame % cameraRate == 0) {
				curFrame = (curFrame / cameraRate) % sequenceSize;
				int n = (int) camera.getWidth() * (int) camera.getHeight() * 3;
				if(!hidden) {
					camera.getThreadedPixels(recent[curFrame].getPixels());
					needsUpdate[curFrame] = true;
				} else {
					string filename = capturePrefix+"capture/" + ofToString(cameraFrameNum) + ".jpg";
					imageSaver.setFilename(filename);
					camera.getThreadedPixels(imageSaver.getPixels());
					imageSaver.next();
				}
			}
			measureCamFps();
			cameraFrameNum++;
		}
	}
}

void captureApp::update1394Cam(){

	camera1394.update();
	if( camera1394.isFrameNew() ){
		int cameraRate = panel.getValueI("cameraRate");
		int cameraOffset = panel.getValueI("cameraOffset");
		int sequenceSize = curGenerator->size();
		
		int curFrame = cameraFrameNum + cameraOffset;
		// the size is 0 when we're preparing it in another thread
		if(sequenceSize != 0) {
			if(curFrame % cameraRate == 0) {
				curFrame = (curFrame / cameraRate) % sequenceSize;
				int n = (int) camera.getWidth() * (int) camera.getHeight() * 3;
				if(!hidden) {
					memcpy(recent[curFrame].getPixels(), camera1394.getPixels(), camera1394.getWidth() * camera1394.getHeight() * 3);					
					needsUpdate[curFrame] = true;
				}else{
					string filename = capturePrefix+"capture/" + ofToString(cameraFrameNum) + ".jpg";
					imageSaver.setFilename(filename);
					memcpy(imageSaver.getPixels(), camera1394.getPixels(), camera1394.getWidth() * camera1394.getHeight() * 3);					
					imageSaver.next();
				}
			}
			measureCamFps();
			cameraFrameNum++;
		}
	}
}

void captureApp::setup(){
	hidden = false;
	lastEnableCamera = false;
	cameraFrameNum = 0;
	sdk			= NULL;
	settings	= NULL;

	threePhase.setSize(1024, 768);
	threePhase.setWavelength(64);
	threePhase.generate();

	grayCode.setSize(1024, 768);
	grayCode.setLength(1024);
	grayCode.setSubdivisions(10);
	grayCode.generate();

	twoPlusOne.setSize(1024, 768);
	twoPlusOne.setWavelength(64);
	twoPlusOne.generate();

	gradient.setSize(1024, 768);
	gradient.generate();

	// setup panel
	panel.setup("control", 0, 0, 300, 768);
	panel.addPanel("capture", 1);
	panel.addPanel("three phase", 1);
	panel.addPanel("gray code", 1);

	panel.setWhichPanel("capture");
	
	vector <string> camModes;
	camModes.push_back("camera off");
	camModes.push_back("camera qt");
	camModes.push_back("camera dc1394");
	
	panel.addMultiToggle("camera mode", "camMode", 0, camModes);
	
	panel.addToggle("camera settings", "cameraSettings", false);
	panel.addToggle("fullscreen", "fullscreen", false);
	panel.addToggle("frame by frame", "frameByFrame", false);

	vector<string> patternTypes;
	patternTypes.push_back("three phase");
	patternTypes.push_back("gray code");
	patternTypes.push_back("gradient");
	patternTypes.push_back("two + one");
	panel.addMultiToggle("pattern type", "patternType", 0, patternTypes);
	vector<string> orientations;
	orientations.push_back("vertical");
	orientations.push_back("horizonal");
	panel.addMultiToggle("orientation", "orientation", 0, orientations);
	panel.addToggle("reverse", "reverse", false);
	panel.addSlider("pattern rate", "patternRate", 1, 1, 6, true);
	panel.addSlider("camera rate", "cameraRate", 1, 1, 6, true);
	panel.addSlider("camera offset", "cameraOffset", 0, 0, 5, true);
	panel.addSlider("min brightness", "minBrightness", 0, 0, 255, true);
	panel.addToggle("use projector lut", "projectorLut", false);

	panel.setWhichPanel("three phase");
	panel.addSlider("wavelength", "wavelength", 64, 8, 512, true);

	panel.setWhichPanel("gray code");
	panel.addSlider("subdivisions", "subdivisions", 10, 1, 10, true);

	panel.loadSettings("controlCapture.xml");

	//overides 
	panel.setValueI("camMode", 0);
	panel.setValueI("fullscreen", 0);
	panel.setValueI("cameraSettings", 0);
	panel.setValueI("frameByFrame", 0);
	
	cameraWidth  = 640;
	cameraHeight = 480;
	int n = 10; // no patterns have more than 10 frames
	for(int i = 0; i < n; i++) {
		recent.push_back(ofImage());
		recent.back().allocate(
			cameraWidth,
			cameraHeight,
			OF_IMAGE_COLOR);
		needsUpdate.push_back(false);
	}
	curGenerator = &threePhase;

	int captureTime = 15;
	imageSaver.setup(cameraWidth, cameraHeight, captureTime * 60);

	ofSetVerticalSync(true);
	ofSetFrameRate(60.0);

	ofBackground(0, 0, 0);
}

void captureApp::update(){
	panel.update();
	
	// this is where an event/callback-based
	// control panel would be really helpful!
	int curWavelength = panel.getValueI("wavelength");
	if(curWavelength != lastWavelength) {
		threePhase.setWavelength(curWavelength);
		threePhase.generate();
		twoPlusOne.setWavelength(curWavelength);
		twoPlusOne.generate();
	}
	lastWavelength = curWavelength;

	int minBrightness = panel.getValueI("minBrightness");
	if (minBrightness != lastMinBrightness) {
		threePhase.setMinBrightness(minBrightness);
		threePhase.generate();
		twoPlusOne.setMinBrightness(minBrightness);
		twoPlusOne.generate();
	}
	lastMinBrightness = minBrightness;

	int curOrientation = panel.getValueI("orientation");
	if(curOrientation != lastOrientation) {
		phaseOrientation orientation = curOrientation == 0 ? PHASE_VERTICAL : PHASE_HORIZONTAL;
		threePhase.setOrientation(orientation);
		threePhase.generate();
		grayCode.setOrientation(orientation);
		grayCode.generate();
		gradient.setOrientation(orientation);
		gradient.generate();
		twoPlusOne.setOrientation(orientation);
		twoPlusOne.generate();
	}
	lastOrientation = curOrientation;

	int curSubdivisions = panel.getValueI("subdivisions");
	if(curSubdivisions != lastSubdivisions) {
		grayCode.setSubdivisions(curSubdivisions);
		grayCode.generate();
	}
	lastSubdivisions = curSubdivisions;

	threePhase.setReverse(panel.getValueB("reverse"));
	twoPlusOne.setReverse(panel.getValueB("reverse"));
	grayCode.setReverse(panel.getValueB("reverse"));

	int curFullscreen = panel.getValueB("fullscreen");
	if(curFullscreen !=	lastFullscreen)
		ofSetFullscreen(curFullscreen);
	lastFullscreen = curFullscreen;

	//-- allow switching between QT and firewire
	bool curEnableCamera = false;
	int currentCamMode = panel.getValueI("camMode");
	
	if( preCamMode == currentCamMode ){
		curEnableCamera = (bool)currentCamMode;
	}else if( currentCamMode == 0 || (currentCamMode == 1 && do1394) || (currentCamMode == 2 && !do1394) ){
		curEnableCamera = false;		
	}else{
		curEnableCamera = true;
		if( currentCamMode == 2 ){
			do1394 = true;
		}else{
			do1394 = false;
		}
		preCamMode = currentCamMode;
	}
	
	//-- end that shit
		
	if(curEnableCamera != lastEnableCamera) {
		if(curEnableCamera) {
			
			if(do1394){
				camera1394.setDeviceID(0);
				sdk = new Libdc1394Grabber();
				//sdk->setFormat7(VID_FORMAT7_1);
				sdk->listDevices();
				sdk->setDiscardFrames(false);
				//sdk->set1394bMode(true);
				//sdk->setROI(0,0,320,200);
				//sdk->setDeviceID("b09d01008bc69e:0");
				settings = new ofxIIDCSettings;
				camera1394.setVerbose(true);
				camera1394.initGrabber( cameraWidth, cameraHeight, VID_FORMAT_Y8, VID_FORMAT_RGB, 60, true, sdk, settings );
				settings->setXMLFilename("mySettingsFile.xml");
				settings->panel.setPosition(318, 136);
			}else{
				camera.setup(cameraWidth, cameraHeight, this);
				camera.setDesiredFrameRate(60);
			}
		} else {
			if( do1394 ){
				camera1394.close();
				if( settings != NULL ){
					delete settings;
					delete sdk;
					settings = NULL;
					sdk		 = NULL;
					do1394   = false;
				}
			}else{
				camera.stopThread();
				camera.close();
			}
		}
	}
	
	if( do1394 && lastEnableCamera){
		update1394Cam();
	}
	
	lastEnableCamera = curEnableCamera;

	if(panel.getValueB("cameraSettings")) {
		if( !do1394 )camera.videoSettings();
		panel.setValueB("cameraSettings", false);
	}

	int curPatternType = panel.getValueI("patternType");
	if(curPatternType != lastPatternType) {
		switch(curPatternType) {
			case 0: curGenerator = &threePhase; break;
			case 1: curGenerator = &grayCode; break;
			case 2: curGenerator = &gradient; break;
			case 3: curGenerator = &twoPlusOne; break;
		}
	}
	lastPatternType = curPatternType;

	if(panel.getValueB("projectorLut")) {
		curGenerator->applyLut(ofToDataPath("projector-lut.tsv"));
		panel.setValueB("projectorLut", false);
	}
}

void captureApp::draw(){
	if(!panel.getValueB("frameByFrame")){
		patternFrame = ofGetFrameNum() / panel.getValueI("patternRate");
	}
	curGenerator->get(patternFrame).draw(0, 0);
	if(!hidden) {
		int cameraRate = panel.getValueI("cameraRate");
		int cameraOffset = panel.getValueI("cameraOffset");
		ofPushStyle();
		ofPushMatrix();
		int padding = 8;
		ofTranslate(panel.getWidth() + padding * 2, padding);
		int recentWidth = 160;
		int recentHeight = 120;
		
		if( panel.getValueB("frameByFrame") ){
			recentWidth		= 640;
			recentHeight	= 480;
		}
		
		for(int i = 0; i < curGenerator->size(); i++) {
			ofPushMatrix();
			ofTranslate(
				(i % 3) * (recentWidth + padding),
				(i / 3) * (recentHeight + padding));
			ofSetColor(255, 255, 255);
			if(needsUpdate[i]) {
				recent[i].update();
				needsUpdate[i] = false;
			}
			recent[i].draw(0, 0, recentWidth, recentHeight);
			ofSetColor(255, 0, 0);
			ofNoFill();
			ofRect(0, 0, recentWidth, recentHeight);
			ofPopMatrix();
		}
		ofPopMatrix();
		ofPopStyle();
		
		panel.draw();
		
		if( do1394 && lastEnableCamera){
			settings->draw();
		}
		if( lastEnableCamera ){
			ofSetColor(240, 10, 70);
			ofDrawBitmapString("cam fps: "+ofToString(camFps, 2), 600, 20);
			ofSetColor(255, 255, 255, 255);
		}
	
	}
}

//--------------------------------------------------------------
void captureApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void captureApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void captureApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
}

//--------------------------------------------------------------
void captureApp::keyPressed(int key) {
	if(key == 'f') {
		ofToggleFullscreen();
	}
	if(key == '\t') {
	
		if( !hidden &&  ofxFileHelper::doesFileExist(capturePrefix+"capture") && ofxFileHelper::doesFileExist(capturePrefix+"capture/0.jpg") ){
			string unique = ofToString(ofGetYear()) + ofToString(ofGetMonth()) + ofToString(ofGetDay()) + ofToString(ofGetHours()) + ofToString(ofGetMinutes()) + ofToString(ofGetSeconds());
			ofxFileHelper::moveFromTo(capturePrefix+"capture", capturePrefix+"savedFolder"+unique);
			ofxFileHelper::makeDirectory(capturePrefix+"capture");
		}
	
		hidden = !hidden;
		panel.hidden = hidden;
		if(hidden)
			ofHideCursor();
		else
			ofShowCursor();
		if(!hidden)
			imageSaver.saveAll();
	}
	if(panel.getValueB("frameByFrame") && key == OF_KEY_LEFT){
		patternFrame--;
	}

	if(panel.getValueB("frameByFrame") && key == OF_KEY_RIGHT){
		patternFrame++;
	}
}
