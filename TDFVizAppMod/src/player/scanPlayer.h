
#pragma once

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxDirList.h"
#include "threadedScanLoader.h"
#include "ofxVectorMath.h"
#include "ofxControlPanel.h"
#include "histogramGuiElement.h"
#include "ofxFBOTexture.h"
#include "scanMesh.h"


//THEO
typedef enum{
	PLAYSTATE_UNLOADED,
	PLAYSTATE_NEW_FACE,
	PLAYSTATE_FADEIN,
	PLAYSTATE_NORMAL,
	PLAYSTATE_FADEOUT
}playState;

class scanPlayer {

public:
	scanPlayer();
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
	ofxFBOTexture FBO;
	
	threadedScanLoader TSL;
	histogramGuiElement histogram;
	histogramGuiElement histogramAfter;
	
	void		buildCache();
	
	void		update();
	void		draw();
	void		drawBall();
	
	void		sphereVertex(int i, int j, float pctI, float pctJ, float relief, ofxVec3f smoothedN);
	
	void		drawMesh();
	void		startFadeOut();
	void		calcDepth();
	
	float		dx, dy;
	scanMesh	mesh;
	
	ofImage mask;
	
	static const int
		inputWidth = 640,
		inputHeight = 480;
	int srcWidth, srcHeight, numTotal;

	//theo - don't mod below here
	float pctFadeIn;
	float perlinFacePct;
	float extrudePct;
	playState pState;
	float maskX, maskY, maskW, maskH;
};
