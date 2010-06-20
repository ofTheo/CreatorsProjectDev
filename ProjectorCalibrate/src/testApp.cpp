#include "testApp.h"

void testApp::setup(){
	ofSetVerticalSync(true);
	
	wait = 8;
	
	camera.setup();
	camera.setBrightnessNorm(0);
	camera.setGammaNorm(0);
	camera.setGainNorm(0);
	camera.setExposureNorm(0);
	
	unsigned int min, max;
	camera.getShutterRange(&min, &max);
	currentShutter = max;
	camera.setShutter(currentShutter);
	
	frame = 0;
	saved = false;
	captureMode = false;
	overexposed = true;
	pass = 0;
	
	images.resize(maxPasses);
}

void testApp::update(){
}

void testApp::draw(){
	if(captureMode)
		drawCapture();
	else
		drawPrep();
}

void testApp::drawCapture() {
	int pattern = frame / wait;
	int brightness = pattern % 256;
	
	if(saved) {
		ofBackground(0, 0, 0);
		ofSetColor(255, 255, 255);
		ofDrawBitmapString("Done capturing and saving.", 20, 20);
	} else {
		if(brightness < 128)
			ofSetColor(255, 255, 255);
		else
			ofSetColor(0, 0, 0);
		ofDrawBitmapString(ofToString(pass) + "/" + ofToString(pattern), 10, 10);
		ofBackground(brightness, brightness, brightness);
	}
	
	if(overexposed) {
		int w = camera.getWidth();
		int h = camera.getHeight();
		
		// if we're on a capture frame
		if((frame + 1) % wait == 0) {
			
			// if we're on the first frame of the pass
			if(brightness % 256 == 0) {
				cout << "Shutter is running at " << camera.getShutterAbs() << endl;
				cout << "Allocating another pass of images." << endl;
				vector<ofImage>& cur = images[pass];
				cur.resize(256);
				for(int i = 0; i < cur.size(); i++)
					cur[i].allocate(w, h, OF_IMAGE_GRAYSCALE);
				cout << "Done allocating images." << endl;
			}
			
			// grab an image
			//cout << "Capturing frame " << pass << "/" << brightness << endl;
			ofImage& img = images[pass][brightness];
			camera.getOneShot(img);
			
			// if we're on the last frame of the pass
			if((brightness + 1) % 256 == 0) {
				unsigned char* pixels = img.getPixels();
				int clipping = 0;
				int n = w * h;
				for(int i = 0; i < n; i++)
					if(pixels[i] == 255)
						clipping++;
				cout << "Pass " << pass << ", percent clipping: " << (clipping * 100) / n << endl;
				if(clipping > n / 10) {
					currentShutter /= 2;
					camera.setShutter(currentShutter);
					cout << "Set new shutter to " << currentShutter << endl;
				} else {
					overexposed = false;
				}
				pass++;
			}
		}
	} else if(!saved) {
		processAndSave();
		saved = true;
	}
	frame++;
}

void testApp::drawPrep() {	
	ofImage cur;
	camera.getOneShot(cur);
	
	int w = (int) ofGetWidth();
	int h = (int) ofGetHeight();
	
	// draw a gradient
	for(int i = 0; i < 64; i++) {
		ofSetColor(i, i, i);
		float x = ofMap(i, 0, 64, 0, w);
		ofRect(x, 0, w / 63, h);
	}
	
	ofSetColor(255, 255, 255);
	ofTranslate(10, 10);
	cur.draw(0, 0);
	
	ofImage clipping;
	getClipping(cur, clipping);
	ofEnableAlphaBlending();
	clipping.update();
	ofSetColor(255, 0, 0);
	clipping.draw(0, 0);
	ofDisableAlphaBlending();
	
	ofSetColor(255, 255, 255);
	string msg = "Please open the iris on your camera\nuntil there are no dark pixels clipping.\nThen hit the return key.";
	ofDrawBitmapString(msg, 10, 10);
}

