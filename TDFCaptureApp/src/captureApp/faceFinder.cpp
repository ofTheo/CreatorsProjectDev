/*
 *  faceFinder.cpp
 *  TDFCaptureApp
 *
 *  Created by theo on 19/06/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "faceFinder.h"

//------------------------------------------------------------------------------------------------------
void faceFinder::setup(string haarFile, int resizedW, int resizedH, float numSecondsToSeeFace, float numSecondsToForget){
	if( resizedW <= 0 || resizedH <= 0){
		printf("errooooooooor passed 0, 0 or less for image size\n");
		return;
	}
	
	resetCounters();
	numSecSeeFaceF		= numSecondsToSeeFace;
	numSecForgetF		= numSecondsToForget;
	confidenceAddAmnt	= 0.1;
	confidenceFadeRate  = 0.95;
	confidenceGateValStart  = 0.65;
	confidenceGateValStop   = 0.4;
	
	cvGrayResized.allocate(resizedW, resizedH);
	
	faceHaar.setup(haarFile);
	
	bSetup = true;
}

//------------------------------------------------------------------------------------------------------
void faceFinder::resetCounters(){
	numFaces			= 0;
	bSeeFace			= false;
	confidence			= 0.0;
	faceState			= FACE_NONE;
	timeStartedLooking	= ofGetElapsedTimef();
	timeStartedForget   = ofGetElapsedTimef();
}

//------------------------------------------------------------------------------------------------------
void faceFinder::updatePixels( unsigned char * pix, int w, int h  ){
	if( w <= 0 || h <= 0 || !bSetup){
		printf("Big Error faceFinder::update - returning \n");
		return;
	}
	
	if( cvGray.getWidth() != w || cvGray.getHeight() != h  || cvColor.getWidth() != w || cvColor.getHeight() != h ){
		cvGray.clear();
		cvColor.clear();		
		cvGray.allocate(w, h);
		cvColor.allocate(w, h);		
	}
	
	cvColor.setFromPixels(pix, w, h);
	
	if( cvGrayResized.getWidth() == cvColor.getWidth() && cvGrayResized.getHeight() == cvColor.getHeight() ){
		cvGrayResized = cvColor;
	}else{
		cvGray = cvColor;
		cvGrayResized.scaleIntoMe(cvGray);
	}
	
	numFaces = faceHaar.findHaarObjects(cvGrayResized);

}

void faceFinder::update(){
	
	confidence *= confidenceFadeRate;
	
	if( numFaces > 0 ){
		confidence += confidenceAddAmnt;
		confidence = ofClamp(confidence, 0, 1);
	}
	
	if( confidence >= confidenceGateValStart ){
		if( faceState == FACE_NONE ){
			timeStartedLooking = ofGetElapsedTimef();
			faceState = FACE_COUNTING_IN;
		}else if( faceState == FACE_COUNTING_OUT ){
			faceState = FACE_FOUND;
		}else if( faceState == FACE_COUNTING_IN && ofGetElapsedTimef() - timeStartedLooking > numSecSeeFaceF ){
			faceState = FACE_FOUND_FIRST;
		}else if( faceState == FACE_FOUND_FIRST ){
			faceState = FACE_FOUND;
		}
	}else if( confidence <= confidenceGateValStop ){
		if( faceState == FACE_FOUND || faceState == FACE_FOUND_FIRST ){
			timeStartedForget = ofGetElapsedTimef();
			faceState = FACE_COUNTING_OUT;
		}else if( faceState == FACE_COUNTING_IN ){
			faceState = FACE_NONE;
		}
		else if( faceState == FACE_COUNTING_OUT && ofGetElapsedTimef() - timeStartedForget > numSecForgetF ){
			faceState = FACE_NONE;
		}
	}

}

//------------------------------------------------------------------------------------------------------
void faceFinder::draw(float x, float y, float w, float h ){
	if(!bSetup)return;
	
	ofPushStyle();
		ofDrawBitmapString("STATE is "+getStateAsString(), x+10, y+20);
	//	faceHaar.draw(x, y, w, h);
	ofPopStyle();
}

//------------------------------------------------------------------------------------------------------
void faceFinder::draw(float x, float y){
	if(!bSetup)return;
		
	ofPushStyle();
		ofDrawBitmapString("STATE is "+getStateAsString(), x+10, y+20);	
	//	faceHaar.draw(x, y);
	ofPopStyle();
}

//------------------------------------------------------------------------------------------------------
bool faceFinder::firstSawFace(){
	return faceState == FACE_FOUND_FIRST;
}

//------------------------------------------------------------------------------------------------------
void faceFinder::clearFirstSawFace(){
	if( faceState == FACE_FOUND_FIRST ) faceState = FACE_FOUND;
}

//------------------------------------------------------------------------------------------------------
bool faceFinder::currentlySeeingFace(){
	return ( faceState == FACE_FOUND_FIRST || faceState == FACE_FOUND );
}

//------------------------------------------------------------------------------------------------------
float faceFinder::faceConfidence(){
	return confidence;
}
		