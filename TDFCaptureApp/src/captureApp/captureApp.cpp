#include "captureApp.h"

static bool do1394			= false;
string capturePrefix		= CAPTURE_MAIN_FOLDER;
string currentCaptureFolder = capturePrefix + FRAME_CAPTURE_FOLDER;

static float lastTime	= 0.0;
static float camFps		= 60.0f;
int preCamMode			= 0;
float waitTillTime		= 0.0;

#include "decodeApp.h"
extern decodeApp  * dAppPtr;

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
					string filename = currentCaptureFolder + ofToString(cameraFrameNum) + ".jpg";
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
					string filename = currentCaptureFolder + ofToString(cameraFrameNum) + ".jpg";
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

	// setup panel
	panel.setup("control", 0, 0, 300, 768);
	panel.addPanel("app settings", 1);
	panel.addPanel("pattern settings", 1);
	panel.addPanel("face trigger settings", 1);

	panel.setWhichPanel("app settings");
	
	vector <string> camModes;
	camModes.push_back("camera off");
	camModes.push_back("camera qt");
	camModes.push_back("camera dc1394");
	
	panel.addMultiToggle("camera mode", "camMode", 0, camModes);
	
	panel.addToggle("camera settings", "cameraSettings", false);
	panel.addToggle("fullscreen", "fullscreen", false);
	panel.addToggle("frame by frame", "frameByFrame", false);
	panel.addToggle("large video", "largeVideo", false);

	vector<string> postCapModes;
	postCapModes.push_back("save frames");
	postCapModes.push_back("decode and frames");
	postCapModes.push_back("decode, export and frames");
	postCapModes.push_back("all above + notify");
	panel.addMultiToggle("post capture:", "postCapture", 2, postCapModes);
	
	panel.addSlider("capture time f", "CAPTURE_TIME_F", 4.0, 2.0, 15.0, false);


	panel.setWhichPanel("pattern settings");

	panel.addSlider("pattern rate", "patternRate", 1, 1, 6, true);
	panel.addSlider("camera rate", "cameraRate", 1, 1, 6, true);
	panel.addSlider("camera offset", "cameraOffset", 0, 0, 5, true);

	panel.addToggle("reverse", "reverse", false);

	vector<string> orientations;
	orientations.push_back("vertical");
	orientations.push_back("horizonal");
	panel.addMultiToggle("orientation", "orientation", 0, orientations);

	panel.addSlider("min brightness", "minBrightness", 0, 0, 255, true);
	panel.addSlider("max brightness", "maxBrightness", 255, 0, 255, true);	
	panel.addSlider("3 phase - wavelength", "wavelength", 64, 8, 512, true);
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
	panel.setValueI("largeVideo", 0);
	
	cameraWidth  = 640;
	cameraHeight = 480;
	
	int n = 3; // no patterns have more than 3 frames
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
		if( ofGetFrameNum() % 4 == 0 )printf("fps is %f\n", ofToString(camFps, 2).c_str());
	}else if( debugState == CAP_DEBUG ){
		panel.hidden = false;
	}
	
	if( state == CAP_STATE_CAPTURE && ofGetElapsedTimef() >= timeToEndCapture ){
		printf("time is %f - time to end is %f\n", ofGetElapsedTimef(), timeToEndCapture);
		endCapture();		
		if( panel.getValueB("B_FACE_TRIGGER") ){
			bNeedsToLeaveFrame = true;
		}else{
			bNeedsToLeaveFrame = false;
		}
	}
	
	if( state == CAP_STATE_DECODING ){
		handleDecode();
	}else{	
		handleProjection();
		handleCamera();
		handleFaceTrigger();

		if(panel.getValueB("projectorLut")) {
			curGenerator->applyLut(ofToDataPath("projector-lut.tsv"));
			panel.setValueB("projectorLut", false);
		}
	}
	
	panel.clearAllChanged();

}

//-----------------------------------------------
void captureApp::startDecode(){
	printf("startDecode\n");
	
	if( imageSaver.getSize() ){
		state = CAP_STATE_DECODING;
	
		saveIndex = 0;
		decoder.setupDecoder(imageSaver.width, imageSaver.height);
		decoder.setSettings(dAppPtr->panel.getValueF("gamma"), dAppPtr->panel.getValueF("depthScale"), dAppPtr->panel.getValueF("depthSkew"), dAppPtr->panel.getValueF("rangeThreshold"), dAppPtr->panel.getValueI("orientation"), dAppPtr->panel.getValueB("phasePersistence"));

		if( !ofxFileHelper::doesFileExist("output/incoming/") ){
			ofxFileHelper::makeDirectory("output/incoming/");
		}

	}else{
		ofLog(OF_LOG_ERROR, "startDecode - no images to decode");
	}
	
}

//-----------------------------------------------
void captureApp::handleDecode(){
	if( state == CAP_STATE_DECODING ){

		if( saveIndex < imageSaver.getSize() ){
			float filterMin =  dAppPtr->panel.getValueF("filterMin");
			float filterMax =  dAppPtr->panel.getValueF("filterMax");
			float smoothAmnt=  dAppPtr->panel.getValueF("smooth_y_amnt");
			int   smoothDist=  dAppPtr->panel.getValueI("smooth_y_dist");
			
			printf("decoding %i of %i\n", saveIndex, imageSaver.getSize());
			decoder.decodeFrameAndFilter(imageSaver.images[saveIndex], saveIndex, 3, filterMin, filterMax, smoothAmnt, smoothDist);
			
			bool bSaveToDisk = ( panel.getValueI("postCapture") >= POST_CAPTURE_DECODE_EXPORT );
			
			if( saveIndex % 3 == 0 && bSaveToDisk ){
				decoder.exportFrameToTGA(DECODE_FOLDER, 10000+saveIndex);
			}
			
			saveIndex++;
		}else{
			endDecode();
		}

	}
}

