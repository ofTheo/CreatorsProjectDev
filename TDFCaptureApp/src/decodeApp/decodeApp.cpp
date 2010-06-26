#include "decodeApp.h"

/*
	then implement exporting of sequences ("record").
	then allow skipping to different locations in the sequence.
*/

//--------------------------------------------------------------
void decodeApp::setup(){

	hidden = false;
	sequenceFrame = 0;
	threePhase = NULL;

	// setup control panel
	panel.setup("control", 0, 0, 300, 720);
	panel.addPanel("input", 1);
	panel.addPanel("decode", 1);
	panel.addPanel("export", 1);
	panel.addPanel("misc", 1);
	panel.addPanel("smoothing", 1);

	panel.setWhichPanel("input");

	inputList.listDir(CAPTURE_MAIN_FOLDER);
	inputList.reverseOrder();
	panel.addFileLister("input", &inputList, 240, 500);

	panel.setWhichPanel("decode");

	panel.addToggle("play sequence", "playSequence", false);
	panel.addSlider("jump to", "jumpTo", 0, 0, 100, false);
	panel.addToggle("phase persistence", "phasePersistence", false);
	panel.addToggle("reset view ", "resetView", false);

	panel.addSlider("camera rate", "cameraRate", 1, 1, 6, true);
	panel.addSlider("camera offset", "cameraOffset", 0, 0, 5, true);
	panel.addSlider("play rate", "playRate", 1, 1, 60, true);
	
	//only add the toggle if it finds the csv file
	if( ofxFileHelper::doesFileExist("firefly-lut.csv")){
		lut.setup(ofToDataPath("firefly-lut.csv"));
		panel.addToggle("use camera lut", "useCameraLut", true);
	}	

	styles.push_back("cloud");
	styles.push_back("mesh");
	styles.push_back("none");
	panel.addMultiToggle("style", "style", 0, styles);

	panel.addSlider("range threshold", "rangeThreshold", 4, 0, 32, true);
	panel.addSlider("depth scale", "depthScale", 130, -800, 800, false);
	panel.addSlider("depth skew", "depthSkew", 0, -10, 10, false);
	//panel.addSlider("filter min", "filterMin", -320, -480, 480, false);
	//panel.addSlider("filter max", "filterMax", 320, -480, 480, false);

	panel.setWhichPanel("export");

	exportFormats.push_back(".obj");
	exportFormats.push_back(".ply");
	exportFormats.push_back(".png");
	exportFormats.push_back("RGBA");
	panel.addMultiToggle("export format", "exportFormat", 0, exportFormats);

	panel.addToggle("export", "export", false);
	panel.addToggle("record", "record", false);

	panel.addToggle("render movie", "renderMovie", false);
	panel.addSlider("movie framerate", "movieFramerate", 60, 5, 60, true);

	panel.setWhichPanel("misc");
	vector<string> orientation;
	orientation.push_back("horizontal");
	orientation.push_back("vertical");
	panel.addMultiToggle("orientation", "orientation", 0, orientation);	
	panel.addSlider("gamma", "gamma", 1, 0.0, 1.0, false);
	panel.addToggle("hud", "hud", false);
	panel.addSlider("hudWidth", "hudWidth", 300.0, 0.0, 2000.0, false);
	panel.addSlider("hudHeightOffset", "hudHeightOffset", 0.0, 0.0, 1.0, false);

	panel.addSlider("maxPhase", "maxPhase", 10.0, 0.0, 100.0, false);
	panel.addSlider("maxDepth power", "maxDepth", 3.0, 0.0, 5.0, false);

	panel.setWhichPanel("smoothing");
	panel.addSlider("smooth y dist", "smooth_y_dist", 1, 1, 20, true);
	panel.addSlider("smooth y amnt", "smooth_y_amnt", 0, 0, 1.0, false);
	panel.addToggle("smooth gaussian", "smooth_gaussian", false);
	panel.addSlider("dilate passes", "dilate_passes", 2, 0, 6, true);
	panel.addSlider("max hole size", "max_hole_size", 0, 0, 32, true);
	panel.addDrawableRect("rgbaTex", &rgbaTex, 160, 120);
	panel.addDrawableRect("wrappedPhase", &wrappedPhaseTex, 160, 120);

	panel.loadSettings("controlDecode.xml");
	
	//overides 
	panel.setValueI("export", 0);
	panel.setValueI("record", 0);
	panel.setValueI("renderMovie", 0);
	panel.setValueI("playSequence", 0);	
	
	rgbaTex.allocate(640, 480, GL_RGBA);
	wrappedPhaseTex.allocate(640, 480, GL_LUMINANCE);

}

