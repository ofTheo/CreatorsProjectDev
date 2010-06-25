#pragma once

#include "DepthDecoder.h"
#include "FastBlur.h"
#include "PhaseConstants.h"
#include "ofxOpenCv.h"

//#define LINEARIZE_GAMMA

class PhaseDecoder : public DepthDecoder {
public:
	PhaseDecoder();
	virtual ~PhaseDecoder();
	void setGamma(float gamma);
	void setDepthScale(float depthScale);
	void setDepthSkew(float depthSkew);
	void setOrientation(phaseOrientation orientation);
	void setMaxPasses(int maxPasses);
	void setMinRemaining(float minRemaining);
	void setPhasePersistence(bool phasePersistence);
	void clearLastPhase();
	void set(int i, byte* image, int channels = 3);
	void makeDepth();
	void decode();
	void filterDepth(int yDist, float yAmt, bool useGaussian = false, int dilatePasses = 2);
		
	float* getPhase();
	float* getWrappedPhase();
	byte* getColor();
	byte** getGraySequence();
	int* getBlur();
	void exportCloud(string filename);
	void exportMesh(string filename);
	void exportTexture(string filename);
	void exportDepthAndTexture(string filename, float min, float max);
	float* unwrapOrder;
//protected:
	byte** colorSequence;
	byte** graySequence;
	byte* reflectivity;
	float* phase;
	float* wrappedPhase;
	bool* ready;
	byte* color;
	byte * colorDepth;
	int sequenceSize;
	float gamma;
	int maxPasses;
	float minRemaining;
	float depthScale, depthSkew;
	phaseOrientation orientation;
	FastBlur blur;
	bool phasePersistence;
	float* lastPhase;
	float* range;

	#ifdef LINEARIZE_GAMMA
	int gammaHistogram[256];
	#endif

	/// this is where phase unwrapping begins, and unwrapped phase will be zero
	int startInd;

	void setup(int width, int height, int sequenceSize);
	virtual void makePhase() = 0;
	virtual void unwrapPhase() = 0;
	virtual void makeColor() = 0;
	virtual int getStart();
	float getRemaining();

	int offsetBinSize;
	int offsetBinCount;
};
