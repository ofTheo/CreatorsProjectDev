
#pragma once

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxDirList.h"
#include "threadedScanLoader.h"
#include "ofxVectorMath.h"
#include "ofxControlPanel.h"

class face {
public: 
	int v0;
	int v1;
	int v2;
	bool bVisible;
	ofxVec3f nrml;
};

class scanPlayer {

public:
	
	//~scanPlayer();
	void setup();
	void loadDirectory(string pathName);
	
	ofImage *	depthFrames;
	ofImage *	imageFrames;
	bool	*	bLoaded;
	int			maxNumFrames;	// we should do this at some point
	
	int			totalNumFrames;
	int			currentFrame;
	
	ofTexture	imageData;
	
	threadedScanLoader TSL;
	
	void		update();
	void		draw();
	void		drawMesh();
	
	float		dx;
	
	ofxVec3f	* vertices;
	face		* faces;
	ofxVec3f	* normals;
	ofxVec3f	* normalsSmoothed;
	int			nVertices;
	int			nFaces;
	

	
	
};
