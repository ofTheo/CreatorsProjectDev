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
}

//--------------------------------------------------------------
threadedScanLoader::~threadedScanLoader() {
}

// this could also be done with OpenCV, cvResize + CV_INTER_NN
// or even faster by precomputing a remap function
void threadedScanLoader::resize(ofImage& from, ofImage& to, int toWidth, int toHeight) {
	to.allocate(toWidth, toHeight, OF_IMAGE_COLOR_ALPHA);
	unsigned char* fromPixels = from.getPixels();
	unsigned char* toPixels = to.getPixels();
	int toPosition = 0;
	int fromWidth = from.getWidth();
	int fromHeight = from.getHeight();
	for(int toy = 0; toy < toHeight; toy++) {
		int fromy = (toy * fromHeight) / toHeight;
		int fromPosition = fromy * fromWidth;
		for(int tox = 0; tox < toWidth; tox++) {
			int fromx = (tox * fromWidth) / toWidth;
			int cur = (fromPosition + fromx) * 4;
			toPixels[toPosition++] = fromPixels[cur + 0];
			toPixels[toPosition++] = fromPixels[cur + 1];
			toPixels[toPosition++] = fromPixels[cur + 2];
			toPixels[toPosition++] = fromPixels[cur + 3];
		}
	}
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
		ofImage loader;
		loader.setUseTexture(false);
		
		for (int i = 0; i <  howMany; i++){
			string fileName = directory.getName(i);		
			cout << "file name " << fileName << endl;
			loader.loadImage(directory.getPath(i));
			depthImageFrames[i].setUseTexture(false);
			resize(loader, depthImageFrames[i], resizeW, resizeH);
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

