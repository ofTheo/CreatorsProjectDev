
#include "scanPlayer.h"
extern "C" {
#include "EnvMap.h"
}
#include "ofxVec3f.h"

#include "testApp.h"

#define sphereRes 240

static int sphereResolution = sphereRes;

ofxVec3f cachedPoints[sphereRes][sphereRes];
ofxVec3f cachedNormals[sphereRes][sphereRes];

ofxControlPanel * panelPtr;

scanPlayer::scanPlayer() {
	srcWidth  = 160;
	srcHeight = 120;
	numTotal = srcWidth * srcHeight;
}
	
//---------------------------------------------------------------------------------
float getIncrAmountForFps(float incrAmnt){
	float fps = ofGetFrameRate();
	fps = ofClamp(fps, 10, 100);
	float ratio = 60.0f/fps;
	return incrAmnt * ratio;
}

//---------------------------------------------------------------------------------
float getFadeForFps(float fadeAmnt){
	float fps = ofGetFrameRate();
	fps = ofClamp(fps, 10, 100);
	float ratio = 60.0f/fps;
	return powf(fadeAmnt, ratio);
}

//---------------------------------------------------------------------------------
void scanPlayer::setup(){
	maxNumFrames = 15 * 20; // 15 secs 2
	imageData.allocate(srcWidth,srcHeight, GL_RGB);
	currentFrame = 0;
	totalNumFrames = 0;
		
	//THEO
	extrudePct		= 0.0;
	perlinFacePct   = 0.0;
	buildCache();
	
	TSL.setup(maxNumFrames, srcWidth, srcHeight);
	
	FBO.allocate(1024, 768, GL_RGBA, 4);
	
	ofSetFrameRate(60.0);
	
	// setup the shader: 
	doSomething();
	endShader();
	
	mesh.setup(srcWidth, srcHeight, (float) srcWidth / inputWidth);
	
	//G H E T T O :) 
	panelPtr = &((testApp *)ofGetAppPtr())->panel;
	
	//THEO
	pState    = PLAYSTATE_UNLOADED;
	pctFadeIn = 0.0;
	mask.loadImage("mask.png");
	mask.setAnchorPercent(0.5, 0.5);
	
	maskX = 512 + panelPtr->getValueF("maskX");
	maskY = 384 + panelPtr->getValueF("maskY");
	maskW = panelPtr->getValueF("maskW");	
	maskH = panelPtr->getValueF("maskH");	
		
	dx = 90;
	dy = 0.0;
}

//---------------------------------------------------------------------------------
void scanPlayer::startFadeOut(){
	pState = PLAYSTATE_FADEOUT;
}

//---------------------------------------------------------------------------------
void scanPlayer::update(){
		
	if (TSL.state == TH_STATE_JUST_LOADED) {
		currentFrame	= 0;
		totalNumFrames	= TSL.totalNumFrames;
		TSL.state = TH_STATE_LOADED;			// make sure, but maybe not needed.
		
		//THEO 
		pctFadeIn	= 0.0;
		pState		= PLAYSTATE_FADEIN;
	}
	if (TSL.state != TH_STATE_LOADED){
		totalNumFrames = 0;
	} else {
		totalNumFrames = TSL.totalNumFrames;
	}
	
	if (totalNumFrames > 0){
		currentFrame++;
		
		// this is because some scans sequences were missing chunks of frames (ie output2) 
		// so I just make sure that we really do have something on this frame. 
		
		//while (TSL.bLoaded[currentFrame] == false){
		//	currentFrame++;
		//	currentFrame %= totalNumFrames;
		//}
		currentFrame %= totalNumFrames;
	}
	
	if( TSL.state != TH_STATE_LOADING ){
		histogram.setPixels(&(mesh.depthReal[0]), srcWidth, srcHeight);
		histogramAfter.setPixels(&(mesh.depth[0]), srcWidth, srcHeight);
	}
	
	//THEO ADDED 
	if( pState == PLAYSTATE_FADEIN ){
		pctFadeIn += getIncrAmountForFps(0.02);
		pctFadeIn = ofClamp(pctFadeIn, 0, 1);
		if( pctFadeIn >= 1.0 ){
			pState = PLAYSTATE_NORMAL;
		}
	}
	
	if( pState == PLAYSTATE_FADEOUT ){
		pctFadeIn *= getFadeForFps(0.93);
		if( pctFadeIn <= 0.01 ){
			pctFadeIn = 0.0;
		}
		pctFadeIn = ofClamp(pctFadeIn, 0, 1);
		if( pctFadeIn == 0.0 ){
			pState = PLAYSTATE_UNLOADED;
		}
	}	
	
	//THEO 
	perlinFacePct = ofMap( ofNoise(ofGetElapsedTimef() * -0.19, 100.0), 0.1, 0.95, 0.0, 1.0, true );
	extrudePct	  = perlinFacePct * pctFadeIn; //we have a extrude amount that is a combination of our face fade in and random perlin.
	maskX = 512 + panelPtr->getValueF("maskX");
	maskY = 384 + panelPtr->getValueF("maskY");
	maskW = panelPtr->getValueF("maskW");	
	maskH = panelPtr->getValueF("maskH");	
	dx	  = panelPtr->getValueF("rotateX");
	dy	  = panelPtr->getValueF("rotateY");
}

