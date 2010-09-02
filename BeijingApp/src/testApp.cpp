#include "testApp.h"

/*
 make an attempt at better packing with ellipses?
*/

void testApp::setup() {
	ofSetFrameRate(30);
	lastFps = true;
	lastSaveImages = false;

	ofEnableAlphaBlending();
	ofBackground(128, 128, 128);

	particleSystem.setup(100);
	setupOutput();
	setupMeta();
	setupGui();
	setupCubemap();
}

float testApp::rotationDistance(ofxQuaternion& from, ofxQuaternion& to) {
	ofxVec3f xunit3f(1, 0, 0);
	ofxVec3f fromVec = from * xunit3f;
	ofxVec3f toVec = to * xunit3f;
	return acosf(fromVec.dot(toVec));
}

void testApp::setupOutput() {
	ofxXmlSettings xml;
	xml.loadFile("outputSettings.xml");
	outputPrefix = xml.getValue("outputPrefix", "map");
	outputPostfix = xml.getValue("outputPostfix", ".tif");
	outputPadding = xml.getValue("outputPadding", 4);
	cout << "Using output settings '" << outputPrefix << "#" << outputPostfix << "'" << endl;
}

void testApp::setupMeta() {
	meta.setup("Meta Settings", 650, 30, 350, 300);
	meta.addPanel("Keyframes", 2, false);
	meta.setWhichPanel("Keyframes");

	meta.setWhichColumn(0);
	keyframeCount = keyframeLister.listDir("keyframes");
	meta.addFileLister("keyframes", &keyframeLister, 100, 180);

	meta.setWhichColumn(1);
	meta.addSlider("keyframe number", "keyframeNumber", 0, 0, 20, true);
	meta.addToggle("save keyframe", "saveKeyframe", false);
	meta.addToggle("refresh list", "refreshList", false);

	meta.addPanel("Render", 2, false);
	meta.setWhichPanel("Render");
	meta.addToggle("save images", "saveImages", false);
	meta.addSlider("current position ", "currentPosition", 0, 0, 100, false);
}

void testApp::setupGui() {
	gui.setup("Settings", 10, 10, 280, 700);

	gui.addPanel("Display", 1, false);
	gui.setWhichPanel("Display");
	gui.addToggle("use cubemap", "useCubemap", false);
	gui.addToggle("fps limit", "fpsLimit", lastFps);
	gui.addToggle("animate faces", "animateFaces", false);
	gui.addToggle("show axes", "showAxes", true);
	gui.addToggle("show cities", "showCities", true);
	gui.addToggle("show circles", "showCircles", true);
	gui.addToggle("show collisions", "showCollisions", true);
	gui.addToggle("show faces", "showFaces", true);
	gui.addToggle("use mouse cam", "useMouseCam", false);
	gui.addToggle("reset camera", "resetCamera", false);
	gui.addSlider("rotate x", "rotateX", 0.5, -1, 1, false);
	gui.addSlider("rotate y", "rotateY", 0, -1, 1, false);
	gui.addSlider("rotate z", "rotateZ", 0, -1, 1, false);

	gui.addPanel("Movement", 1, false);
	gui.setWhichPanel("Movement");
	gui.addSlider("velocity damping", "velocityDamping", .2, 0, 1, false);
	gui.addSlider("velocity max", "velocityMax", .03, 0, .05, false);
	gui.addSlider("force scale", "forceScale", .015, 0, .1, false);
	gui.addSlider("hardness", "hardness", 5, 0, 10, false);
	gui.addSlider("meander force", "meanderForce", .005, 0, .1, false);
	gui.addSlider("meander jitter", "meanderJitter", .5, 0, 5, false);
	gui.addSlider("meander range", "meanderRange", 1, 0, 1, false);
	gui.addSlider("angle jitter", "angleJitter", .1, 0, .5, false);
	gui.addSlider("angle range", "angleRange", 1, 0, 1, false);

	gui.addPanel("Size", 1, false);
	gui.setWhichPanel("Size");
	gui.addSlider("sphere coverage", "sphereCoverage", 1, .5, 1.5, false);
	gui.addSlider("face scale", "faceScale", 2, .5, 4, false);
	gui.addSlider("birth time", "birthTime", 10, 0, 40, true);
	gui.addSlider("death time", "deathTime", 10, 0, 40, true);

	gui.addPanel("Cities", 1, false);
	gui.setWhichPanel("Cities");
	gui.addSlider("Total Count", "totalCount", 0, 0, 200, true);
	gui.addSlider("New York Radius", "newYorkRadius", 0, 0, PI, false);
	gui.addSlider("London Radius", "londonRadius", 0, 0, PI, false);
	gui.addSlider("Sao Paolo Radius", "saoPaoloRadius", 0, 0, PI, false);
	gui.addSlider("Seoul Radius", "seoulRadius", 0, 0, PI, false);
	gui.addSlider("Beijing Radius", "beijingRadius", 0, 0, PI, false);
	gui.addSlider("keyframe length", "keyframeLength", 1, 1, 1800, true);
	
	gui.addPanel("City Locations", 1, false);
	gui.setWhichPanel("City Locations");
	gui.addSlider("New York Latitude", "newYorkLatitude", 40.7, -90, 90, false);
	gui.addSlider("New York Longitude", "newYorkLongitude", -74, -180, 180, false);
	gui.addSlider("London Latitude", "londonLatitude", 51.5, -90, 90, false);
	gui.addSlider("London Longitude", "londonLongitude", -.1, -180, 180, false);
	gui.addSlider("Sao Paolo Latitude", "saoPaoloLatitude", -23.5, -90, 90, false);
	gui.addSlider("Sao Paolo Longitude", "saoPaoloLongitude", -46.6, -180, 180, false);
	gui.addSlider("Seoul Latitude", "seoulLatitude", 37.5, -90, 90, false);
	gui.addSlider("Seoul Longitude", "seoulLongitude", 127, -180, 180, false);
	gui.addSlider("Beijing Latitude", "beijingLatitude", 39.9, -90, 90, false);
	gui.addSlider("Beijing Longitude", "beijingLongitude", 116.4, -180, 180, false);
}

