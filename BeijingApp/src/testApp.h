#pragma once

#include "ofMain.h"

#include "ofxAutoControlPanel.h"
#include "ofxCubeMapRenderer.h"
#include "ParticleSystem.h"

class testApp : public ofBaseApp, public ofxCubeMapRenderer {
public:
	void setup();
	void setupMeta();
	void setupGui();
	void setupCubemap();
	void update();
	void draw();
	void setupParticleSystem();
	void keyPressed(int key);

	void drawScene();

	bool lastFps;

	ParticleSystem particleSystem;

	simpleFileLister keyframeLister;
	int keyframeCount;
	ofxAutoControlPanel gui, meta;

	ofxFbo cubemapFbo;
	ofImage cubemapImage;

	bool lastSaveImages;
	int totalFrames;
	int curKeyframe;
	int startFrame, curFrame, endFrame;

	// various interpolated parameters
	float nextVelocityDamping, nextVelocityMax, nextForceScale, nextHardness;
	float nextMeanderForce, nextMeanderJitter, nextMeanderRange, nextAngleJitter, nextAngleRange;
	float nextSphereCoverage, nextFaceScale, nextBirthTime, nextDeathTime;
	int nextTotalCount;
	float nextNewYorkRadius, nextLondonRadius, nextSaoPaoloRadius, nextSeoulRadius, nextBeijingRadius;

	void grabInterpolatedValues();
	void interpolateValues();
	void interpolateValueF(string name, float target, float amt);
	void interpolateValueI(string name, int target, float amt);
};
