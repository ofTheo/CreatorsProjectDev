#include "ofxCubeMapRenderer.h"

ofxCubeMapRenderer::ofxCubeMapRenderer() {
}

void ofxCubeMapRenderer::setup(int cubeSize) {
	this->cubeSize = cubeSize;

	fbo.setup(cubeSize, cubeSize);

	int type = GL_RGBA;
	frontView.allocate(cubeSize, cubeSize, type);
	rightView.allocate(cubeSize, cubeSize, type);
	backView.allocate(cubeSize, cubeSize, type);
	leftView.allocate(cubeSize, cubeSize, type);
	upView.allocate(cubeSize, cubeSize, type);
	downView.allocate(cubeSize, cubeSize, type);
}

void ofxCubeMapRenderer::render() {
	// x+ is right, y- is up, z+ is forward
	renderDirection(frontView, 0, 0, 1, 0, -1, 0);
	renderDirection(rightView, 1, 0, 0, 0, -1, 0);
	renderDirection(backView, 0, 0, -1, 0, -1, 0);
	renderDirection(leftView, -1, 0, 0, 0, -1, 0);
	renderDirection(upView, 0, -1, 0, 0, 0, -1);
	renderDirection(downView, 0, 1, 0, 0, 0, 1);

	// restore the main window viewport
	// is there a better way?
	glViewport(0, 0, ofGetWidth(), ofGetHeight());
}

void ofxCubeMapRenderer::draw() {
	glPushMatrix();
	glTranslatef(0, cubeSize * 3, 0);
	glScalef(1, -1, 1);
	leftView.draw(cubeSize * 0, cubeSize * 1);
	frontView.draw(cubeSize * 1, cubeSize * 1);
	rightView.draw(cubeSize * 2, cubeSize * 1);
	backView.draw(cubeSize * 3, cubeSize * 1);
	upView.draw(cubeSize * 1, cubeSize * 2);
	downView.draw(cubeSize * 1, cubeSize * 0);
	glPopMatrix();
}

void ofxCubeMapRenderer::renderDirection(ofTexture& target,
		float targetX, float targetY, float targetZ,
		float upX, float upY, float upZ) {

	pushEverything();

	fbo.attach(target);
	fbo.setBackground(0, 0, 0, 0);
	fbo.begin();

	// setup the projection matrix
	// render a square aspect, 90 degree fov
	// at the size of the fbo
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, cubeSize, cubeSize);
	gluPerspective(90, 1, 1, 1000);

	// setup the camera in the center
	// looking in the direction specified
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0, 0, 0,
		targetX, targetY, targetZ,
		upX, upY, upZ);

	// draw the whole scene
	drawScene();

	fbo.end();

	popEverything();
}

void ofxCubeMapRenderer::pushEverything() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void ofxCubeMapRenderer::popEverything() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