//--------------------------------------------------------------
decodeApp::~decodeApp() {

}

//--------------------------------------------------------------
void decodeApp::jumpTo(unsigned frame) {
	sequenceFrame = frame;
	for (int i = 0; i < 3; i++)
		nextFrame();
}

//--------------------------------------------------------------
void decodeApp::nextFrame() {
	int cameraRate = panel.getValueI("cameraRate");
	int cameraOffset = panel.getValueI("cameraOffset");
	unsigned totalFrames = (totalImages - cameraOffset) / cameraRate;

	int cameraFrame = (sequenceFrame * cameraRate) + cameraOffset;
	cameraFrame %= totalImages;
	if (usingDirectory) {
		phase.setUseTexture(false);
		if (!phase.loadImage(inputDir + imageList.getName(cameraFrame))) {
			cout << "couldn't load file " << (inputDir + imageList.getName(cameraFrame)) << endl;
			return;
		}
		
		if(panel.getValueB("useCameraLut")) {
			//force it for now - note that just changing the image to grayscale helps the ripple quality.
			//actually commenting out lut.filter looks the same as having it in. 
			phase.setImageType(OF_IMAGE_GRAYSCALE);
			lut.filter(phase);
		}
		pipeline(phase.getPixels(), phase.type == OF_IMAGE_GRAYSCALE ? 1 : 3, sequenceFrame);		
	} else {
		movieInput.setFrame(cameraFrame);		
		pipeline(movieInput.getPixels(), 3, sequenceFrame);
	}

	sequenceFrame = (sequenceFrame + 1) % totalFrames;

	if (sequenceFrame == 0)
		cout << "Starting sequence over." << endl;
}

//--------------------------------------------------------------
void decodeApp::pipeline(unsigned char * pixels, int numChannelsIn, int frameIndex){
	if( threePhase == NULL )return;	
	threePhase->set(frameIndex % 3, pixels, numChannelsIn == 3 ? 3 : 1);
}

//--------------------------------------------------------------
void decodeApp::processFrame(){
	if( threePhase == NULL )return;

	threePhase->decode();

	threePhase->filterRange(filterMin, filterMax);
	
	//TODO: optimize
	if( panel.getValueF("smooth_y_amnt") > 0.0 || panel.getValueB("smooth_gaussian") ){
		float smoothDist = panel.getValueF("smooth_y_dist");
		float smoothAmnt = panel.getValueF("smooth_y_amnt");
		bool smoothGaussian = panel.getValueB("smooth_gaussian");
		int dilatePasses = panel.getValueI("dilate_passes");
		threePhase->filterDepth(smoothDist, smoothAmnt, smoothGaussian, dilatePasses); // <3
	}
}

//------------------------------------------------------------------------------
void decodeApp::initDecoder(int w, int h){
	if (threePhase != NULL){
		delete threePhase;
	}
	threePhase = new FastThreePhase();
	threePhase->setup(w, h);	
}

