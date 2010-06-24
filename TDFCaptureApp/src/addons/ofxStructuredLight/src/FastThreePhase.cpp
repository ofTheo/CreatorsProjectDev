#include "FastThreePhase.h"

FastThreePhase::FastThreePhase() {
}

void FastThreePhase::setup(int width, int height) {
	ThreePhaseDecoder::setup(width, height);

	threePhaseWrap.setup(width, height);
	partialQualityMap.setup(width, height);
	scanlineOffset.setup(width, height);

	phaseImage.allocate(width, height, OF_IMAGE_GRAYSCALE);
	qualityImage.allocate(width, height, OF_IMAGE_GRAYSCALE);
	offsetImage.allocate(width, height, OF_IMAGE_GRAYSCALE);

	phasePixels = phaseImage.getPixels();
	qualityPixels = qualityImage.getPixels();
	offsetPixels = offsetImage.getPixels();
}

void FastThreePhase::decode() {
	makeColor();

	threePhaseWrap.setThreshold((unsigned char) rangeThreshold);
	threePhaseWrap.makeWrappedPhase(phasePixels, qualityPixels,
		graySequence[0], graySequence[1], graySequence[2]);
	partialQualityMap.makeQualityMap(phasePixels, qualityPixels);
	scanlineOffset.makeOffset(phasePixels, qualityPixels, (char*) offsetPixels, phasePersistence);

	int n = width * height;

	for(int i = 0; i < n; i++)
		mask[i] = qualityPixels[i] != LABEL_UNWRAPPED;

	for(int i = 0; i < n; i++) {
		int cur = ((char*) offsetPixels)[i];
		phase[i] = (cur * 256) + phasePixels[i];
		phase[i] *= .002f; // this should be handled by skew/scale instead
	}

	makeDepth();
}
