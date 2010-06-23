#include "captureApp.h"

static bool do1394			= false;
string capturePrefix		= CAPTURE_MAIN_FOLDER;

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
					string filename = currentCaptureFolder + ofToString(FRAME_START_INDEX+cameraFrameNum) + ".jpg";
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
					string filename = currentCaptureFolder + ofToString(FRAME_START_INDEX+cameraFrameNum) + ".jpg";
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
	bOscSetup			= false;
	fadeInStartTime		= 0.0;
	spotLightAlpha		= 1.0;
	bDoThreadedRSync		= false;
	bDoThreadedFrameSave	= false;
	
	camera3D.disableMouseEvents();

	threePhase.setSize(1024, 768);
	threePhase.setWavelength(64);
	threePhase.generate();
	
	ofxXmlSettings xml;
	xml.loadFile("locationSettings.xml");
	currentCity			= xml.getValue("city_name", "NYC");
	transformSpaces(currentCity); //remove space and add underscores
	currentDecodeFolder = DECODE_FOLDER;
	currentCaptureFolder   = string(CAPTURE_MAIN_FOLDER) + CAPTURE_MAIN_FOLDER;
	currentTimestamp    = (ofxTimeStamp()).getUnixTimeAsString();
	
	// setup panel
	panel.setup("control", 0, 0, 300, 768);
	//panel.loadFont("resources/myFont.ttf", 9);
	panel.addPanel("app settings", 1);
	panel.addPanel("pattern settings", 1);
	panel.addPanel("face trigger settings", 1);
	panel.addPanel("spotlight settings", 1);

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
	panel.addToggle("brightness setting", "brightnessSetting", false);
	panel.addSlider("check brigheness", "checkBrightness", 0, 0, 255, true);
	panel.addSlider("fade in time", "fadeInTime", 2.0, 0.0, 5.0, false);

	vector<string> postCapModes;
	postCapModes.push_back("save frames");
	postCapModes.push_back("decode and frames");
	postCapModes.push_back("decode, export and frames");
	postCapModes.push_back("all above + notify");
	panel.addMultiToggle("post capture:", "postCapture", 2, postCapModes);

	panel.addToggle("notify with osc", "use_osc", false);

	panel.addSlider("decode skip frames", "decodeSkipFrame", 2, 0, 5, true);
	
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
	
	panel.setWhichPanel("spotlight settings");
	panel.addToggle("spot light image", "bSpotLight", true);
	panel.addSlider("spotlight %", "spotLightBrightness", 1.0, 0.0, 1.0, false);

	panel.loadSettings("controlCapture.xml");
	
	if( panel.getValueB("use_osc") ){
		setupOsc();
	}

	//overides 
	panel.setValueI("camMode", 0);
	panel.setValueI("fullscreen", 0);
	panel.setValueI("cameraSettings", 0);
	panel.setValueI("frameByFrame", 0);
	panel.setValueI("largeVideo", 0);
	panel.setValueI("brightnessSetting",0);
	
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
	
	scanningSound.loadSound("resources/scanningPlaceholder.mp3");
	scanningSound.setLoop(true);
	
	spotLightImage.loadImage("resources/captureMask.jpg");
	spotLightImage.setAnchorPercent(0.5, 0.5);
	
	face.setup("faceHaar/haarcascade_frontalface_alt.xml", 160, 120, 2.0, 2.0);
	prevFaceCheckTimeF = ofGetElapsedTimef();
}

//-----------------------------------------------
void captureApp::setupOsc(){
	if( !bOscSetup ){
		//TODO: do OSC!
		
		ofxXmlSettings xml;
		if( xml.loadFile("notification.xml") ){
			int remotePort		= xml.getValue("remote_osx_port", 70007);
			string remoteIp     = xml.getValue("remote_ip", "127.0.0.1");
			oscTx.setup(remoteIp, remotePort);
			bOscSetup = true;
		}
		
	}
}

