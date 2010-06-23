/*
 *  faceFinder.h
 *  TDFCaptureApp
 *
 *  Created by theo on 19/06/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
 
#include "ofxCvHaarFinder.h"

typedef enum{
	FACE_NONE,
	FACE_COUNTING_IN,
	FACE_FOUND_FIRST,
	FACE_FOUND,
	FACE_COUNTING_OUT
}faceFoundState;

class faceFinder : public ofBaseDraws{

	public:
		
		void setup(string haarFile, int resizeW, int resizeH, float numSecondsToSeeFace, float numSecondsToForget);
		void updatePixels( unsigned char * pix, int incommingW, int incommingH );
		void update();
		
		bool firstSawFace();
		void clearFirstSawFace();
		bool currentlySeeingFace();
		float faceConfidence();	
		
		void resetCounters();
				
		void draw(float x, float y, float w, float h);
		void draw(float x, float y);
		
		faceFoundState getState(){
			return faceState;
		}	
		
		string getStateAsString(){
			if( faceState == FACE_NONE )return string("FACE_NONE");
			if( faceState == FACE_COUNTING_IN )return string("FACE_COUNTING_IN");
			if( faceState == FACE_FOUND_FIRST )return string("FACE_FOUND_FIRST");
			if( faceState == FACE_FOUND )return string("FACE_FOUND");
			if( faceState == FACE_COUNTING_OUT )return string("FACE_COUNTING_OUT");
		}
		
		float getWidth(){
			return cvGrayResized.getWidth();
		}

		float getHeight(){
			return cvGrayResized.getHeight();
		}
						
		float confidenceAddAmnt;
		float confidenceFadeRate; 
		float confidenceGateValStart; //what val between 0.0 and 1.0 is considered 'on' - ie we are now seeing a face
		float confidenceGateValStop;  //what val between 0.0 and 1.0 is considered 'off' - ie we are not seeing a face anymore.

		ofxCvColorImage		cvColor;
		ofxCvGrayscaleImage cvGray;
		ofxCvGrayscaleImage cvGrayResized;

		bool bSeeFace;
		float confidence;
		float numSecSeeFaceF;
		float numSecForgetF;
		int numFaces;

		protected:
			float timeStartedLooking;
			float timeStartedForget;
			faceFoundState faceState;
			bool bSetup;
			ofxCvHaarFinder faceHaar;
};