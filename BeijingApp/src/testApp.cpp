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
	setupMeta();
	setupGui();
	setupCubemap();
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
	gui.addSlider("New York Count", "newYorkCount", 0, 0, 100, true);
	gui.addSlider("London Count", "londonCount", 0, 0, 100, true);
	gui.addSlider("Sao Paolo Count", "saoPaoloCount", 0, 0, 100, true);
	gui.addSlider("Seoul Count", "seoulCount", 0, 0, 100, true);
	gui.addSlider("Beijing Count", "beijingCount", 0, 0, 100, true);
	gui.addSlider("keyframe length", "keyframeLength", 1, 1, 1800, true);
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

void testApp::grabInterpolatedValues() {
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
	nextNewYorkCount = gui.getValueI("newYorkCount");
	nextLondonCount = gui.getValueI("londonCount");
	nextSaoPaoloCount = gui.getValueI("saoPaoloCount");
	nextSeoulCount = gui.getValueI("seoulCount");
	nextBeijingCount = gui.getValueI("beijingCount");
}

void testApp::interpolateValueF(string name, float target, float amt) {
	gui.setValueF(name, ofLerp(gui.getValueF(name), target, amt));
}

void testApp::interpolateValueI(string name, int target, float amt) {
	gui.setValueI(name, ofLerp(gui.getValueI(name), target, amt));
}

void testApp::interpolateValues() {
	float i = (float) (curFrame - startFrame) / (endFrame - startFrame);
	interpolateValueF("velocityDamping", nextVelocityDamping, i);
	interpolateValueF("velocityMax", nextVelocityMax, i);
	interpolateValueF("forceScale", nextForceScale, i);
	interpolateValueF("hardness", nextHardness, i);
	interpolateValueF("meanderForce", nextMeanderForce, i);
	interpolateValueF("meanderJitter", nextMeanderJitter, i);
	interpolateValueF("meanderRange", nextMeanderRange, i);
	interpolateValueF("angleJitter", nextAngleJitter, i);
	interpolateValueF("angleRange", nextAngleRange, i);
	interpolateValueF("sphereCoverage", nextSphereCoverage, i);
	interpolateValueF("faceScale", nextFaceScale, i);
	interpolateValueF("birthTime", nextBirthTime, i);
	interpolateValueF("deathTime", nextDeathTime, i);
	interpolateValueI("newYorkCount", nextNewYorkCount, i);
	interpolateValueI("londonCount", nextLondonCount, i);
	interpolateValueI("saoPaoloCount", nextSaoPaoloCount, i);
	interpolateValueI("seoulCount", nextSeoulCount, i);
	interpolateValueI("beijingCount", nextBeijingCount, i);
}

void testApp::update() {
	if(meta.getValueB("refreshList")) {
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
				grabInterpolatedValues();
				gui.loadSettings(keyframeLister.getPath(curKeyframe++));
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

	particleSystem.newYork.moveTowards(gui.getValueI("newYorkCount"));
	particleSystem.london.moveTowards(gui.getValueI("londonCount"));
	particleSystem.saoPaolo.moveTowards(gui.getValueI("saoPaoloCount"));
	particleSystem.seoul.moveTowards(gui.getValueI("seoulCount"));
	particleSystem.beijing.moveTowards(gui.getValueI("beijingCount"));

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

			cubemapImage.saveImage("render/" + ofToString(curFrame) + ".tif");

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
