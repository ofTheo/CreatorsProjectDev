/*
 *  histogramGuiElement.h
 *  emptyExample
 *
 *  Created by theo on 6/24/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofMain.h"

class histogramGuiElement : public ofBaseDraws{

	public:
	
	histogramGuiElement(){
		histogram.assign(256, 0.0);
		width = 255;
		height = 100;
	}
	
	float getWidth(){
		return width;
	}
	
	float getHeight(){
		return height;
	}

	void setPixels(float * pix, int width, int height){
		histogram.clear();
		histogram.assign(256, 0.0);
		
		int n = width * height; 
		int val = 0;
		float highestAmnt = 0.0;
		for(int i = 0; i < n; i++){
			
			val	= (int)pix[i];
			if( val < 1 || val >= 256 ) continue;
			
			histogram[val] += 1.0;
			
			if( histogram[val] > highestAmnt ){
				highestAmnt = histogram[val];
			}
		}
		
		float divideBy = 1.0/(1.0+highestAmnt);
		
		for(int i = 0; i < 256; i++){
			histogram[i] *= divideBy;
		}
	}
	
	void setPixels(unsigned char * pix, int width, int height){
		
		histogram.clear();
		histogram.assign(256, 0.0);
		
		int n = width * height; 
		
		float highestAmnt = 0.0;
		for(int i = 0; i < n; i++){
			if( pix[i] == 0 )continue;
			
			histogram[pix[i]] += 1.0;
			
			if( histogram[pix[i]] > highestAmnt ){
				highestAmnt = histogram[pix[i]];
			}
		}
		
		float divideBy = 1.0/(1.0+highestAmnt);
		
		for(int i = 0; i < 256; i++){
			histogram[i] *= divideBy;
		}
		
	}

	void draw(float x, float y){
		draw(x, y, 255, 100);
	}
	
	void draw(float x, float y, float w, float h){
		ofPushStyle();
		
		ofSetColor(255, 255, 255);
		ofNoFill();
		ofRect(x, y, w, h);
		
		for(int i = 0; i < histogram.size(); i++){
			float xx = ofMap(i, 0, 256, x, x+w, true);
			ofLine(xx, y+h, xx, y+h - (histogram[i]*h) );
		}
				   
		ofPopStyle();
	}

	bool bColor;
	float width, height;
	vector <float> histogram;	
};