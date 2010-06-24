#include "FastThreePhase.h"

FastThreePhase::FastThreePhase() {
}

void FastThreePhase::setup(int width, int height) {
	ThreePhaseDecoder::setup(width, height);
	
	threePhaseWrap.setup(width, height);
	partialQualityMap.setup(width, height);
	scanlineOffset.setup(width, height);

	phaseImage.setUseTexture(false);
	qualityImage.setUseTexture(false);
	offsetImage.setUseTexture(false);
	
	phaseImage.allocate(width, height, OF_IMAGE_GRAYSCALE);
	qualityImage.allocate(width, height, OF_IMAGE_GRAYSCALE);
	offsetImage.allocate(width, height, OF_IMAGE_GRAYSCALE);

	phasePixels = phaseImage.getPixels();
	qualityPixels = qualityImage.getPixels();
	offsetPixels = offsetImage.getPixels();
}

void FastThreePhase::decode() {
	makeColor();
	
	int n = width * height;

	threePhaseWrap.setThreshold((unsigned char) rangeThreshold);
	threePhaseWrap.makeWrappedPhase(phasePixels, qualityPixels,
		graySequence[0], graySequence[1], graySequence[2]);
	
	// this is used by getStart() to determine the center
	for(int i = 0; i < n; i++)
		ready[i] = qualityPixels[i] != LABEL_BACKGROUND;
	
	partialQualityMap.makeQualityMap(phasePixels, qualityPixels);
	
	scanlineOffset.makeOffset(phasePixels, qualityPixels, (char*) offsetPixels, getStart(), phasePersistence);

	for(int i = 0; i < n; i++)
		mask[i] = qualityPixels[i] != LABEL_UNWRAPPED;

	for(int i = 0; i < n; i++) {
		int cur = ((char*) offsetPixels)[i];
		phase[i] = (cur * 256) + phasePixels[i];
		phase[i] *= .002f; // this should be handled by skew/scale instead
	}

	makeDepth();
}