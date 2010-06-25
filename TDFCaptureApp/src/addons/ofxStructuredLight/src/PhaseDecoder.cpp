#include "PhaseDecoder.h"

PhaseDecoder::PhaseDecoder() :
		sequenceSize(0),
		gamma(1.0),
		colorSequence(NULL),
		graySequence(NULL),
		reflectivity(NULL),
		phase(NULL),
		ready(NULL),
		depthScale(1),
		depthSkew(1),
		maxPasses(1),
		minRemaining(0),
		color(NULL),
		colorDepth(NULL),
		orientation(PHASE_VERTICAL),
		phasePersistence(false),
		lastPhase(false),
		range(NULL),
		unwrapOrder(NULL),
		offsetBinSize(32),
		offsetBinCount(64) {
}

// This code should migrate to DepthDecoder, as it will
// be used for allocating a gray coded image sequence also.
void PhaseDecoder::setup(int width, int height, int sequenceSize) {
	this->width = width;
	this->height = height;
	this->sequenceSize = sequenceSize;

	colorSequence = new byte*[sequenceSize];
	graySequence = new byte*[sequenceSize];
	int n = width * height;
	for (int i = 0; i < sequenceSize; i++) {
		colorSequence[i] = new byte[n * 3];
		graySequence[i] = new byte[n];
	}
	reflectivity = new byte[n];
	phase = new float[n];
	wrappedPhase = new float[n];
	mask = new bool[n];
	ready = new bool[n];
	depth = new float[n];
	depthCopy = new float[n];
	color = new byte[n * 3];
	colorDepth = new byte[n*4];

	blur.setup(width, height);

	lastPhase = new float[n];
}

PhaseDecoder::~PhaseDecoder() {
	if (colorSequence != NULL) {
		for (int i = 0; i < sequenceSize; i++) {
			delete [] colorSequence[i];
			delete [] graySequence[i];
		}
		delete [] reflectivity;
		delete [] colorSequence;
		delete [] graySequence;
		delete [] phase;
		delete [] ready;
		delete [] color;
		delete [] colorDepth;
		delete [] lastPhase;
		delete [] wrappedPhase;
	}
}

void PhaseDecoder::setMaxPasses(int maxPasses) {
	this->maxPasses = maxPasses;
}

void PhaseDecoder::setMinRemaining(float minRemaining) {
	this->minRemaining = minRemaining;
}

void PhaseDecoder::set(int position, byte* image, int channels) {
	byte* curColor = colorSequence[position];
	byte* curGray  = graySequence[position];

	int n = width * height;
	int i = 0;
	int j = 0;
	if (channels == 3) {
		memcpy(curColor, image, n * 3);
		while (j < n) {
			float sum =
				(float) curColor[i++] +
				(float) curColor[i++] +
				(float) curColor[i++];
			sum /= 3;
			curGray[j++] = (byte) sum;
		}

		#ifdef LINEARIZE_GAMMA
		memset(gammaHistogram, 0, 256 * sizeof(int));
		for(i = 0; i < n; i++)
			gammaHistogram[curGray[i]]++;
		for(i = 1; i < 256; i++)
			gammaHistogram[i] += gammaHistogram[i - 1];
		for(i = 0; i < 256; i++)
			gammaHistogram[i] = (gammaHistogram[i] * 256) / n;
		for(i = 0; i < n; i++)
			curGray[i] = gammaHistogram[curGray[i]];
		#endif
	} else if(channels == 1) {
		while(i < n) {
			curColor[j++] = image[i];
			curColor[j++] = image[i];
			curColor[j++] = image[i];
			curGray[i] = image[i];
			i++;
		}
	}
}

void PhaseDecoder::decode() {
	makeColor();
	makePhase();
	memcpy( wrappedPhase, phase, sizeof(float) * width * height);
	for (int pass = 0; pass < maxPasses; pass++) {
		unwrapPhase();
		if (minRemaining != 0 && getRemaining() < minRemaining)
			break;
	}
	if (phasePersistence)
		memcpy(lastPhase, phase, sizeof(float) * width * height);
	makeDepth();
}

