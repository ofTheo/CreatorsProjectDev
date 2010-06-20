
#include "scanPlayer.h"


void scanPlayer::setup(){

	maxNumFrames = 15 * 20; // 15 secs 2
	/*
	depthFrames = new ofImage[maxNumFrames];
	imageFrames = new ofImage[maxNumFrames];
	bLoaded		= new bool	 [maxNumFrames];
	
	for (int i = 0; i < maxNumFrames; i++){
		depthFrames[i].setUseTexture(false);
		imageFrames[i].setUseTexture(false);
		bLoaded[i] = false;
	}
	 */
	
	imageData.allocate(640,480, GL_RGB);

	currentFrame = 0;
	totalNumFrames = 0;
	
	TSL.setup();
}


void scanPlayer::update(){
	
	
	if (TSL.state == TH_STATE_JUST_LOADED) {
		currentFrame = 0;
		totalNumFrames = TSL.totalNumFrames;
		TSL.state = TH_STATE_LOADED;			// make sure, but maybe not needed.
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
		
		while (TSL.bLoaded[currentFrame] == false){
			currentFrame++;
			currentFrame %= totalNumFrames;
		}
		currentFrame %= totalNumFrames;
		
	}
	
}

void scanPlayer::draw(){
	
	if (TSL.state == TH_STATE_LOADED){
		if (totalNumFrames > 0){
			imageData.loadData(TSL.imageFrames[currentFrame].getPixels(), 640,480, GL_RGB);
			ofSetColor(255, 255, 255);
			imageData.draw(300,0);
		}
	}
}


void scanPlayer::loadDirectory(string pathName){
	
	
	TSL.start(pathName);

};