//-----------------------------------------------
void captureApp::endDecode(){
	printf("endDecode\n");
	if( state == CAP_STATE_DECODING ){
		//in this case we want to copy decoded files and notify viz app first then export frames last. 
		if( panel.getValueI("postCapture") == POST_CAPTURE_ALL_AND_NOTIFY ){
			
		}else{
			exportFramesToDisk();
		}
	}
}

//-----------------------------------------------
void captureApp::exportFramesToDisk(){
	if( imageSaver.getSize() >  0){
		state = CAP_STATE_SAVING;

		//backup the current capture folder
		if( ofxFileHelper::doesFileExist(capturePrefix+FRAME_CAPTURE_NAME) ){
			string unique = getTimeAsUniqueString();
			ofxFileHelper::moveFromTo(capturePrefix+FRAME_CAPTURE_NAME, capturePrefix+"savedFolder"+unique);
		}
		
		//make a new folder with a settings folder inside
		ofxFileHelper::makeDirectory(currentCaptureFolder);
		ofxFileHelper::makeDirectory(currentCaptureFolder+"_settings");		

		//save the current capture and decode settings to the settings folder
		panel.saveSettings(currentCaptureFolder+"_settings/captureSettings.xml");
		dAppPtr->panel.saveSettings(currentCaptureFolder+"_settings/decodeSettings.xml");
	
		startThread(false, true); //now save jpegs in background ! - this calls threadedFunction ( below )
	}else{
		ofLog(OF_LOG_ERROR, "exportFramesToDisk no frames to export");
	}
}

//-----------------------------------------------
void captureApp::threadedFunction(){
	if( lock() ){
	
		printf("threadedFunction - saving started! at %f since app start\n",ofGetElapsedTimef()); 
		imageSaver.saveAll();
		printf("threadedFunction - saving done! at %f since app start\n",ofGetElapsedTimef()); 		
		state = CAP_STATE_WAITING;
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
		state = CAP_STATE_CAPTURE;

		//clear the image saver buffer
		imageSaver.clear();
		cameraFrameNum = 0;

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
		ofShowCursor();
		
		if( panel.getValueI("postCapture") > POST_CAPTURE_SAVE ){
			startDecode();
		}else{
			exportFramesToDisk();
		}
		
	}else{
		printf("ERROR sorry - we weren't capturing!!!!!\n");
	}
	
}

//--------------------------------------------------------
void captureApp::handleProjection(){
		// this is where an event/callback-based
	// control panel would be really helpful!
		
	if( panel.hasValueChanged("wavelength") ){
		int curWavelength = panel.getValueI("wavelength");
		threePhase.setWavelength(curWavelength);
		threePhase.generate();
	}

	if ( panel.hasValueChanged("minBrightness") || panel.hasValueChanged("maxBrightness") ){	
		threePhase.setMinBrightness(panel.getValueI("minBrightness"));
		threePhase.setMaxBrightness(panel.getValueI("maxBrightness"));
		threePhase.generate();
	}

	if(panel.hasValueChanged("orientation") ) {
		int curOrientation = panel.getValueI("orientation");
		phaseOrientation orientation = curOrientation == 0 ? PHASE_VERTICAL : PHASE_HORIZONTAL;
		threePhase.setOrientation(orientation);
		threePhase.generate();
	}

	threePhase.setReverse(panel.getValueB("reverse"));

	if( panel.hasValueChanged("fullscreen") ) {
		ofSetFullscreen(panel.getValueB("fullscreen"));
	}
	
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
					//camera1394.setVerbose(true);
					camera1394.initGrabber( cameraWidth, cameraHeight, VID_FORMAT_YUV422, VID_FORMAT_RGB, 60.0, false, sdk, settings );
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
		
		if( panel.hasValueChanged("B_FACE_TRIGGER") && panel.getValueB("B_FACE_TRIGGER") ){
				bNeedsToLeaveFrame = false;
				face.resetCounters();
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



//-------------------------------------------------------------
void captureApp::draw(){

	if( state == CAP_STATE_DECODING ){
		camera3D.place();
		ofBackground(0, 0, 0);
		glEnable(GL_DEPTH_TEST);
			ofPushMatrix();
				ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);		
				decoder.drawCloud();
			ofPopMatrix();
			glDisable(GL_DEPTH_TEST);			
		camera3D.remove();	
		decoder.drawCurrentFrame(0, 0, 320, 240);
		ofNoFill();
		ofSetColor(255, 255, 255, 255);
		ofRect( ofGetWidth()/4, ofGetHeight()-150, ofGetWidth()/2, 60);	
		ofFill();
		ofRect( ofGetWidth()/4, ofGetHeight()-150, ofMap(saveIndex, 0, imageSaver.getSize(), 0.0, 1.0, true) * (float)(ofGetWidth()/2), 60);			
		return;
	}

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
		
		if( panel.getValueB("largeVideo") ){
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
	
	if(panel.getValueB("frameByFrame") && key == OF_KEY_UP){
		patternFrame--;
	}

	if(panel.getValueB("frameByFrame") && key == OF_KEY_DOWN){
		patternFrame++;
	}
}