//---------------------------------------------------------------------------------
void scanPlayer::draw(){
	

	//if (TSL.state == TH_STATE_LOADED){
		if (totalNumFrames > 0){
			
			FBO.clear();
			FBO.begin();
				ofSetColor(0, 0, 0);
				ofRect(0, 0, 1024, 768);
		
				glEnable(GL_DEPTH_TEST);
				ofSetColor(255, 255, 255);
				
				
				if( panelPtr->getValueB("showFaceOnBall") ){
					drawBall();
				}else {
					drawMesh();
				}
								
				glDisable(GL_DEPTH_TEST);
				
				ofPushStyle();
					ofEnableAlphaBlending();
					ofSetColor(0, 0, 0, 255);
					mask.draw(maskX, maskY, maskW, maskH);
				ofPopStyle();
				
			FBO.end();
		}
	//}

	ofSetColor(255, 255, 255);
	FBO.draw(0, 0, 1024, 768);
	FBO.draw(1024, 0, 1024, 768);
		
}

//---------------------------------------------------------------------------------
void scanPlayer::loadDirectory(string pathName){
	if( pState != PLAYSTATE_UNLOADED ){
		printf("loadDirectory - warning SP is not yet unloaded!\n");
	}
	TSL.start(pathName);
}



//ghetto ripped from drawMesh - we use this in draw ball to do the normal calc and smoothing
//---------------------------------------------------------------------------------
void scanPlayer::calcDepth() {
	mesh.calcDepth(TSL.depthImageFrames[currentFrame]);
}

//---------------------------------------------------------------------------------
void scanPlayer::buildCache(){
	
	for(int i=0; i<sphereResolution; i++){
		float theta = (float)i/(sphereResolution-1) * TWO_PI;
		float nextTheta = (float)((i+1)%sphereResolution)/(sphereResolution-1) * TWO_PI;
		for(int j=0; j<sphereResolution; j++){		
				
				float phi   = (float)j/(sphereResolution-1) * PI;
			
				float x		= cos(theta) * sin(phi);
				float y		= sin(theta) * sin(phi);
				float z		= cos(phi);
				
				cachedPoints[i][j].set(x,y,z);
		}
	}
	

}

ofxVec3f pt;
inline void scanPlayer::sphereVertex(int i, int j, float pctI, float pctJ, float relief, ofxVec3f smoothedN){
	ofxVec3f sphereNormal;
	ofxVec3f normal;

	pt = cachedPoints[i][j];

	float noise = ofNoise(pt.x/3.0,pt.y/3.0,pt.z/3.0,ofGetElapsedTimef()*0.2);
	noise = 2*noise - 1;
	noise = fabs(noise);

	//TODO: note - we use the sphere's normal if we are in the mask
	// if we are in the face we use the face's normal - but it doens't match the angle of the face on the ball
	//so there is some funky reversing going on - I think its right????
	sphereNormal.set( pt.x + pt.x*noise*0.5, pt.y + pt.y*noise*0.5, pt.z + pt.z*noise*0.5 );
	if( relief == 1.0 ){
		normal = sphereNormal;
	}else{
		normal.set(-smoothedN.z, smoothedN.y, -smoothedN.x);
		normal *= extrudePct;
		normal += sphereNormal * (1.0 - extrudePct );
	}
	
	float amnt		= ofMap( fabs(j - sphereResolution/2), 0, sphereResolution/2, 1.0, 0.0 );
	float bumpNoise = ofNoise(pctI*33.0f, pctJ*33.0f, ofGetElapsedTimef());
	
	//normal *= (1.0-amnt);
	
	normal.x += 0.2 * bumpNoise * amnt;
	normal.y += 0.2 * bumpNoise * amnt;	
	
	pt.x *= relief;
	pt.y *= relief;
	pt.z *= relief;
	
	glNormal3fv(normal.v);
	glVertex3f( (pt.x + pt.x*noise*0.5), (pt.y + pt.y*noise*0.5), (pt.z + pt.z*noise*0.5));
}