void testApp::getClipping(ofImage& img, ofImage& clipping) {
	int w = img.getWidth();
	int h = img.getHeight();
	clipping.allocate(w, h, OF_IMAGE_COLOR_ALPHA);
	unsigned char* imgPixels = img.getPixels();
	unsigned char* clippingPixels = clipping.getPixels();
	int n = w * h;
	for(int i = 0; i < n; i++) {
		if(imgPixels[i] == 0 || imgPixels[i] == 255) {
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

void testApp::keyPressed(int key) {
	if(key == OF_KEY_RETURN)
		captureMode = !captureMode;
	if(key == 'f')
		ofToggleFullscreen();
}

void testApp::processAndSave() {
	vector< vector<float> > brightness;
	vector< vector<int> > clipping;
	vector< vector<bool> > legit;
	LutFilter lut;
	
	lut.setup("firefly-lut.csv");
	
	brightness.resize(pass);
	clipping.resize(pass);
	legit.resize(pass);
	cout << "Analysing images for brightnss and clipping." << endl;
	for(int i = 0; i < pass; i++) {
		brightness[i].resize(256);
		clipping[i].resize(256);
		legit[i].resize(256);
		for(int j = 0; j < 256; j++) {
			cout << i << "/" << j;
			ofImage& cur = images[i][j];
			lut.filter(cur);
			int width = (int) cur.getWidth();
			int height = (int) cur.getHeight();
			int n = width * height;
			unsigned char* pixels = cur.getPixels();
			float& curBrightness = brightness[i][j];
			int& curClipping = clipping[i][j];
			for(int k = 0; k < n; k++) {
				curBrightness += pixels[k];
				if(pixels[k] == 255 || pixels[k] == 0)
					curClipping++;
			}
			legit[i][j] = curClipping < n / 10;
			curBrightness *= (1 << i);
			curBrightness /= n;
			cout << "(" << curBrightness << "/" << curClipping << "/" << legit[i][j] << ") ";
		}
	}
	cout << "Done loading images." << endl;
	
	
	cout << "Minimizing offsets." << endl;
	vector<float> offset(pass);
	for(int i = 1; i < pass; i++) {
		cout << "Minimizing pass " << i << endl;
		int count = 0;
		for(int j = 0; j < 256; j++) {
			if(legit[i][j] && legit[i - 1][j]) {
				offset[i] += brightness[i - 1][j] - brightness[i][j];
				count++;
			}
		}
		offset[i] /= count;
	}
	for(int i = 1; i < pass; i++)
		offset[i] += offset[i - 1];
	
	cout << "Applying offsets." << endl;
	for(int i = 1; i < pass; i++)
		for(int j = 0; j < 256; j++)
			brightness[i][j] += offset[i];
	
	cout << "Building single curve." << endl;
	vector<float> single;
	for(int j = 0; j < 256; j++) {
		int best = 0;
		int bestCount = clipping[0][j];
		for(int i = 1; i < pass; i++) {
			if(clipping[i][j] < bestCount) {
				best = i;
				bestCount = clipping[i][j];
			}
		}
		single.push_back(brightness[best][j]);
	}
	
	cout << "Writing out raw response." << endl;
	ofstream file;
	file.open(ofToDataPath("projector-response-raw.csv").c_str());
	for(int j = 0; j < 256; j++)
		file << single[j] << endl;
	file.flush();
	file.close();
	
	cout << "Applying double-ended low pass filter." << endl;
	smooth(single, .9);
	
	cout << "Normalizing single curve." << endl;
	normalize(single);
	
	cout << "Inverting single curve." << endl;
	invert(single);
	
	cout << "Writing out LUT." << endl;
	file.open(ofToDataPath("projector-lut.csv").c_str());
	for(int j = 0; j < 256; j++)
		file << single[j] << endl;
	file.flush();
	file.close();
}

void testApp::invert(vector<float>& f) {
	vector<float> fp;
	for(int i = 0; i < f.size(); i++) {
		for(int j = 0; j < f.size() - 1; j++) {
			if(f[j] <= i && f[j + 1] > i) {
				fp.push_back(ofMap(i, f[j], f[j + 1], j, j + 1));
				break;
			}
		}
	}
	f = fp;
}

void testApp::normalize(vector<float>& f) {
	float min = f[0];
	float max = f[0];
	for(int i = 0; i < 256; i++) {
		if(f[i] < min)
			min = f[i];
		if(f[i] > max)
			max = f[i];
	}
	for(int i = 0; i < 256; i++)
		f[i] = (f[i] - min) * 255 / (max - min);
}

void testApp::smooth(vector<float>& f, float lambda) {
	vector<float> fromLeft(256);
	vector<float> fromRight(256);
	fromLeft[0] = f[0];
	fromRight[255] = f[255];
	for(int i = 1; i < 256; i++) {
		fromLeft[i] = fromLeft[i - 1] * lambda + f[i] * (1 - lambda);
		int j = 255 - i;
		fromRight[j] = fromRight[j + 1] * lambda + f[j] * (1 - lambda);
	}
	for(int i = 0; i < 256; i++)
		f[i] = (fromLeft[i] + fromRight[i]) / 2;
}
