#pragma once

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxVectorMath.h"
#include "ofxControlPanel.h"
#include "ofxOpenCv.h"

class face {
public: 
	int v0;
	int v1;
	int v2;
	bool bVisible;
	ofxVec3f nrml;
};

class scanMesh {
public:
	void setup(int srcWidth, int srcHeight, float sizeScaling);
	void calcDepth(ofImage & currentFrame);
	void drawMesh(ofImage & currentFrame, float dx = 0);
	// add an extra function for loading an ofImage, storing values internally
	
	vector <bool> mask;
	vector <float> depth;
	vector <unsigned char> depthReal;

	ofxVec3f	* vertices;
	face		* faces;
	ofxVec3f	* normals;
	ofxVec3f	* normalsSmoothed;
	int			nVertices;
	int			nFaces;
	
	ofxControlPanel * panelPtr;
	
	int numTotal;
	int srcWidth;
	int srcHeight;
	
	// +/-320 means a range of 640
	// PNG alpha is range of 256
	// so 640 divided by 256 is 2.5
	static const float alphaScaling = 2.5;
	
	// e.g., 640x480 -> 160x120 means 1/4 = .25
	float sizeScaling;
};
