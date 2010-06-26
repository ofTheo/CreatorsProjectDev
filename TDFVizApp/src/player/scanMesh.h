
#pragma once

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxVectorMath.h"
#include "ofxControlPanel.h";


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
	
	
	void setup();
	void calcDepth(ofImage & currentFrame);
	void drawMesh(ofImage & currentFrame, float dx = 0);
	
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
	
	int numTotal; //= srcWidth * srcHeight;
	int srcWidth; // = 160;
	int srcHeight; // = 120;
};
