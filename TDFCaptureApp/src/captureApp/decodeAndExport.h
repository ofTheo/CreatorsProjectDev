/*
 *  decodeAndExport.h
 *  TDFCaptureApp
 *
 *  Created by theo on 20/06/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxStructuredLight.h"
#include "ofx3DUtils.h"

class decodeAndExport{
	
	public:
		decodeAndExport(){
			threePhase = NULL;
			width	= 0;
			height  = 0;
		}
	
		~decodeAndExport(){
			if( threePhase ){
				delete threePhase;
			}
		}
	
		void setupDecoder(int w, int h){
			if( width != w || height != h || threePhase == NULL ){
				if( threePhase ){	
					delete threePhase;
				}
				
				width	= w;
				height  = h;
				
				threePhase = new FastThreePhase();
				threePhase->setup(width, height);
				img.allocate(w, h, OF_IMAGE_COLOR_ALPHA);
			}
		}
		
		void setSettings(float gamma, float curDepthScale, float curDepthSkew, float curRangeThreshold, int curOrientation, bool curPhasePersistence, int curMaxHoleSize){
			if( threePhase ){
				threePhase->setGamma(gamma);
				threePhase->setDepthScale(curDepthScale);
				threePhase->setDepthSkew(curDepthSkew);
				threePhase->setRangeThreshold(curRangeThreshold);
				threePhase->setOrientation(curOrientation == 0 ? PHASE_HORIZONTAL : PHASE_VERTICAL);
				threePhase->setMaxHoleSize(curMaxHoleSize);
				
				//printf("set %f %f %f %f %i %i\n", gamma, curDepthScale, curDepthSkew, curRangeThreshold, curOrientation, curPhasePersistence);
				
				bool bChanged = false;
				if( curPhasePersistence != threePhase->phasePersistence ){
					bChanged = true;
				}
				threePhase->setPhasePersistence(curPhasePersistence);
				if( bChanged )threePhase->clearLastPhase();
			}
		}

		void decodeFrameAndFilter(unsigned char * pix, int index, int numChannels, float filterMin, float filterMax, float smoothAmnt, int smoothDist, bool smoothGaussian, int dilatePasses){
			if( threePhase ){
				threePhase->set(index%3, pix, numChannels == 3 ? 3 : 1);
				threePhase->decode();
				threePhase->filterRange(filterMin, filterMax);
				
				//printf("decodeFrameAndFilter %i %i %f %f %f %i\n", index,  numChannels,  filterMin,  filterMax,  smoothAmnt,  smoothDist);

				//TODO: optimize

				if( smoothAmnt > 0.0 || smoothGaussian){
					threePhase->filterDepth(smoothDist, smoothAmnt, smoothGaussian, dilatePasses); // <3
				}
				
				//img.setFromPixels(threePhase->getColorAndDepth(filterMin, filterMax), threePhase->getWidth(), threePhase->getHeight(), OF_IMAGE_COLOR_ALPHA);
			}
		}

		void exportFrameToTGA(string directoryPath,  int frameNo, float filterMin, float filterMax){
			threePhase->exportDepthAndTexture(directoryPath+ofToString(frameNo)+".tga", filterMin, filterMax);
			
			//img.saveImage(directoryPath+ofToString(frameNo)+".tga");
		}
					
		void exportFrameToPNG(string directoryPath, int frameNo, float filterMin, float filterMax){
			threePhase->exportDepthAndTexture(directoryPath+ofToString(frameNo)+".png", filterMin, filterMax);
		
			//img.saveImage(directoryPath+ofToString(frameNo)+".png");
		}

	//--------------------------------------------------------------
	void drawCurrentFrame(float x, float y, float w, float h){
		ofEnableAlphaBlending();
		ofSetColor(255, 0, 0, 255);
		ofRect(x, y, w, h);
		ofSetColor(255, 255, 255, 255);
		img.draw(x, y, w, h);
	}
		
	//--------------------------------------------------------------
	void drawCloud() {

		if( threePhase == NULL ){
			return;
		}

		ofTranslate(-threePhase->getWidth() / 2, -threePhase->getHeight() / 2);
			
		ofSetColor(255, 255, 255);

		bool* mask		= threePhase->getMask();
		float* depth	= threePhase->getDepth();
		byte* color		= threePhase->getColor();

		int srcWidth	= threePhase->getWidth();
		int srcHeight	= threePhase->getHeight();

		glEnable(GL_POINT_SIZE);
		glPointSize(1.5);
		glBegin(GL_POINTS);
		int i = 0;
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				if (!mask[i]) {
					glColor3ubv(&color[i * 3]);
					glVertex3f(x, y, depth[i]);
				}
				i++;
			}
		}
		glEnd();
		
	}
			
			
	ofImage img;

	int width;
	int height;
	FastThreePhase* threePhase; // whoops, this is very important
};