float* PhaseDecoder::getPhase() {
	return phase;
}

float* PhaseDecoder::getWrappedPhase() {
	return wrappedPhase;
}

void PhaseDecoder::setGamma(float gamma) {
	this->gamma = gamma;
}

void PhaseDecoder::setDepthScale(float depthScale) {
	this->depthScale = depthScale;
}

void PhaseDecoder::setDepthSkew(float depthSkew) {
	this->depthSkew = depthSkew;
}

void PhaseDecoder::setOrientation(phaseOrientation orientation) {
	this->orientation = orientation;
}

void PhaseDecoder::setPhasePersistence(bool phasePersistence) {
	this->phasePersistence = phasePersistence;
}

void PhaseDecoder::clearLastPhase() {
	memset(lastPhase, 0, sizeof(float) * width * height);
}

void PhaseDecoder::makeDepth() {
	int n = width * height;
	// this is where the "raw" depth is made
	if (orientation == PHASE_VERTICAL) {
		float planeZero = (float) (startInd % width) / width;
		for (int i = 0; i < n; i++) {
			if (!mask[i]) {
				float x = (float) (i % width);
				float planephase = ((x / width) - planeZero) * depthSkew;
				depth[i] = (phase[i] - planephase) * depthScale;
			} else {
				depth[i] = 0;
			}
		}
	} else if (orientation == PHASE_HORIZONTAL) {
		// this should be a bit faster
		int i = 0;
		for(int y = 0; y < height; y++) {
			float planephase = (y * depthSkew) / height;
			for(int x = 0; x < width; x++) {
				if(!mask[i])
					depth[i] = (phase[i] - planephase) * depthScale;
				else
					depth[i] = 0;
				i++;
			}
		}
	}
	
	// do an initial pass to take the mean of everything
	float depthSum = 0;
	int depthCount = 0;
	for(int i = 0; i < n; i++)
		if(!mask[i]) {
			depthSum += depth[i]; 
			depthCount++;
		}

	// use that mean as an offset for the binning
	// and quantize it to bin counts
	float averageOffset = depthSum / depthCount;
	averageOffset = (int) (averageOffset / offsetBinSize);
	averageOffset *= offsetBinSize;

	// do a second pass where we bin all the points
	float* offsetBins = new float[offsetBinCount];
	float* offsetBinDepths = new float[offsetBinCount];
	memset(offsetBins, 0, offsetBinCount * sizeof(int));
	float offsetBinOffset =
		(offsetBinCount * offsetBinSize) / 2 +
		(offsetBinSize / 2) +
		-averageOffset;
	for(int i = 0; i < n; i++) {
		if(!mask[i]) {
			int cur = (int) ((depth[i] + offsetBinOffset) / offsetBinSize);
			if(cur > 0 && cur < offsetBinCount) {
				offsetBins[cur]++;
				offsetBinDepths[cur] += depth[i];
			}
		}
	}
	
	// smooth out the bins, at golan's suggestion
	CvMat data = cvMat(1, offsetBinCount, CV_32F, offsetBins);
	cvSmooth(&data, &data, CV_GAUSSIAN, 1, 5);
	
	// determine which of these bins is the biggest
	// ignore the furthest l/r bins
	int biggestBin = 1;
	for(int i = 1; i < offsetBinCount - 1; i++)
		if(offsetBins[i] > offsetBins[biggestBin])
			biggestBin = i;

	// use the average of the depths in that bin as an offset
	// float offset = (biggestBin * offsetBinSize) - offsetBinOffset;
	
	// do a weighted average of the three bins around the center
	float weight = .3;
	float offset =
		(biggestBin - 1) * offsetBins[biggestBin - 1] * weight +
		biggestBin * offsetBins[biggestBin] +
		(biggestBin + 1) * offsetBins[biggestBin + 1] * weight;
	offset /=
		offsetBins[biggestBin - 1] * weight + 
		offsetBins[biggestBin] +
		offsetBins[biggestBin + 1] * weight;
	offset = (offset * offsetBinSize) - offsetBinOffset;
	
	for(int i = 0; i < n; i++) {
		if(mask[i])
			depth[i] = 0;
		else
			depth[i] -= offset;
	}
	
	delete [] offsetBins;
	delete [] offsetBinDepths;
	
	// if all that isn't enough, this can be revisited with temporal
	// matching of depth histograms using cvCompareHist, which will
	// return a single value describing the offset given two depth
	// histograms
}

