#pragma once

#include "ofMain.h"
#include "ofxShader.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	ofxShader shader;
	ofImage displacement;

	float rotX, rotY;
	static const float rotSmooth = .9;
};
