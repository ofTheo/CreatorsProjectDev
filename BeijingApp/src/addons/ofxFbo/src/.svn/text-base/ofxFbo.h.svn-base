#pragma once

#include "ofAppRunner.h"
#include "ofGraphics.h"

/*
	add multisampling support
	need to be able to load results back to an ofImage maybe using glGetTexImage
	add all the checks for warnings and errors (just before begin())
	add OpenGLes support
	should ofFbo override the screen width and screen height returned by ofGetWidth() and ofGetHeight()?
*/

class ofxFbo {
public:
	ofxFbo();
	void setup(int width, int height, bool useDepth = true, bool useStencil = false);
	~ofxFbo();

	void setFov(float fov);
	void setBackground(float r, float g, float b, float a = 255);
	void clearAlpha();

	void attach(ofTexture& target, int position = 0);
	void detach(int position = 0);

	void begin();
	void end();

	void draw(float x, float y);
	void draw(float x, float y, float width, float height);

protected:
	int width, height;
	float fov;
	GLuint fboId, depthId, stencilId;
	ofTexture internalColor;
	vector<ofTexture*> attachments;

	int levels;
	void push();
	void pop();
	ofTexture* getAttachment();
	void checkAttachment();
	void setupScreenForFbo();
	void setupScreenForWindow();
};
