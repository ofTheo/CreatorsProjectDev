#pragma once

#include "ofMain.h"
#include "ofxDirList.h"

class Face {
public:
	static float faceScale;

	ofxDirList dirList;
	int dirSize;
	int curImage;
	bool countingUp;

	ofImage img;
	int w, h;
	float xMean, yMean;
	//float xStdDev, yStdDev;

	unsigned char color;

	bool needsUpdating;

	Face();
	void setup(unsigned char color, string path);
	void update();
	void next();
	void fillCityColor();
	void analyzeImage();
	void draw();
};