void scanPlayer::drawBall(){
	
	//we use this to smooth our normals
	calcDepth();

	if( panelPtr->getValueB("doShader") ){
		startShader();
	}
	
	glEnable(GL_NORMALIZE);
	
	glPushMatrix();
	glTranslatef(512, ofGetHeight()/2, -100);
	glRotatef(dx, 0, 1, 0);
	glRotatef(dy, 0, 0, 1);
	
	glScalef(panelPtr->getValueF("ballScale"), panelPtr->getValueF("ballScale"),panelPtr->getValueF("ballScale"));
	//ofRotate(ofGetElapsedTimef()*100, 1,1,0.2);
	//ofRotate(180, 0, 1, 0);
	//ofRotate(90, 1, 0, 0);
	
	float theta,nextTheta, phi, x, y, z;
	
	float scale = 190.0;
	float relief = 1.0;

	
	float depthAmnt = 0.0;
	
	float bPX = 0.35;
	float tPX = 0.65;

	float bPY = 0.4;
	float tPY = 0.6;	
	
	ofxVec3f normal;
	ofxVec3f smoothedN;
	
	float zFaceScale = panelPtr->getValueF("zPercent") * 0.008;

	//THEO
	ofxVec3f sphereNormal;
	
	for(int i=0; i<sphereResolution; i++){

		float nextPctI	= ofMap(i+1, 0, sphereResolution, 1.0, 0.0, true);
		float pctI		= ofMap(i, 0, sphereResolution, 1.0, 0.0, true);

		glBegin(GL_QUAD_STRIP);
		for(int j=0; j<sphereResolution; j++){
			
			float pctJ = ofMap(j, 0, sphereResolution, 0.0, 1.0, true);

			if( pctJ >= bPX && pctJ < tPX && pctI >= bPY && pctI < tPY ){
				int depthIX = ofMap(pctJ, bPX, tPX, (float)srcWidth * 0.3, (float)srcWidth * 0.7, true);
				int depthIY = ofMap(pctI, bPY, tPY, 0, srcHeight, true);
				depthAmnt = mesh.depth[depthIX + srcWidth*depthIY];
				smoothedN = mesh.normalsSmoothed[depthIX + srcWidth*depthIY];
				//THEO
				relief =  1.0 + depthAmnt * zFaceScale * extrudePct;
			}else {
				relief = 1.0;
			}
				
			sphereVertex( i, j, pctI, pctJ, relief, smoothedN);
						 
						 
			if( pctJ >= bPX && pctJ < tPX && nextPctI >= bPY && nextPctI < tPY ){
				int depthIX = ofMap(pctJ, bPX, tPX, (float)srcWidth * 0.3, (float)srcWidth * 0.7, true);
				int depthIY = ofMap(nextPctI, bPY, tPY, 0, srcHeight, true);
				depthAmnt = mesh.depth[depthIX + srcWidth*depthIY];
				smoothedN = mesh.normalsSmoothed[depthIX + srcWidth*depthIY];
				//THEO
				relief =  1.0 + depthAmnt * zFaceScale * extrudePct;
			}else {
				relief = 1.0;
			}			
			
			sphereVertex((i+1)%sphereResolution, j, nextPctI, pctJ, relief, smoothedN);
			
		}
		glEnd();
	}	
	
	glPopMatrix();

	if( panelPtr->getValueB("doShader") ){
		endShader();
	}	
}

//---------------------------------------------------------------------------------
void scanPlayer::drawMesh() {
	
	mesh.drawMesh(TSL.depthImageFrames[currentFrame], dx);	

}