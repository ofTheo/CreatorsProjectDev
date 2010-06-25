#pragma once

#include "ofxStructuredLight.h"
#include "ThreePhaseWrap.h"
#include "PartialQualityMap.h"
#include "ScanlineOffset.h"

class FastThreePhase : public ThreePhaseDecoder {
public:
	FastThreePhase();
	void setup(int width, int height);
	void decode();
	
	void setMaxHoleSize(int maxHoleSize);
	
	ofImage phaseImage, qualityImage, offsetImage;
	unsigned char *qualityPixels, *offsetPixels, *phasePixels;
	
	ThreePhaseWrap threePhaseWrap;
	PartialQualityMap partialQualityMap;
	ScanlineOffset scanlineOffset;

protected:	
	int maxHoleSize;
	
	void fillHoles();
};
