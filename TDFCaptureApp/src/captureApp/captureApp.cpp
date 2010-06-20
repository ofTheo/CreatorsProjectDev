#include "captureApp.h"

static bool do1394 = false;
string capturePrefix = "input/";

static float lastTime = 0.0;
static float camFps = 60.0f;
int preCamMode = 0;
float waitTillTime = 0.0;

//--------------------------------------------------------
void measureCamFps(){
	float timeDiff = ofGetElapsedTimef()-lastTime;
	lastTime = ofGetElapsedTimef();
		
	float fps = 1.0/ofClamp(timeDiff, 0.01, 1000.0);
	camFps *= 0.95;
	camFps += 0.05 * fps;
}

//--------------------------------------------------------
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
				if(state != CAP_STATE_CAPTURE ) {
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

//--------------------------------------------------------
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
				if(state != CAP_STATE_CAPTURE) {
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

//--------------------------------------------------------
void captureApp::setup(){
	state			= CAP_STATE_WAITING;
	debugState		= CAP_DEBUG;
	camState		= CAMERA_CLOSED;
	prevCamState	= CAMERA_CLOSED;

	preCamMode			= 0;
	cameraFrameNum		= 0;
	sdk					= NULL;
	settings			= NULL;
	timeToEndCapture	= 0.0;
	bNeedsToLeaveFrame	= false;

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
	panel.addPanel("extra settings", 1);
	panel.addPanel("face trigger settings", 1);

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
	panel.addSlider("capture time f", "CAPTURE_TIME_F", 10.0, 2.0, 45.0, false);

	panel.setWhichPanel("extra settings");
	panel.addSlider("3 phase - wavelength", "wavelength", 64, 8, 512, true);
	panel.addSlider("gray code - subdivisions", "subdivisions", 10, 1, 10, true);
	panel.addToggle("use projector lut", "projectorLut", false);

	panel.setWhichPanel("face trigger settings");
	panel.addToggle("use face trigger", "B_FACE_TRIGGER", false);
	panel.addSlider("face trigger fps", "FACE_FPS", 10, 3, 30, false);
	panel.addSlider("face detected time", "FACE_DETECT_TIME", 2.0, 0.1, 10.0, false);
	panel.addSlider("face forget time", "FACE_FORGET_TIME", 2.0, 0.1, 10.0, false);
	
	guiStatVarPointer varPtr;
	varPtr.setup("num faces", &face.numFaces, GUI_VAR_INT, true);
	panel.addChartPlotter("num faces", varPtr, 240, 60, 240, -1, 3);
	panel.addChartPlotter("face confidence", guiStatVarPointer("confidence", &face.confidence, GUI_VAR_FLOAT, true, 2), 240, 70, 240, -0.2, 1.2);

	panel.addDrawableRect("state", &face, 240, 40);
		
	panel.addSlider("confidence add amnt", "confidence_add", 0.1, 0.05, 1.0, false);
	panel.addSlider("confidence fade amnt", "confidence_fade", 0.95, 0.8, 0.995, false);
	panel.addSlider("confidence gate start val", "confidence_gate_start", 0.65, 0.15, 1.0, false);
	panel.addSlider("confidence gate stop val", "confidence_gate_stop", 0.4, 0.0, 1.0, false);

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
	
	face.setup("faceHaar/haarcascade_frontalface_alt.xml", 160, 120, 2.0, 2.0);
	prevFaceCheckTimeF = ofGetElapsedTimef();
}

//-----------------------------------------------
void captureApp::update(){
	panel.update();
	
	if( state == CAP_STATE_CAPTURE ){
		panel.hidden = true;
	}else if( debugState == CAP_DEBUG ){
		panel.hidden = false;
	}
	
	if( state == CAP_STATE_CAPTURE && ofGetElapsedTimef() >= timeToEndCapture ){
		printf("time is %f - time to end is %f\n", ofGetElapsedTimef(), timeToEndCapture);
		
		endCapture();
		
		if( panel.getValueB("B_FACE_TRIGGER") ){
			bNeedsToLeaveFrame = true;
		}
	}
	
	if( state == CAP_STATE_NOTIFY ){
		//
		printf("fake notify and copy files here!\n");
		state = CAP_STATE_WAITING;
	}
	
	handleProjection();
	handleCamera();
	handleFaceTrigger();

	if(panel.getValueB("projectorLut")) {
		curGenerator->applyLut(ofToDataPath("projector-lut.tsv"));
		panel.setValueB("projectorLut", false);
	}
}

//--------------------------------------------------------
void captureApp::handleProjection(){
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
}

//--------------------------------------------------------
void captureApp::handleCamera(){
	
	if( ofGetElapsedTimef() > waitTillTime ){
	
		if( camState == CAMERA_OPEN && do1394 ){
			update1394Cam();
		}

		if( camState == CAMERA_OPEN && panel.getValueB("cameraSettings")) {
			if( !do1394 )camera.videoSettings();
			panel.setValueB("cameraSettings", false);
		}

		//-- allow switching between QT and firewire
		int requestedMode = panel.getValueI("camMode");
		
		if( preCamMode == requestedMode ){  // don't do anything
			//
		}else if( requestedMode == 0 && preCamMode > 0 ){ //if the camera is on but we want if off set closed
			preCamMode = 0;
			camState = CAMERA_NEEDS_CLOSING;
		}else if( requestedMode == 1 && preCamMode == 2 ){
			preCamMode = 0;
			//switching is too buggy - so we only allow to close. 
			panel.setValueI("camMode", 0);			
			camState = CAMERA_NEEDS_CLOSING;			
		}else if( requestedMode == 2 && preCamMode == 1 ){
			preCamMode = 0;
			//switching is too buggy - so we only allow to close. 
			panel.setValueI("camMode", 0);			
			camState = CAMERA_NEEDS_CLOSING;
		}else{
			camState = CAMERA_NEEDS_OPENING;
			if( requestedMode == 2 ){
				do1394 = true;
			}else{
				do1394 = false;
			}
			preCamMode = requestedMode;
		}
		
		//-- end that shit
			
		if(camState != prevCamState) {
			
			if(camState == CAMERA_NEEDS_OPENING) {
				
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
				
				camState = CAMERA_OPEN;

			}else if( camState == CAMERA_NEEDS_CLOSING ){
				
				if( do1394 ){
					printf("closing camera\n");
					camera1394.close();
					if( settings != NULL ){
						delete settings;
						delete sdk;
						printf("deleting settings and sdk\n");
						settings = NULL;
						sdk		 = NULL;
						do1394   = false;
					}
				}else{
					camera.stopThread();
					camera.close();
				}
				
				camState	 = CAMERA_CLOSED;
				waitTillTime = ofGetElapsedTimef() + 5.0; //wait 5 seconds before allowing next camera change. 
			}
		}
	
		prevCamState = camState;	
	}	
}

//-----------------------------------------------
void captureApp::handleFaceTrigger(){
	
	if( camState != CAMERA_CLOSED && state != CAP_STATE_CAPTURE ){
	
		float faceFps  = panel.getValueF("FACE_FPS");
		float timeDiff = 1.0/ofClamp(faceFps, 4.0, 60.0);
		
		face.confidenceAddAmnt  = panel.getValueF("confidence_add");
		face.confidenceFadeRate = panel.getValueF("confidence_fade");
		face.confidenceGateValStart  = panel.getValueF("confidence_gate_start");
		face.confidenceGateValStop  = panel.getValueF("confidence_gate_stop");
		
		if( ofGetElapsedTimef() - prevFaceCheckTimeF >= timeDiff ){
	
			unsigned char * pix = NULL;
			if( do1394 ){
				pix = camera1394.getPixels();
			}else{
				pix = camera.getPixels();
			}
			if(pix != NULL){
				face.update(pix, cameraWidth, cameraHeight);
			}
			
			prevFaceCheckTimeF = ofGetElapsedTimef();
		}
		
		if( bNeedsToLeaveFrame ){
			if( face.getState() == FACE_NONE ){
				bNeedsToLeaveFrame = false;
			}
		}else{		
			//only trigger a capture if the flag is set!!!
			if( panel.getValueB("B_FACE_TRIGGER") && face.firstSawFace() ){
				face.clearFirstSawFace();
				startCapture();
			}
		}
	}

}

//-----------------------------------------------
void captureApp::threadedFunction(){
	if( lock() ){
		printf("saving started! at %f since app start\n",ofGetElapsedTimef()); 
		imageSaver.saveAll();
		state = CAP_STATE_NOTIFY;
		printf("saving done! at %f since app start\n",ofGetElapsedTimef()); 
		unlock();
	}
}

//-----------------------------------------------
void captureApp::startCapture(){
	printf("startCapture\n");

	if( camState == CAMERA_CLOSED ){
		printf("ERROR - OPEN CAMERA STUPID!\n");
		return;
	}

	if( state <= CAP_STATE_CAPTURE ){
	
		if( ofxFileHelper::doesFileExist(capturePrefix+"capture") ){
			string unique = ofToString(ofGetYear()) + ofToString(ofGetMonth()) + ofToString(ofGetDay()) + ofToString(ofGetHours()) + ofToString(ofGetMinutes()) + ofToString(ofGetSeconds());
			ofxFileHelper::moveFromTo(capturePrefix+"capture", capturePrefix+"savedFolder"+unique);
		}
		
		ofxFileHelper::makeDirectory(capturePrefix+"capture");
		ofxFileHelper::makeDirectory(capturePrefix+"capture/_settings");
		
		cameraFrameNum = 0;
		
		state = CAP_STATE_CAPTURE;
		ofHideCursor();
		printf("time is %f time to end is %f\n",ofGetElapsedTimef(), panel.getValueF("CAPTURE_TIME_F"));
		timeToEndCapture = ofGetElapsedTimef() + panel.getValueF("CAPTURE_TIME_F");
	}else{
		printf("ERROR - sorry - you can't capture yet!\n");
		if( state == CAP_STATE_SAVING ){
			printf("we are still saving!\n");
		}
	}
	
}

//-----------------------------------------------
void captureApp::endCapture(){
	printf("endCapture\n");
	if( state == CAP_STATE_CAPTURE ){
		(int)state = (int)state + 1;
		ofShowCursor();
	
		if(state == CAP_STATE_SAVING){
			if( !ofxFileHelper::doesFileExist(capturePrefix+"capture/_settings") ){
				ofxFileHelper::makeDirectory(capturePrefix+"capture/_settings");
			}
			panel.saveSettings(capturePrefix+"capture/_settings/captureSettings.xml");
		
			//do threaded image saving!
			startThread(false, true);
		}	
	}else{
		printf("ERROR sorry - we weren't capturing!!!!!\n");
	}
	
}


void captureApp::draw(){
	if(!panel.getValueB("frameByFrame")){
		patternFrame = ofGetFrameNum() / panel.getValueI("patternRate");
	}
	curGenerator->get(patternFrame).draw(0, 0);
	
	if(state != CAP_STATE_CAPTURE && debugState == CAP_DEBUG) {
		int cameraRate   = panel.getValueI("cameraRate");
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
		
		if( do1394 && camState == CAMERA_OPEN ){
			settings->draw();
		}
		if( camState == CAMERA_OPEN  ){
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
		if( state < CAP_STATE_CAPTURE ){
			startCapture();
		}
		else if( state == CAP_STATE_CAPTURE ){
			endCapture(); //force override - normally capture is timed. 
		}
	}
	
	if(panel.getValueB("frameByFrame") && key == OF_KEY_LEFT){
		patternFrame--;
	}

	if(panel.getValueB("frameByFrame") && key == OF_KEY_RIGHT){
		patternFrame++;
	}
}
