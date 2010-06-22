#include "testApp.h"

void testApp::setup(){
	passCount = 16; // how many images to capture
	wait = 100; // how long to wait, in ms, between captures
	camera.setImageType(OF_IMAGE_GRAYSCALE); // comment this out for color
	
	camera.setup();
	// set everything as low as possible
	camera.setBrightnessNorm(0);
	camera.setGammaNorm(0);
	camera.setGainNorm(0);
	camera.setExposureNorm(0);

	camera.setShutterNorm(.5);
	
	pass = 0;	
	images.resize(passCount);
	shutter.resize(passCount);
	captureMode = false;
	saved = false;
}

void testApp::update(){
	if(captureMode) {
		if(pass < passCount) {
			float curShutter = (float) pass / passCount;
			camera.setShutterNorm(curShutter);
			ofSleepMillis(wait); // need to wait for settings to be applied
			camera.getOneShot(curImage);
			images[pass] = curImage;
			shutter[pass] = camera.getShutterAbs();
			pass++;
		} else {
			if(!saved) {
				cout << "Saving images." << endl;
				for(int i = 0; i < passCount; i++) {
					int curShutter = (int) (1000 * 1000 * shutter[i]); // us
					images[i].saveImage(ofToString(curShutter) + ".png");
				}
				cout << "Images saved." << endl;
				saved = true;
			}
		}
	} else {
		camera.getOneShot(curImage);
	}
}

void testApp::draw(){
	ofBackground(0, 0, 0);
	curImage.setAnchorPercent(.5, .5);
	curImage.draw(ofGetWidth() / 2, ofGetHeight() / 2);
	if(captureMode)
		drawCapture();
	else
		drawPrep();
}

void testApp::drawCapture() {
	ofSetColor(255, 255, 255);
	stringstream description;
	if(saved)
		description << "Done saving files." << endl;
	else
		description << "Capturing pass " << pass << "/" << passCount << endl;
	ofDrawBitmapString(description.str(), 10, 10);
}

void testApp::drawPrep() {
	ofSetColor(255, 255, 255);
	ofDrawBitmapString("Point the camera at a high dynamic range scene.\nThen hit the return key.", 10, 10);
}

void testApp::keyPressed(int key) {
	if(key == OF_KEY_RETURN)
		captureMode = !captureMode;
}
