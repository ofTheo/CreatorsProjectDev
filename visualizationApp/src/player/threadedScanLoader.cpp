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
threadedScanLoader::threadedScanLoader() {
	folderName = "";
	state = TH_STATE_UNLOADED;
}


void threadedScanLoader::setup(){
	maxNumFrames = 15 * 20; // 15 secs 2
	depthFrames = new ofImage[maxNumFrames];
	imageFrames = new ofImage[maxNumFrames];
	bLoaded		= new bool	 [maxNumFrames];
	for (int i = 0; i < maxNumFrames; i++){
		depthFrames[i].setUseTexture(false);
		imageFrames[i].setUseTexture(false);
		bLoaded[i] = false;
	}
	totalNumFrames = 0;
}

//--------------------------------------------------------------
threadedScanLoader::~threadedScanLoader() {
}

//--------------------------------------------------------------
void threadedScanLoader::loadScans(string pathName) {
	
	
	for (int i = 0; i < maxNumFrames; i++){
		bLoaded[i] = false;
	}
	
	totalNumFrames = 0;
	
	ofxDirList directory;
	int howMany = directory.listDir(pathName); //MIN(maxNumFrames, directory.listDir(pathName));
	for (int i = 0; i < howMany; i++){
		
		string fileName = directory.getName(i);
		//cout << "file name " << fileName << endl;
		vector < string > nameSplit = ofSplitString(fileName, "-");
		
		if (nameSplit.size() == 3){
			
			int whoAmI = atoi(nameSplit[1].c_str());
			bool bDepth = nameSplit[2] == "depth.png" ? true : false;
			if (whoAmI > maxNumFrames) continue; // we can't take these frames;
			//cout << "whoAmI " << whoAmI << " bDepth "  << bDepth << endl;
			if (whoAmI > totalNumFrames) totalNumFrames = whoAmI;
			bLoaded[whoAmI] = true; // a bit of a sanity check since listing is not necessarily in order. 
			
			if (bDepth == true){
				depthFrames[whoAmI].loadImage(pathName + "/" + fileName);
				//ofSleepMillis(10);
			} else {
				imageFrames[whoAmI].loadImage(pathName + "/" + fileName);
				//ofSleepMillis(10);
			}
		}
	}
	
	cout << totalNumFrames << endl;
	
	if (totalNumFrames > 0){
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
		state = TH_STATE_LOADING;
		folderName = _folderName;
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
	while (isThreadRunning() != 0) {
			loadScans(folderName);
	}
}

//--------------------------------------------------------------