void PhaseDecoder::filterDepth(int yDist, float yAmt, bool useGaussian, int dilatePasses){
	if(useGaussian) {
		CvMat data = cvMat(height, width, CV_32F, depth);
		cvSmooth(&data, &data, CV_GAUSSIAN, 5, 5);
		
		if(dilatePasses > 0) {
			IplImage* dmask = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 1);
			dmask->origin = IPL_ORIGIN_TL;
			dmask->widthStep = width;
			dmask->imageData = (char*) mask;
			cvDilate(dmask, dmask, NULL, dilatePasses);
			cvReleaseImageHeader(&dmask);
		}
	}
	
	// theo - wip - messy and prob slow for now. 
	if(yAmt > 0) {
		memcpy(depthCopy, depth, sizeof(float) * width * height);
		int k = 0;
		for(int y = 0; y < height; y++){
		
			int minY = -yDist;
			if( y + minY < 0 ) minY = -y;
			int maxY = yDist;
			if( y + maxY >= height ) maxY = (height-1)-y;
			
			int num = maxY - minY;
			if( num == 0 ){
				k+= width;
				continue;
			}
			
			for(int x = 0; x < width; x++){
			
				depth[k] *= (1.0-yAmt);
				float amnt = 0.0;
				int numReal = 0;
				for(int j = minY; j <= maxY; j++){
					if( j+y < 0 || j + y == height ){
						printf("FOOOOOOOOOOOOL\n");
						assert(0);
					}
					if( !mask[k + j*width] ){
						amnt +=  depthCopy[k + j*width];
						numReal++;
					}
				}
				
				amnt /= (float)(numReal+1);
				depth[k] += yAmt * amnt;
				
				k++;
			}
		}
	}
}

byte* PhaseDecoder::getColor() {
	return color;
}

byte** PhaseDecoder::getGraySequence() {
	return graySequence;
}

float PhaseDecoder::getRemaining() {
	int n = width * height;
	int readySum = 0;
	int maskSum = 0;
	for (int i = 0; i < n; i++) {
		readySum += (unsigned char) ready[i];
		maskSum += (unsigned char) mask[i];
	}
	return (float) readySum / (float) (n - maskSum);
}

int PhaseDecoder::getStart() {
	blur.blur((unsigned char*) ready, ready, 64);
	int* sum = blur.getSum();
	int max = 0;
	int n = width * height;
	for (int i = 0; i < n; i++)
		if (sum[i] > sum[max])
			max = i;
	return max;
}

int* PhaseDecoder::getBlur() {
	return blur.getSum();
}

void PhaseDecoder::exportCloud(string filename) {
	DepthExporter::exportCloud(filename, width, height, mask, depth, color);
}

void PhaseDecoder::exportMesh(string filename) {
	DepthExporter::exportMesh(filename, width, height, mask, depth, color);
}

void PhaseDecoder::exportTexture(string filename) {
	DepthExporter::exportTexture(filename, width, height, mask, color);
}

void PhaseDecoder::exportDepthAndTexture(string filename, float min, float max) {
	DepthExporter::exportDepthAndTexture(filename, width, height, mask, depth, min, max, color);
}