void testApp::setupCubemap() {
	int cubemapSide = 256;
	ofxCubeMapRenderer::setup(cubemapSide);
	int cubemapRenderWidth = cubemapSide * 4;
	int cubemapRenderHeight = cubemapSide * 3;
	cubemapFbo.setup(cubemapRenderWidth, cubemapRenderHeight, false);
	cubemapImage.allocate(cubemapRenderWidth, cubemapRenderHeight, OF_IMAGE_COLOR_ALPHA);
	cubemapFbo.attach(cubemapImage.getTextureReference());
}

void testApp::grabCurValues() {
	curVelocityDamping = gui.getValueF("velocityDamping");
	curVelocityMax = gui.getValueF("velocityMax");
	curForceScale = gui.getValueF("forceScale");
	curHardness = gui.getValueF("hardness");
	curMeanderForce = gui.getValueF("meanderForce");
	curMeanderJitter = gui.getValueF("meanderJitter");
	curMeanderRange = gui.getValueF("meanderRange");
	curAngleJitter = gui.getValueF("angleJitter");
	curAngleRange = gui.getValueF("angleRange");
	curSphereCoverage = gui.getValueF("sphereCoverage");
	curFaceScale = gui.getValueF("faceScale");
	curBirthTime = gui.getValueF("birthTime");
	curDeathTime = gui.getValueF("deathTime");
	curTotalCount = gui.getValueI("totalCount");
	curNewYorkRadius = gui.getValueF("newYorkRadius");
	curLondonRadius = gui.getValueF("londonRadius");
	curSaoPaoloRadius = gui.getValueF("saoPaoloRadius");
	curSeoulRadius = gui.getValueF("seoulRadius");
	curBeijingRadius = gui.getValueF("beijingRadius");
}

void testApp::grabNextValues() {
	nextVelocityDamping = gui.getValueF("velocityDamping");
	nextVelocityMax = gui.getValueF("velocityMax");
	nextForceScale = gui.getValueF("forceScale");
	nextHardness = gui.getValueF("hardness");
	nextMeanderForce = gui.getValueF("meanderForce");
	nextMeanderJitter = gui.getValueF("meanderJitter");
	nextMeanderRange = gui.getValueF("meanderRange");
	nextAngleJitter = gui.getValueF("angleJitter");
	nextAngleRange = gui.getValueF("angleRange");
	nextSphereCoverage = gui.getValueF("sphereCoverage");
	nextFaceScale = gui.getValueF("faceScale");
	nextBirthTime = gui.getValueF("birthTime");
	nextDeathTime = gui.getValueF("deathTime");
	nextTotalCount = gui.getValueI("totalCount");
	nextNewYorkRadius = gui.getValueF("newYorkRadius");
	nextLondonRadius = gui.getValueF("londonRadius");
	nextSaoPaoloRadius = gui.getValueF("saoPaoloRadius");
	nextSeoulRadius = gui.getValueF("seoulRadius");
	nextBeijingRadius = gui.getValueF("beijingRadius");
}