//------------------------------------------------------------------------------
void decodeApp::setupInput() {
	
	string name = inputList.getSelectedName();
	usingDirectory = name.find('.') == string::npos;
	
	bool success = false;
	
	if(usingDirectory) {
			
		inputDir = string(CAPTURE_MAIN_FOLDER) + name + "/";
		imageList.reset();
		imageList.allowExt("jpg");
		imageList.allowExt("JPG");
		imageList.allowExt("png");
		imageList.allowExt("PNG");
		imageList.allowExt("tga");
		imageList.allowExt("tiff");		
		imageList.allowExt("jpeg");		
		totalImages = imageList.listDir(inputDir);
		ofImage phase;
		//  this image loading is just so the dimensions are known
		
		if (totalImages <= 0 ){
			reload = false;
			ofLog(OF_LOG_ERROR, "no images in %s", inputDir.c_str());
			return;
		 }

		string imageName = imageList.getName(0);
		 if( imageName == "" || !phase.loadImage(inputDir + imageName) ) {
			cout << "couldn't load file " << (inputDir + imageName) << endl;
			ofLog(OF_LOG_ERROR, "couldn't load file from %s", inputDir.c_str());
			reload = false;
			return;
		}

		currentName = name;		
		int w = (int) phase.getWidth();
		int h = (int) phase.getHeight();

		initDecoder(w, h);

		phaseUnwrapped.clear();
		phaseUnwrapped.allocate(w, h, OF_IMAGE_COLOR);
		phaseWrapped.clear();
		phaseWrapped.allocate(w, h, OF_IMAGE_GRAYSCALE);
		rangeIm.clear();
		rangeIm.allocate(w, h, OF_IMAGE_GRAYSCALE);
		unwrapOrderIm.clear();
		unwrapOrderIm.allocate(w, h, OF_IMAGE_COLOR);
		depthIm.clear();
		depthIm.allocate(w, h, OF_IMAGE_COLOR);
		
		success = true;

	} else if( ofxFileHelper::getFileExt(name) == "mov" ||  ofxFileHelper::getFileExt(name) == "mp4" ){
		movieInput.loadMovie("input/" + name);
		if( movieInput.bLoaded ){
			totalImages = movieInput.getTotalNumFrames();
			if( totalImages > 0 ){
				movieInput.setVolume(0);
				initDecoder((int) movieInput.getWidth(), (int) movieInput.getHeight());	
				success = true;				
			}
		}
	}
	
	if( success ){
		jumpTo(0);
	}
}


//------------------------------------------------------------------------------
void decodeApp::updateDecoderSettings(){
	if( threePhase != NULL ){
		
		int curRangeThreshold		= panel.getValueI("rangeThreshold");
		float gamma					= panel.getValueF("gamma");
		float curDepthScale			= panel.getValueF("depthScale");
		float curDepthSkew			= panel.getValueF("depthSkew");
		int curOrientation			= panel.getValueI("orientation");
		bool curPhasePersistence	= panel.getValueB("phasePersistence");
		int maxHoleSize = panel.getValueI("max_hole_size");
		
		threePhase->setGamma(gamma);
		threePhase->setDepthScale(curDepthScale);
		threePhase->setDepthSkew(curDepthSkew);
		threePhase->setRangeThreshold(curRangeThreshold);
		threePhase->setOrientation(curOrientation == 0 ? PHASE_HORIZONTAL : PHASE_VERTICAL);
		threePhase->setPhasePersistence(curPhasePersistence);
		threePhase->setMaxHoleSize(maxHoleSize);

		if (panel.hasValueChanged("phasePersistence")){
			threePhase->clearLastPhase();
		}
	
	
	}
}

//--------------------------------------------------------------
void decodeApp::update() {
	panel.update();

	reload = inputList.selectedHasChanged();
	if (reload) {
		setupInput();
		inputList.clearChangedFlag();
	}

	updateDecoderSettings();
	handlePlayback();
	handleExport();
	
	panel.clearAllChanged();
}

