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
	
	ofImage phaseImage, qualityImage, offsetImage;
	
private:
	ThreePhaseWrap threePhaseWrap;
	PartialQualityMap partialQualityMap;
	ScanlineOffset scanlineOffset;

	unsigned char *qualityPixels, *offsetPixels, *phasePixels;
};