void testApp::interpolateValues() {
	float i = (float) (curFrame - startFrame) / (endFrame - startFrame);
	gui.setValueF("velocityDamping", ofLerp(curVelocityDamping, nextVelocityDamping, i));
	gui.setValueF("velocityMax", ofLerp(curVelocityMax, nextVelocityMax, i));
	gui.setValueF("forceScale", ofLerp(curForceScale, nextForceScale, i));
	gui.setValueF("hardness", ofLerp(curHardness, nextHardness, i));
	gui.setValueF("meanderForce", ofLerp(curMeanderForce, nextMeanderForce, i));
	gui.setValueF("meanderJitter", ofLerp(curMeanderJitter, nextMeanderJitter, i));
	gui.setValueF("meanderRange", ofLerp(curMeanderRange, nextMeanderRange, i));
	gui.setValueF("angleJitter", ofLerp(curAngleJitter, nextAngleJitter, i));
	gui.setValueF("angleRange", ofLerp(curAngleRange, nextAngleRange, i));
	gui.setValueF("sphereCoverage", ofLerp(curSphereCoverage, nextSphereCoverage, i));
	gui.setValueF("faceScale", ofLerp(curFaceScale, nextFaceScale, i));
	gui.setValueF("birthTime", ofLerp(curBirthTime, nextBirthTime, i));
	gui.setValueF("deathTime", ofLerp(curDeathTime, nextDeathTime, i));
	gui.setValueI("totalCount", ofLerp(curTotalCount, nextTotalCount, i));
	gui.setValueF("newYorkRadius", ofLerp(curNewYorkRadius, nextNewYorkRadius, i));
	gui.setValueF("londonRadius", ofLerp(curLondonRadius, nextLondonRadius, i));
	gui.setValueF("saoPaoloRadius", ofLerp(curSaoPaoloRadius, nextSaoPaoloRadius, i));
	gui.setValueF("seoulRadius", ofLerp(curSeoulRadius, nextSeoulRadius, i));
	gui.setValueF("beijingRadius", ofLerp(curBeijingRadius, nextBeijingRadius, i));
}

void testApp::update() {
	if(meta.getValueB("refreshList")) {
		keyframeCount = keyframeLister.refreshDir();
		meta.setValueB("refreshList", false);
	}
	
	if(meta.getValueB("saveKeyframe")) {
		int keyframeNumber = meta.getValueI("keyframeNumber");
		stringstream ss;
    ss << setw(2) << setfill('0') << keyframeNumber;
    gui.saveSettings("keyframes/" + ss.str() + ".xml");
		meta.setValueB("saveKeyframe", false);
	}

	if(keyframeLister.selectedHasChanged()) {
		gui.loadSettings(keyframeLister.getSelectedPath());
		keyframeLister.clearChangedFlag();
	}

	bool curSaveImages = meta.getValueB("saveImages");
	if(curSaveImages) { // currently rendering
		if(curSaveImages != lastSaveImages) { // just started rendering
			if(keyframeCount > 1) {
				// add up all the durations
				totalFrames = 0;
				for(int i = 0; i < keyframeCount - 1; i++) {
					gui.loadSettings(keyframeLister.getPath(i));
					totalFrames += gui.getValueI("keyframeLength");
				}
				curKeyframe = 0;
				startFrame = 0;
				curFrame = 0;
				endFrame = 0;
			}
			lastSaveImages = true;
		}

		if(curFrame == totalFrames) { // done rendering
			meta.setValueB("saveImages", false);
		} else {
			if(curFrame == endFrame) { // need to load next keyframe
				gui.loadSettings(keyframeLister.getPath(curKeyframe + 1));
				grabNextValues();
				gui.loadSettings(keyframeLister.getPath(curKeyframe++));
				grabCurValues();
				startFrame = curFrame;
				endFrame += gui.getValueI("keyframeLength");
			} else {
				interpolateValues();
			}
		}
		meta.setValueF("currentPosition", (100 * curFrame) / totalFrames);

		// render just the faces into a cubemap
		gui.setValueB("fpsLimit", false);
		gui.setValueB("resetCamera", true);
		gui.setValueB("useCubemap", true);
		gui.setValueB("showAxes", false);
		gui.setValueB("showCircles", false);
		gui.setValueB("showFaces", true);
		gui.setValueB("useMouseCam", false);
	}
	lastSaveImages = curSaveImages;

	bool curFps = gui.getValueB("fpsLimit");
	if(curFps != lastFps)
		ofSetFrameRate(curFps ? 30 : 0);
	lastFps = curFps;

	if(gui.getValueB("resetCamera")) {
		gui.setValueF("rotateX", 0.5);
		gui.setValueF("rotateY", 0);
		gui.setValueF("rotateZ", 0);
		gui.setValueB("resetCamera", false);
	}

	particleSystem.moveTowards(gui.getValueI("totalCount"));
	
	particleSystem.newYork.setRadius(gui.getValueF("newYorkRadius"));
	particleSystem.london.setRadius(gui.getValueF("londonRadius"));
	particleSystem.saoPaolo.setRadius(gui.getValueF("saoPaoloRadius"));
	particleSystem.seoul.setRadius(gui.getValueF("seoulRadius"));
	particleSystem.beijing.setRadius(gui.getValueF("beijingRadius"));
	
	particleSystem.newYork.setPosition(ofDegToRad(gui.getValueF("newYorkLatitude")),
																		 ofDegToRad(gui.getValueF("newYorkLongitude")));
	particleSystem.london.setPosition(ofDegToRad(gui.getValueF("londonLatitude")),
																		ofDegToRad(gui.getValueF("londonLongitude")));
	particleSystem.saoPaolo.setPosition(ofDegToRad(gui.getValueF("saoPaoloLatitude")),
																			ofDegToRad(gui.getValueF("saoPaoloLongitude")));
	particleSystem.seoul.setPosition(ofDegToRad(gui.getValueF("seoulLatitude")),
																	 ofDegToRad(gui.getValueF("seoulLongitude")));
	particleSystem.beijing.setPosition(ofDegToRad(gui.getValueF("beijingLatitude")),
																		 ofDegToRad(gui.getValueF("beijingLongitude")));

	ParticleSystem::sphereCoverage = gui.getValueF("sphereCoverage");
	ParticleSystem::showAxes = gui.getValueB("showAxes");

	Particle::showCircles = gui.getValueF("showCircles");
	Particle::showFaces = gui.getValueF("showFaces");
	Particle::showCollisions = gui.getValueB("showCollisions");
	Particle::animateFaces = gui.getValueB("animateFaces");
	Particle::velocityDamping = gui.getValueF("velocityDamping");
	Particle::velocityMax = gui.getValueF("velocityMax");
	Particle::forceScale = gui.getValueF("forceScale");
	Particle::hardness = gui.getValueF("hardness");
	Particle::meanderForce = gui.getValueF("meanderForce");
	Particle::meanderJitter = gui.getValueF("meanderJitter");
	Particle::meanderRange = gui.getValueF("meanderRange");
	Particle::angleJitter = gui.getValueF("angleJitter");
	Particle::angleRange = gui.getValueF("angleRange");
	Particle::birthTime = gui.getValueI("birthTime");
	Particle::deathTime = gui.getValueI("deathTime");

	Face::faceScale = gui.getValueF("faceScale");
	
	City::showCities = gui.getValueB("showCities");

	particleSystem.update();
}