//--------------------------------------------------------------
void decodeApp::handlePlayback(){
	
	bool curPlaySequence		= panel.getValueB("playSequence");
	int curPlayRate				= panel.getValueF("playRate");
	int curCameraRate			= panel.getValueI("cameraRate");
	int curCameraOffset			= panel.getValueI("cameraOffset");

	bool curResetView = panel.getValueB("resetView");
	if(curResetView) {
		camera = ofxEasyCam();
		panel.setValueB("resetView", false);
	}

	unsigned totalFrames = (totalImages - curCameraOffset) / curCameraRate;

	if (threePhase != NULL) {
				
		if (panel.hasValueChanged("jumpTo")) {
			// map slider to entire range of input images
			unsigned targetFrame = (unsigned) ofMap(panel.getValueI("jumpTo"), 0, 100, 0, totalFrames);
			// clamp to amaxmimum of last image - 3, so you don't try reading in a loop
			targetFrame = (unsigned) ofClamp(targetFrame, 0, totalFrames - 3);

			// so long as we're not just jumping to the same place
			if ((targetFrame + 3) % totalFrames != sequenceFrame) {
				jumpTo(targetFrame);
				reload = true;
			}
		} else if (ofGetFrameNum() % curPlayRate == 0 && (curPlaySequence || panel.hasValueChanged("playRate") || panel.hasValueChanged("cameraOffset"))) {

			nextFrame();
				
			panel.setValueF("jumpTo", ofMap(sequenceFrame, 0, totalFrames, 0, 100));
			reload = true;
		}

		if(reload || panel.hasValueChanged("gamma") || panel.hasValueChanged("rangeThreshold") 
			|| panel.hasValueChanged("orientation") || panel.hasValueChanged("depthScale") 
			|| panel.hasValueChanged("depthSkew")  || panel.hasValueChanged("smooth_y_amnt") || panel.hasValueChanged("smooth_y_dist")
			|| panel.hasValueChanged("smooth_gaussian") || panel.hasValueChanged("dilate_passes")
			|| panel.hasValueChanged("max_hole_size")) {

			processFrame();
			redraw = true;
		}

		if( redraw ){
			//theo added - for debugging		
			rgbaTex.loadData(threePhase->getColorAndDepth(filterMin, filterMax), 640, 480, GL_RGBA);
			wrappedPhaseTex.loadData(threePhase->phasePixels, 640, 480, GL_LUMINANCE);
		}

	}

}

//--------------------------------------------------------------
void decodeApp::handleExport(){
	if( threePhase != NULL ){
		
		// export handling
		bool curExport = panel.getValueB("export");
		bool curRecord = panel.getValueB("record");
		if (curExport || curRecord) {
			string curFormat = exportFormats[panel.getValueI("exportFormat")];
			string name = inputList.getSelectedName();
			
			string exportPath = EXPORT_FOLDER+string("export-")+name+"/";
			if( !ofxFileHelper::doesFileExist(exportPath) ){
				ofxFileHelper::makeDirectory(exportPath);
			}
						
			if (curRecord)
				name += "-" + ofToString(FRAME_START_INDEX+sequenceFrame);
			if (curFormat == ".png") {
				threePhase->exportDepth(exportPath + name + "-depth.png", filterMin, filterMax);
				threePhase->exportTexture(exportPath + name + "-texture.png");
			} else if(curFormat == "RGBA") {
				cout << "exporting frames using " << filterMin << " to " << filterMax << endl;
				threePhase->exportDepthAndTexture(exportPath + name + ".png", filterMin, filterMax);
			} else {
				int curStyle = panel.getValueI("style");
				string outputFile = exportPath + name + "-" + styles[curStyle] + curFormat;
				if (curStyle == 0) {
					threePhase->exportCloud(outputFile);
				} else if (curStyle == 1) {
					threePhase->exportMesh(outputFile);
				}
			}
			panel.setValueB("export", false);
		}
	}	
}

