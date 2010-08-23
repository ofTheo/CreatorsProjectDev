#pragma once

#include "ofMain.h"
#include "ofxFbo.h"

class ofxCubeMapRenderer {
public:
	ofxCubeMapRenderer();
	void setup(int cubeSize);
	void render();
	void draw();
	virtual void drawScene() = 0;

protected:
	void renderDirection(ofTexture& target,
			float targetX, float targetY, float targetZ,
			float upX, float upY, float upZ);
	void pushEverything();
	void popEverything();

	int cubeSize;
	ofxFbo fbo;
	ofTexture frontView, leftView, backView, rightView, upView, downView;
};
