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
	
	if(maxHoleSize > 0)
		fillHoles();
}

void FastThreePhase::setMaxHoleSize(int maxHoleSize) {
	this->maxHoleSize = maxHoleSize;
}

void FastThreePhase::fillHoles() {
	// horizontal pass
	int i = 0;
	for(int y = 0; y < height; y++) {
		bool hasStart = false;
		bool integrating = false;
		int start;
		for(int x = 0; x < width; x++) {
			if(mask[i]) {
				if(hasStart)
					integrating = true;
			} else {
				if(integrating) {
					if(i - start < maxHoleSize) {
						float startValue = depth[start];
						float outputRange = depth[i] - startValue;
						int inputRange = i - start;
						for(int j = start + 1; j < i; j++) {
							depth[j] = ((j - start) * outputRange) / inputRange + startValue;
							mask[j] = false;
						}
					}
					integrating = false;
				}
				hasStart = true;
				start = i;
			}
			i++;
		}
	}
	// vertical pass
	int yMaxHoleSize = maxHoleSize * width;
	for(int x = 0; x < width; x++) {
		bool hasStart = false;
		bool integrating = false;
		int start;
		i = x;
		for(int y = 0; y < height; y++) {
			if(mask[i]) {
				if(hasStart)
					integrating = true;
			} else {
				if(integrating) {
					if(i - start < yMaxHoleSize) {
						float startValue = depth[start];
						float outputRange = depth[i] - startValue;
						int inputRange = i - start;
						for(int j = start + width; j < i; j += width) {
							depth[j] = ((j - start) * outputRange) / inputRange + startValue;
							mask[j] = false;
						}
					}
					integrating = false;
				}
				hasStart = true;
				start = i;
			}
			i += width;
		}
	}
}