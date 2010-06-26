/*
 *  threadedScanLoader.cpp
 *  emptyExample
 *
 *  Created by itotaka on 1/13/10.
 *  Copyright 2010 YCAM. All rights reserved.
 *
 */

#include "threadedScanLoader.h"

//--------------------------------------------------------------
threadedScanLoader::threadedScanLoader() :
		folderName(""),
		state(TH_STATE_UNLOADED),
		resizeW(0),
		resizeH(0) {
}

//--------------------------------------------------------------
void threadedScanLoader::setup(int maxNumFramesIn, int resizeToW, int resizeToH){
	maxNumFrames	= maxNumFramesIn;
	totalNumFrames	= 0;
	
	resizeW		 = resizeToW;
	resizeH		 = resizeToH;
	
	depthImageFrames.assign(maxNumFrames, ofImage());
//	for (int i = 0; i < maxNumFrames; i++){
//		imageFrames[i].setUseTexture(false);
//	}
	
//	depthImageFrames = new ofImage[maxNumFrames];
//	//imageFrames = new ofImage[maxNumFrames];
//	bLoaded		= new bool	 [maxNumFrames];
//	for (int i = 0; i < maxNumFrames; i++){
//		depthImageFrames[i].allocate(640, 480, OF_IMAGE_COLOR_ALPHA);
//		depthImageFrames[i].setUseTexture(false);
//		//imageFrames[i].setUseTexture(false);
//		bLoaded[i] = false;
//	}
//	totalNumFrames = 0;
}

//--------------------------------------------------------------
threadedScanLoader::~threadedScanLoader() {
}

//--------------------------------------------------------------
void threadedScanLoader::loadScans(string pathName) {
	
	totalNumFrames = 0;
	
	ofxDirList directory;
	directory.allowExt("png");
	directory.allowExt("tga");
	directory.allowExt("jpg");

		
	int howMany = directory.listDir(pathName); 
	//howMany		= ofClamp(howMany, 0, maxNumFrames);
	
	if( howMany <= 0 ){
		printf("no images found!\n");
	}else{
				
		for (int i = 0; i <  howMany; i++){
			
			string fileName = directory.getName(i);		
			cout << "file name " << fileName << endl;
			
			depthImageFrames[i].setUseTexture(false);
			depthImageFrames[i].loadImage(directory.getPath(i));
			
			if( depthImageFrames[i].width != resizeW || depthImageFrames[i].height != resizeH ){
				printf("%i %i\n", resizeW, resizeH);
				depthImageFrames[i].resize(resizeW,resizeH);
			}
		}
		

		totalNumFrames = howMany;
	}

	cout << howMany << endl;
	
	if (howMany > 0){
		state = TH_STATE_JUST_LOADED;
	} else {
		state = TH_STATE_UNLOADED;
	}

	stop();

}

//--------------------------------------------------------------
void threadedScanLoader::unload() {
	
	if(true){
		state = TH_STATE_UNLOADED;
		//imgLoaded = false;
	}
}


//--------------------------------------------------------------
bool threadedScanLoader::start(string & _folderName) {

	if (!isThreadRunning() && (_folderName != "")) {
		state		= TH_STATE_LOADING;
		folderName	= _folderName;
		startThread(true, false);  // blocking, verbose
		return true;
	}else{
		return false;
	}
}

//--------------------------------------------------------------
void threadedScanLoader::stop() {
	stopThread();
}

//--------------------------------------------------------------
void threadedScanLoader::threadedFunction() {
	while (isThreadRunning() != 0){
		loadScans(folderName);
	}
}

//--------------------------------------------------------------