void testApp::draw() {
	ofSetColor(255, 255, 255);
	if(gui.getValueB("useCubemap")) {
		ofxCubeMapRenderer::render();

		bool saveImages = meta.getValueB("saveImages");
		if(saveImages) {
			cubemapFbo.begin();
			cubemapFbo.setBackground(0, 0, 0, 0);
		}
		ofxCubeMapRenderer::draw();
		if(saveImages) {
			cubemapFbo.end();
			cubemapImage.draw(0, 0);

			// load texture into pixels
			// in the future, should be updatePixels()
			ofTextureData& data = cubemapImage.getTextureReference().texData;
			glGetTexImage(
				data.textureTarget,
				0,
				data.glType,
				data.pixelType,
				cubemapImage.getPixels());

			stringstream paddedNumber;
			paddedNumber << setw(outputPadding) << setfill('0') << curFrame;
			cubemapImage.saveImage("render/" + outputPrefix + paddedNumber.str() + outputPostfix);

			curFrame++;
		}
	} else {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, .5);
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		ofScale(2, 2, 2);
		drawScene();
		ofPopMatrix();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
	}

	ofSetColor(255, 255, 255);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()) + " fps", ofGetWidth() - 80, 20);
}

void testApp::keyPressed(int key) {
	/*
	if(key == OF_KEY_ESC || key == 's')
		ofSaveScreen(
			"screenshots/" +
			ofToString(ofGetHours()) + " " +
			ofToString(ofGetMinutes()) + " " +
			ofToString(ofGetSeconds()) + ".png");
			*/
}

void testApp::drawScene() {
	if(gui.getValueB("useMouseCam")) {
		ofRotateY(mouseX - ofGetWidth() / 2);
		ofRotateX(-(mouseY - ofGetHeight() / 2));
	} else {
		ofRotateX(gui.getValueF("rotateX") * 180);
		ofRotateY(gui.getValueF("rotateY") * 180);
		ofRotateZ(gui.getValueF("rotateZ") * 180);
	}
	particleSystem.draw();
}