//--------------------------------------------------------------
void decodeApp::draw() {
	if (hidden)
		ofBackground(0, 0, 0);
	else
		ofBackground(128, 128, 128);

	glPushMatrix();
	camera.place();
	glEnable(GL_DEPTH_TEST);

	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

	if (threePhase != NULL) {
		if (!hidden) {
			drawAxes(256); // major axes
		}

		int w = threePhase->getWidth();
		int h = threePhase->getHeight();
		
		ofTranslate(-w / 2, -h / 2);

		if (!hidden) {
			ofxPoint3f min, max;
			getBounds(min, max);
			ofSetColor(255, 255, 255);
			boxOutline(min, max);
			min.z = filterMin;
			max.z = filterMax;
			min -= 4;
			max += 4;
			ofSetColor(0, 0, 255);
			boxOutline(min, max);
		}

		ofSetColor(255, 255, 255);
		int useCloud = panel.getValueI("style");
		if (useCloud == 0) {
			drawCloud();
		} else if (useCloud == 1) {
			drawMesh();
		}
		
		if(!hidden) {
			ofEnableAlphaBlending();
			ofSetColor(255, 255, 255, 32);
			glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(w, 0);
			glVertex2f(w, h);
			glVertex2f(0, h);
			glEnd();
			ofDisableAlphaBlending();
		}

	}

	glDisable(GL_DEPTH_TEST);

	camera.remove();

	if (panel.getValueB("renderMovie") && hidden) {
		screenCapture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		movieOutput.addFrame(screenCapture.getPixels(), 1. / panel.getValueI("movieFramerate"));
	}

	glPopMatrix();

	if ((threePhase != NULL) && panel.getValueB("hud")) {

		int srcWidth = threePhase->getWidth();
		int srcHeight = threePhase->getHeight();

		if (redraw) {
			bool* mask = threePhase->getMask();
			float* depth = threePhase->getDepth();
			byte* color = threePhase->getColor();
			float* phase = threePhase->getPhase();
			float* wrappedPhase = threePhase->getWrappedPhase();
			float* range = threePhase->getRange();
			float* unwrapOrder = threePhase->unwrapOrder;

			unsigned char* upix = phaseUnwrapped.getPixels();
			unsigned char* ppix = phaseWrapped.getPixels();
			unsigned char* rpix = rangeIm.getPixels();
			unsigned char* opix = unwrapOrderIm.getPixels();
			unsigned char* dpix = depthIm.getPixels();

			///float minPhase = threePhase->minPhase;
			///float maxPhase = threePhase->maxPhase;
			float minPhase = -panel.getValueF("maxPhase");
			float maxPhase = -minPhase;

			// threePhase->minDepth,threePhase->maxDepth
			float maxDepth = pow(10, panel.getValueF("maxDepth"));
			float minDepth = -maxDepth;

			int i = 0;
			for (int y = 0; y < srcHeight; y++) {
				for (int x = 0; x < srcWidth; x++) {
					ppix[i] = (char) ofMap(wrappedPhase[i],
					                            0, 1,
					                            0, 255);

					rpix[i] = (char) range[i];

					float mag;
					ofColor col;

					if (!mask[i]) {
						mag = ofMap(phase[i], minPhase, maxPhase, 0.0, 1.0);
					} else {
						mag = 0.5;
					}
					col = makeColor(mag);

					upix[i*3] =  (char)col.r;
					upix[i*3+1] = (char)col.g;
					upix[i*3+2] = (char)col.b;

					mag = 1.0 - ofMap(depth[i], minDepth, maxDepth, 0.0, 1.0);
					col = makeColor(mag);
					dpix[i*3] = (short) col.r;
					dpix[i*3+1] = (short) col.g;
					dpix[i*3+2] = (short) col.b;

					mag = unwrapOrder[i] / (float)(srcWidth * srcHeight);
					col = makeColor(mag);
					opix[i*3] = (short) col.r;
					opix[i*3+1] = (short) col.g;
					opix[i*3+2] = (short) col.b;

					i++;
				}
			}
			phaseUnwrapped.update();
			phaseWrapped.update();
			rangeIm.update();
			unwrapOrderIm.update();
			depthIm.update();

		}

		int w = (int)panel.getValueF("hudWidth");
		float hOff = panel.getValueF("hudHeightOffset");

		ofSetColor(255, 255, 255);
		glColor3f(1, 1, 1);
		float sc = (float)srcHeight / (float)srcWidth;
		hOff *= -2 * w * sc;

		phaseUnwrapped.getTextureReference().draw(ofGetWidth() - w, hOff + 0*w*sc, w, w*sc);
		depthIm.getTextureReference().draw(ofGetWidth() - w, hOff + 1*w*sc, w, w*sc);
		rangeIm.getTextureReference().draw(ofGetWidth() - w, hOff + 2*w*sc, w, w*sc);

		unwrapOrderIm.getTextureReference().draw(ofGetWidth() - 2*w, hOff + 0*w*sc, w, w*sc);
		phaseWrapped.getTextureReference().draw(ofGetWidth() - 2*w, hOff + 1*w*sc, w, w*sc);
	}
	
	if(!hidden){
		panel.draw();
	}
}


