#pragma once

#include "ofMain.h"
#include "ofxDirList.h"

class Face {
public:
	static bool mirrorAnimation;
	static float faceScale;

	ofxDirList dirList;
	int dirSize;
	int curImage;
	bool countingUp;

	ofImage img;
	int w, h;
	float xMean, yMean;
	//float xStdDev, yStdDev;

	unsigned char id;

	bool needsUpdating;

	Face();
	void setup(unsigned char id, string path);
	void update();
	void next();
	void fillId();
	void analyzeImage();
	void draw();
};