//-----------------------------------------------
void captureApp::update(){
	panel.update();

	bEnableOsc = panel.getValueB("use_osc");
	if( panel.hasValueChanged("use_osc") ){
		if( bEnableOsc && !bOscSetup ){
			setupOsc();
		}
	}
	
	//the capture part happens in the camera callbacks at the top.
	//this just checks to make sure that the capture doesn't need to keep running.
	if( state == CAP_STATE_CAPTURE ){
		panel.hidden = true;
		if( ofGetFrameNum() % 4 == 0 ){
			printf("fps is %s\n", ofToString(camFps, 2).c_str());
		}
		if( ofGetElapsedTimef() >= timeToEndCapture ){
			printf("time is %f - time to end is %f\n", ofGetElapsedTimef(), timeToEndCapture);
			endCapture();		
		}
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
		
	if( state == CAP_STATE_FADEIN && ofGetElapsedTimef() > fadeInStartTime + panel.getValueF("fadeInTime") ){
		startCapture();
	}	
	
	if( state == CAP_STATE_DECODING ){
		handleDecode();
	}else{	
		handleProjection();
		handleCamera();
		handleFaceTrigger();
	}
	
	panel.clearAllChanged();
}

//-----------------------------------------------
void captureApp::startFadeIn(){
	if( panel.getValueF("fadeInTime") > 0 ){
		state		= CAP_STATE_FADEIN;
		fadeInStartTime  = ofGetElapsedTimef();
	}else{
		startCapture();
	}
}

//-----------------------------------------------
void captureApp::startDecode(){
	printf("startDecode\n");
	
	if( imageSaver.getSize() ){
		state = CAP_STATE_DECODING;
	
		timeToDecode = 0.0;
		saveIndex = 0;
		saveCount = 0;
		decoder.setupDecoder(imageSaver.width, imageSaver.height);
		decoder.setSettings(dAppPtr->panel.getValueF("gamma"), dAppPtr->panel.getValueF("depthScale"), dAppPtr->panel.getValueF("depthSkew"), dAppPtr->panel.getValueF("rangeThreshold"), dAppPtr->panel.getValueI("orientation"), dAppPtr->panel.getValueB("phasePersistence"));

		if( !ofxFileHelper::doesFileExist(currentDecodeFolder) ){
			ofxFileHelper::makeDirectory(currentDecodeFolder);
		}else if( !ofxFileHelper::isDirectoryEmpty(currentDecodeFolder) ){
			ofxFileHelper::moveFromTo( ofxFileHelper::removeTrailingSlash(currentDecodeFolder), string(EXPORT_FOLDER) + "savedDecode"+currentTimestamp );
			ofxFileHelper::makeDirectory(currentDecodeFolder);			
		}
		
	}else{
		ofLog(OF_LOG_ERROR, "startDecode - no images to decode");
	}
	
}

//-----------------------------------------------
void captureApp::handleDecode(){
	if( state == CAP_STATE_DECODING ){
	
		if( !ofxFileHelper::doesFileExist(currentDecodeFolder) ){
			ofLog(OF_LOG_ERROR, "handleDecode - decode folder not found - can't save files!\n");
		}

		float filterMin =  dAppPtr->panel.getValueF("filterMin");
		float filterMax =  dAppPtr->panel.getValueF("filterMax");
		float smoothAmnt=  dAppPtr->panel.getValueF("smooth_y_amnt");
		int   smoothDist=  dAppPtr->panel.getValueI("smooth_y_dist");
		
		int numMissed	= 1+ panel.getValueI("decodeSkipFrame");
		
		//you can changed this to a higher number to decode more frames per app frame. 
		for(int k = 0; k < numMissed; k++){
			if( saveIndex < imageSaver.getSize() ){
			
				printf("decoding %i of %i\n", saveIndex, imageSaver.getSize());
				decoder.decodeFrameAndFilter(imageSaver.images[saveIndex], saveIndex, 3, filterMin, filterMax, smoothAmnt, smoothDist);
				
				bool bSaveToDisk = ( panel.getValueI("postCapture") >= POST_CAPTURE_DECODE_EXPORT );
				
				if( saveIndex % ( numMissed ) == 0 && bSaveToDisk ){
					float t1 = ofGetElapsedTimef();
					decoder.exportFrameToTGA(currentDecodeFolder, 10000+saveIndex);
					timeToDecode += ofGetElapsedTimef()-t1;
					saveCount++;
				}
				
				saveIndex++;
			}else{
				endDecode();
				break;
			}
		}

	}
}

//-----------------------------------------------
void captureApp::endDecode(){
	printf("endDecode\n");
	if( state == CAP_STATE_DECODING ){
		state = CAP_STATE_END_DECODE;
	}
	
	if( state == CAP_STATE_END_DECODE ){
		
		if( panel.getValueI("postCapture") >= POST_CAPTURE_DECODE_EXPORT ){
			printf("total time to save %i to tga is %f - average %f\n", saveCount, timeToDecode, (float)timeToDecode/saveCount);
		}
		
		//in this case we want to copy decoded files and notify viz app first then export frames last. 
		if( panel.getValueI("postCapture") == POST_CAPTURE_ALL_AND_NOTIFY ){

			printf("postCapture == POST_CAPTURE_ALL_AND_NOTIFY \n");
			prepareTransferFramesToVizApp();				
		}
		
		//either way, we export the frames. 
		prepareExportFramesToDisk();
		startThread(true, false);
	}
}

//TODO: clean this up? though current advantage is we can login and change it while the app is running. 
//-----------------------------------------------
void captureApp::prepareTransferFramesToVizApp(){
	ofxXmlSettings xml;
	xml.setVerbose(true);
	
	if( xml.loadFile("notification.xml") ){
		string command		= xml.getValue("command", "");
		string remoteUser   = xml.getValue("remote_user", "");
		string remoteip     = xml.getValue("remote_ip", "");

		string localFolder	= ofxFileHelper::removeTrailingSlash(ofToDataPath(currentDecodeFolder));
		
		printf("command is %s\n", command.c_str());
		
		//TODO: put this in threadedFunction?
		vector <string> explode = ofSplitString(command, "!");
		if( explode.size() >=5 ){
			executeStr = explode[0] + localFolder + explode[1] + remoteUser + explode[2] + remoteip + explode[3] + remoteUser + explode[4];
			
			bDoThreadedRSync = true;
			prepareExportFramesToDisk();
		}
	}else{
		bDoThreadedRSync = false;
		printf("error loading notification.xml");
	}
	
}

//-----------------------------------------------
void captureApp::prepareExportFramesToDisk(){
	if( imageSaver.getSize() >  0){
		state = CAP_STATE_SAVING;

		//backup the current capture folder
		if( ofxFileHelper::doesFileExist(currentCaptureFolder) ){
			ofxFileHelper::moveFromTo(ofxFileHelper::removeTrailingSlash(currentCaptureFolder), capturePrefix+"savedFolder"+currentTimestamp);
		}
		
		//make a new folder with a settings folder inside
		ofxFileHelper::makeDirectory(currentCaptureFolder);
		ofxFileHelper::makeDirectory(currentCaptureFolder+"_settings");		

		//save the current capture and decode settings to the settings folder
		panel.saveSettings(currentCaptureFolder+"_settings/captureSettings.xml");
		dAppPtr->panel.saveSettings(currentCaptureFolder+"_settings/decodeSettings.xml");
	
		bDoThreadedFrameSave = true;
	}else{
		bDoThreadedFrameSave = false;
		ofLog(OF_LOG_ERROR, "prepareExportFramesToDisk no frames to export");
	}
}

//-----------------------------------------------
void captureApp::threadedFunction(){
	if( lock() ){
		
		if( bDoThreadedRSync && executeStr != "" ){
			printf("executing rsync %s\n", executeStr.c_str());
			
			if( bEnableOsc ){
				ofxOscMessage m;
				m.addStringArg("TxStarted");
				m.addStringArg(currentCaptureFolder); //folder transferred eg: decode-NYC-12939327117
				m.addStringArg(currentTimestamp);	  //just the timestamp as a string eg: 12939327117
				m.addIntArg(saveCount);				  //num images to be transfered
				oscTx.sendMessage(m);
			}
			
			system(executeStr.c_str());
			
			if( bEnableOsc ){
				ofxOscMessage m;
				m.addStringArg("TxEnded");
				m.addStringArg(currentCaptureFolder);	
				m.addStringArg(currentTimestamp);		
				m.addIntArg(saveCount);		
				oscTx.sendMessage(m);
			}
		}
	
		if( bDoThreadedFrameSave ){
			printf("threadedFunction - saving started! at %f since app start\n",ofGetElapsedTimef()); 
			imageSaver.saveAll();
			printf("threadedFunction - saving done! at %f since app start\n",ofGetElapsedTimef()); 		
		}
		
		bDoThreadedFrameSave = false;
		bDoThreadedRSync     = false;
		state				 = CAP_STATE_WAITING;
		
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
		cameraFrameNum		 = 0;
		scanningSound.play();
		
		ofxTimeStamp ts;
		ts.setTimestampToCurrentTime();
		currentTimestamp	= ts.getUnixTimeAsString();
		currentDecodeFolder = EXPORT_FOLDER + string("decoded-") + currentCity + "-" + currentTimestamp + "/";
		currentCaptureFolder   = CAPTURE_MAIN_FOLDER + string("capture-") + currentCity + "-" + currentTimestamp + "/";
		printf("decoding to %s\n frames saved to %s\n", currentDecodeFolder.c_str(), currentCaptureFolder.c_str());			

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
		
		if( panel.getValueB("B_FACE_TRIGGER") ){
			bNeedsToLeaveFrame = true;
		}else{
			bNeedsToLeaveFrame = false;
		}

		scanningSound.stop();
		
		if( panel.getValueI("postCapture") > POST_CAPTURE_SAVE ){
			startDecode();
		}else{
			prepareExportFramesToDisk();
			startThread(true, false);
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

	if ( panel.hasValueChanged("minBrightness") || panel.hasValueChanged("maxBrightness") || 
			panel.hasValueChanged("projectorLut")) {	
		threePhase.setMinBrightness(panel.getValueI("minBrightness"));
		threePhase.setMaxBrightness(panel.getValueI("maxBrightness"));
		threePhase.generate();
		if(panel.getValueB("projectorLut") && ofxFileHelper::doesFileExist("projector-lut.csv"))
				curGenerator->applyLut(ofToDataPath("projector-lut.csv"));
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
			//we have to do this because ofxVideoGrabber uses events :(
			if( settings != NULL && state <= CAP_STATE_CAPTURE && debugState == CAP_DEBUG ){
				settings->panel.show();				
			}else{
				settings->panel.hide();
			}
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
					settings = new ofxIIDCSettings();
					//camera1394.setVerbose(true);
					if( camera1394.initGrabber( cameraWidth, cameraHeight, VID_FORMAT_YUV422, VID_FORMAT_RGB, 60.0, false, sdk, settings ) ){
						settings->setXMLFilename("mySettingsFile.xml");
						settings->panel.setPosition(318, 136);
						camState = CAMERA_OPEN;
					}else{
						printf("no camera to open\n");
						delete settings;
						delete sdk;
						settings = NULL;
						sdk		 = NULL;
						do1394   = false;
						camState = CAMERA_CLOSED;
						panel.setValueI("camMode", 0);
					}
				}else{
					camera.setup(cameraWidth, cameraHeight, this);
					camera.setDesiredFrameRate(60);
					camState = CAMERA_OPEN;					
				}
				

			}else if( camState == CAMERA_NEEDS_CLOSING ){
				
				if( do1394 ){
					printf("closing camera\n");
					camera1394.close();
					if( settings != NULL ){
						printf("deleting settings and sdk\n");
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
				
				camState	 = CAMERA_CLOSED;
				waitTillTime = ofGetElapsedTimef() + 5.0; //wait 5 seconds before allowing next camera change. 
			}
		}
	
		prevCamState = camState;	
	}	

}

//-----------------------------------------------
void captureApp::handleFaceTrigger(){
	
	if( camState != CAMERA_CLOSED && state < CAP_STATE_FADEIN ){
	
		float faceFps  = panel.getValueF("FACE_FPS");
		float timeDiff = 1.0/ofClamp(faceFps, 4.0, 60.0);
		
		face.confidenceAddAmnt		= panel.getValueF("confidence_add");
		face.confidenceFadeRate		= panel.getValueF("confidence_fade");
		face.confidenceGateValStart = panel.getValueF("confidence_gate_start");
		face.confidenceGateValStop  = panel.getValueF("confidence_gate_stop");
		
		if( ofGetElapsedTimef() - prevFaceCheckTimeF >= timeDiff ){
	
			unsigned char * pix = NULL;
			if( do1394 ){
				pix = camera1394.getPixels();
			}else{
				pix = camera.getPixels();
			}
			if(pix != NULL){
				face.updatePixels(pix, cameraWidth, cameraHeight);
			}
			
			prevFaceCheckTimeF = ofGetElapsedTimef();
		}
		
		//update the trigger dispite the rate we update the pixels
		face.update();
		
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
				startFadeIn();
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
				ofRotate( ofMap(saveIndex, 0, imageSaver.getSize(), -30, 30), 0, 1, 0);
				decoder.drawCloud();
			ofPopMatrix();
			glDisable(GL_DEPTH_TEST);			
		camera3D.remove();	
		//decoder.drawCurrentFrame(0, 0, 320, 240);

		ofNoFill();
		ofSetColor(100, 100, 100, 255);
		ofRect( ofGetWidth()/4, ofGetHeight()-150, ofGetWidth()/2, 20);	
		ofFill();
		ofRect( ofGetWidth()/4, ofGetHeight()-150, ofMap(saveIndex, 0, imageSaver.getSize(), 0.0, 1.0, true) * (float)(ofGetWidth()/2), 20);			
		return;
	}

	if(!panel.getValueB("frameByFrame")){
		patternFrame = ofGetFrameNum() / panel.getValueI("patternRate");
	}

	if (!panel.getValueB("brightnessSetting")){
		ofSetColor(255, 255, 255, 255);
		curGenerator->get(patternFrame).draw(0, 0);
	} else {
		int checkBrightness = panel.getValueI("checkBrightness");
		ofSetColor(checkBrightness, checkBrightness, checkBrightness);
		ofRect(0, 0, ofGetWidth(), ofGetHeight());
	}

	if( state == CAP_STATE_CAPTURE ){
		return;
	}
	
	if( panel.getValueB("bSpotLight") ){
		if( state < CAP_STATE_FADEIN ){
			spotLightAlpha = 1.0;
		}else if( state > CAP_STATE_END_DECODE ){
			spotLightAlpha += 0.05;	
			spotLightAlpha = ofClamp(spotLightAlpha, 0, 1);	
		}
	}else if(  !panel.getValueB("bSpotLight") || state == CAP_STATE_CAPTURE ){
		spotLightAlpha = 0.0;
	}
		
	if( state == CAP_STATE_FADEIN ){
		float alpha = ofMap(ofGetElapsedTimef(), fadeInStartTime, fadeInStartTime + panel.getValueF("fadeInTime"), 255, 0.0);
				
		ofPushStyle();
			ofEnableAlphaBlending();
			ofSetColor(0, 0, 0, alpha);
			ofRect(0, 0, ofGetWidth(), ofGetHeight());
		ofPopStyle();
	}
	
	if( state >= CAP_STATE_END_DECODE ){
		ofPushStyle();
			ofSetColor(0, 0, 0, 255);
			ofRect(0, 0, ofGetWidth(), ofGetHeight());
		ofPopStyle();
	}
	
	if( spotLightAlpha <= 0.01 ){
		spotLightAlpha = 0.0;
	}else{
		ofPushStyle();
			ofEnableAlphaBlending();
			float val = 255.0 * panel.getValueF("spotLightBrightness");
			ofSetColor(val, val, val, spotLightAlpha*255.0);
			spotLightImage.draw( ofGetWidth()/2, ofGetHeight()/2, ofGetWidth(), ofGetHeight() );		
		ofPopStyle();
		
		if( state == CAP_STATE_FADEIN ){
			spotLightAlpha *= 0.94;
		}
	}	
	
	if( state == CAP_STATE_CAPTURE || state == CAP_STATE_FADEIN ){
		return;
	}
	
	if(debugState == CAP_DEBUG) {
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

			// ito wrote..begin
			
			if (i == 0) {
				ofImage clipping;
				getClipping(recent[i], clipping);
				ofEnableAlphaBlending();
				clipping.update();
				ofSetColor(255, 0, 0);
				clipping.draw(0, 0, recentWidth, recentHeight);
				ofDisableAlphaBlending();
			}
			
			// it wrote..end
			ofPopMatrix();
		}
		ofPopMatrix();
		ofPopStyle();
		
		panel.draw();
		
		if( do1394 && settings != NULL ){
			if( camState == CAMERA_OPEN ){
				settings->draw();
			}
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
	if( debugState == CAP_DEBUG){
		panel.mouseDragged(x, y, button);
		if( settings ){
			settings->mouseDragged(x, y, button);
		}
	}
}

//--------------------------------------------------------------
void captureApp::mousePressed(int x, int y, int button){
	if( debugState == CAP_DEBUG){
		panel.mousePressed(x, y, button);
		if( settings ){
			settings->mousePressed(x, y, button);
		}		
	}
}

//--------------------------------------------------------------
void captureApp::mouseReleased(int x, int y, int button){
	if( debugState == CAP_DEBUG){
		panel.mouseReleased();
		if( settings ){
			settings->mouseReleased(x, y, button);
		}
	}
}

//--------------------------------------------------------------
void captureApp::keyPressed(int key) {
	if(key == 'f') {
		ofToggleFullscreen();
	}
	
	if(key == '\t') {
		if( state < CAP_STATE_CAPTURE ){
			startFadeIn();
		}
		else if( state == CAP_STATE_CAPTURE ){
			endCapture(); //force override - normally capture is timed. 
		}
	}
	
	if( key == 'D' ){
		if( debugState == CAP_DEBUG ){
			debugState = CAP_NORMAL;
		}else if( debugState == CAP_NORMAL ){
			debugState = CAP_DEBUG;
		}	
	}
	
	if( debugState == CAP_DEBUG ){
		if(panel.getValueB("frameByFrame") && key == OF_KEY_UP){
			patternFrame--;
		}

		if(panel.getValueB("frameByFrame") && key == OF_KEY_DOWN){
			patternFrame++;
		}
	}
}

//--------------------------------------------------------------
void captureApp::getClipping(ofImage& img, ofImage& clipping) {
	int w = img.getWidth();
	int h = img.getHeight();
	clipping.allocate(w, h, OF_IMAGE_COLOR_ALPHA);
	unsigned char* imgPixels = img.getPixels();
	unsigned char* clippingPixels = clipping.getPixels();
	int n = w * h;
	for(int i = 0; i < n; i++) {
		if(imgPixels[i*3] == 0 || imgPixels[i*3] == 255 ||
		   imgPixels[i*3+1] == 0 || imgPixels[i*3+1] == 255 ||
		   imgPixels[i*3+2] == 0 || imgPixels[i*3+2] == 255 ) {
			clippingPixels[i * 4 + 0] = 255;
			clippingPixels[i * 4 + 1] = 255;
			clippingPixels[i * 4 + 2] = 255;
			clippingPixels[i * 4 + 3] = 255;
		} else {
			clippingPixels[i * 4 + 0] = 0;
			clippingPixels[i * 4 + 1] = 0;
			clippingPixels[i * 4 + 2] = 0;
			clippingPixels[i * 4 + 3] = 0;

		}
	}
}
