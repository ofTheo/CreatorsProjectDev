#include "testApp.h"

void testApp::setup() {
	displacement.loadImage("displacement.png");
	int w = (int) displacement.getWidth();
	int h = (int) displacement.getHeight();

	shader.setup("shader");
	shader.begin();

	shader.setUniform1f("depthScaling", 255 * 2.5);
	shader.setUniform3f("baseColor", 1, 0, 0);
	shader.setUniform3f("lightPosition", 0, 200, 200);

	// this effectively creates some smoothing of the normals
	shader.setUniform2f("normalOffset", 8. / w, 8. / h);

	displacement.getTextureReference().bind();
	// necessary to use GL_NEAREST for sampler2D in vertex shader
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// need to reupload the image data, i'm not sure why...
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, displacement.getPixels());

	shader.setSampler2d("displacementMap", displacement, 0);

	shader.end();

	rotX = 0;
	rotY = 0;

	glEnable(GL_DEPTH_TEST);
}

void testApp::update(){
}

void testApp::draw(){
	ofBackground(128, 128, 128);

	int w = (int) displacement.getWidth();
	int h = (int) displacement.getHeight();

	glTranslatef(ofGetWidth() / 2, ofGetHeight() / 2, 0);
	rotX = ofLerp(mouseX, rotX, rotSmooth);
	rotY = ofLerp(mouseY, rotY, rotSmooth);
	glRotatef(rotX, 0, 1, 0);
	glRotatef(-rotY, 1, 0, 0);
	glTranslatef(-w / 2, -h / 2, -320);

	shader.begin();

	// this whole bit can be moved into a vbo or even display list, as it never changes
	int step = 2;
	glBegin(GL_TRIANGLES);
	for(int y = 0; y < h - step; y += step) {
		for(int x = 0; x < w - step; x += step) {
			glTexCoord2f(x / (float) w, y / (float) h);
			glVertex2f(x, y);

			glTexCoord2f((x + step) / (float) w, (y + step) / (float) h);
			glVertex2f(x + step, y + step);

			glTexCoord2f((x + step) / (float) w, y / (float) h);
			glVertex2f(x + step, y);

			glTexCoord2f(x / (float) w, y / (float) h);
			glVertex2f(x, y);

			glTexCoord2f((x + step) / (float) w, (y + step) / (float) h);
			glVertex2f(x + step, y + step);

			glTexCoord2f(x / (float) w, (y + step) / (float) h);
			glVertex2f(x, y + step);
		}
	}
	glEnd();

	shader.end();
}
