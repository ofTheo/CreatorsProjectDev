#pragma once

#include "ofMain.h"

#include "ofxAutoControlPanel.h"
#include "ofxCubeMapRenderer.h"
#include "ParticleSystem.h"

class testApp : public ofBaseApp, public ofxCubeMapRenderer {
public:
	void setup();
	void setupOutput();
	void setupMeta();
	void setupGui();
	void setupCubemap();
	void update();
	void draw();
	void setupParticleSystem();
	void keyPressed(int key);

	void drawScene();
	
	string outputPrefix, outputExtension;

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
	float curVelocityDamping, curVelocityMax, curForceScale, curHardness;
	float curMeanderForce, curMeanderJitter, curMeanderRange, curAngleJitter, curAngleRange;
	float curSphereCoverage, curFaceScale, curBirthTime, curDeathTime;
	int curTotalCount;
	float curNewYorkRadius, curLondonRadius, curSaoPaoloRadius, curSeoulRadius, curBeijingRadius;
	void grabCurValues();
	
	float nextVelocityDamping, nextVelocityMax, nextForceScale, nextHardness;
	float nextMeanderForce, nextMeanderJitter, nextMeanderRange, nextAngleJitter, nextAngleRange;
	float nextSphereCoverage, nextFaceScale, nextBirthTime, nextDeathTime;
	int nextTotalCount;
	float nextNewYorkRadius, nextLondonRadius, nextSaoPaoloRadius, nextSeoulRadius, nextBeijingRadius;
	void grabNextValues();
	
	void interpolateValues();
};