//--------------------------------------------------------------
void decodeApp::getBounds(ofxPoint3f& min, ofxPoint3f& max) {
	bool* mask = threePhase->getMask();
	float* depth = threePhase->getDepth();

	int srcWidth = threePhase->getWidth();
	int srcHeight = threePhase->getHeight();

	min.set(srcWidth, srcHeight, filterMax);
	max.set(0, 0, filterMin);

	int i = 0;
	for (int y = 0; y < srcHeight; y++) {
		for (int x = 0; x < srcWidth; x++) {
			if (!mask[i]) {
				if (x < min.x)
					min.x = x;
				if (x > max.x)
					max.x = x;
				if (y < min.y)
					min.y = y;
				if (y > max.y)
					max.y = y;
				if (depth[i] < min.z)
					min.z = depth[i];
				if (depth[i] > max.z)
					max.z = depth[i];
			}
			i++;
		}
	}
}

//--------------------------------------------------------------
void decodeApp::boxOutline(ofxPoint3f min, ofxPoint3f max) {
	ofPushMatrix();
	ofTranslate(min.x, min.y, min.z);
	ofScale(max.x - min.x, max.y - min.y, max.z - min.z);
	glBegin(GL_LINES);
	// back
	glVertex3s(0, 0, 0);
	glVertex3s(1, 0, 0);
	glVertex3s(0, 0, 0);
	glVertex3s(0, 1, 0);
	glVertex3s(1, 1, 0);
	glVertex3s(1, 0, 0);
	glVertex3s(1, 1, 0);
	glVertex3s(0, 1, 0);
	// front
	glVertex3s(0, 0, 1);
	glVertex3s(1, 0, 1);
	glVertex3s(0, 0, 1);
	glVertex3s(0, 1, 1);
	glVertex3s(1, 1, 1);
	glVertex3s(1, 0, 1);
	glVertex3s(1, 1, 1);
	glVertex3s(0, 1, 1);
	// extrusion
	glVertex3s(0, 0, 0);
	glVertex3s(0, 0, 1);
	glVertex3s(0, 1, 0);
	glVertex3s(0, 1, 1);
	glVertex3s(1, 0, 0);
	glVertex3s(1, 0, 1);
	glVertex3s(1, 1, 0);
	glVertex3s(1, 1, 1);
	glEnd();
	ofPopMatrix();
}

//--------------------------------------------------------------
void decodeApp::drawAxes(float size) {
	ofPushMatrix();
	ofScale(size, size, size);
	ofPushStyle();
	ofSetLineWidth(2.0);
	ofSetColor(255, 0, 0);
	glBegin(GL_LINES);
	glVertex3s(0, 0, 0);
	glVertex3s(1, 0, 0);
	glEnd();
	ofSetColor(0, 255, 0);
	glBegin(GL_LINES);
	glVertex3s(0, 0, 0);
	glVertex3s(0, 1, 0);
	glEnd();
	ofSetColor(0, 0, 255);
	glBegin(GL_LINES);
	glVertex3s(0, 0, 0);
	glVertex3s(0, 0, 1);
	glEnd();
	ofPopStyle();
	ofPopMatrix();
}

//--------------------------------------------------------------
void decodeApp::drawCloud() {
	bool* mask = threePhase->getMask();
	float* depth = threePhase->getDepth();
	byte* color = threePhase->getColor();

	int srcWidth = threePhase->getWidth();
	int srcHeight = threePhase->getHeight();

	glEnable(GL_POINT_SIZE);
	glPointSize(3);
	glBegin(GL_POINTS);
	int i = 0;
	for (int y = 0; y < srcHeight; y++) {
		for (int x = 0; x < srcWidth; x++) {
			if (!mask[i]) {
				glColor3ubv(&color[i * 3]);
				glVertex3f(x, y, depth[i]);

			}
			i++;
		}
	}
	glEnd();
}

