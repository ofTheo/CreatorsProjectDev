/*
 *  appConstants.h
 *  TDFCaptureApp
 *
 *  Created by theo on 21/06/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once 

#include "ofMain.h"

#define CAPTURE_MAIN_FOLDER "input/"
#define FRAME_CAPTURE_NAME "capture"
#define FRAME_CAPTURE_FOLDER (FRAME_CAPTURE_NAME + string("/"))
#define EXPORT_FOLDER "output/"
#define DECODE_FOLDER (EXPORT_FOLDER + string("incoming/"))

static string getTimeAsUniqueString(){
	string unique = ofToString(ofGetYear()) + ofToString(ofGetMonth()) + ofToString(ofGetDay()) + ofToString(ofGetHours()) + ofToString(ofGetMinutes()) + ofToString(ofGetSeconds());
	return unique;
}