//--------------------------------------------------------------
void decodeApp::drawMesh() {
	bool* mask = threePhase->getMask();
	float* depth = threePhase->getDepth();
	byte* color = threePhase->getColor();

	int srcWidth = threePhase->getWidth();
	int srcHeight = threePhase->getHeight();

	glBegin(GL_TRIANGLES);
	for (int y = 0; y < srcHeight - 1; y++) {
		for (int x = 0; x < srcWidth - 1; x++) {
			int nw = y * srcWidth + x;
			int ne = nw + 1;
			int sw = nw + srcWidth;
			int se = ne + srcWidth;
			if (!mask[nw] && !mask[se]) {
				if (!mask[ne]) { // nw, ne, se
					glColor3ubv(&color[nw * 3]);
					glVertex3f(x, y, depth[nw]);
					glColor3ubv(&color[ne * 3]);
					glVertex3f(x + 1, y, depth[ne]);
					glColor3ubv(&color[se * 3]);
					glVertex3f(x + 1, y + 1, depth[se]);
				}
				if (!mask[sw]) { // nw, se, sw
					glColor3ubv(&color[nw * 3]);
					glVertex3f(x, y, depth[nw]);
					glColor3ubv(&color[se * 3]);
					glVertex3f(x + 1, y + 1, depth[se]);
					glColor3ubv(&color[sw * 3]);
					glVertex3f(x, y + 1, depth[sw]);
				}
			} else if (!mask[ne] && !mask[sw]) {
				if (!mask[nw]) { // nw, ne, sw
					glColor3ubv(&color[nw * 3]);
					glVertex3f(x, y, depth[nw]);
					glColor3ubv(&color[ne * 3]);
					glVertex3f(x + 1, y, depth[ne]);
					glColor3ubv(&color[sw * 3]);
					glVertex3f(x, y + 1, depth[sw]);
				}
				if (!mask[se]) { // ne, se, sw
					glColor3ubv(&color[ne * 3]);
					glVertex3f(x + 1, y, depth[ne]);
					glColor3ubv(&color[se * 3]);
					glVertex3f(x + 1, y + 1, depth[se]);
					glColor3ubv(&color[sw * 3]);
					glVertex3f(x, y + 1, depth[sw]);
				}
			}
		}
	}
	glEnd();
}

//--------------------------------------------------------------
void decodeApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void decodeApp::mousePressed(int x, int y, int button){
	if( panel.mousePressed(x, y, button) ){
		camera.disableMouseEvents();
	}
}

//--------------------------------------------------------------
void decodeApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
	camera.enableMouseEvents();
}

//--------------------------------------------------------------
void decodeApp::keyPressed(int key) {
	if (key == 'f')
		ofToggleFullscreen();
	if (key == '\t') {
		hidden = !hidden;
		panel.hidden = hidden;
		if (panel.getValueB("renderMovie")) {
			if (hidden) {
				screenCapture.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR);
				movieOutput.setup(ofGetWidth(), ofGetHeight(), EXPORT_FOLDER + getTimeAsUniqueString() + ".mov");
			} else {
				movieOutput.finishMovie();
			}
		}
	}
		
	//panel.keyPressed(key);
	
}

//--------------------------------------------------------------
const unsigned char decodeApp::scol[8][3] = {{255, 255, 255},
	{255, 0, 0}, /// red is the warmest
	{255, 255, 0},
	{0, 255, 0},
	{0, 255, 255},
	{0, 0, 255},/// blue is the coolest, but purple comes later
	{255, 0, 255},
	{0, 0, 0}
};

//--------------------------------------------------------------
ofColor decodeApp::makeColor(float f) {
	/// or wrap around?
	if (f > 1.0) f = 1.0;
	if (f < 0.0) f = 0.0;

	int i1 = 0;
	int i2 = 1;
	f = f * (8 - 1);

	while (f > 1.0) {
		f -= 1.0;
		i1++;
		i2++;
	}

	int r = int((1.0 - f) * (float)scol[i1][0] + f * (float)scol[i2][0]);
	int g = int((1.0 - f) * (float)scol[i1][1] + f * (float)scol[i2][1]);
	int b = int((1.0 - f) * (float)scol[i1][2] + f * (float)scol[i2][2]);

	ofColor rv;
	rv.r = r;
	rv.g = g;
	rv.b = b;
	return rv;
}
