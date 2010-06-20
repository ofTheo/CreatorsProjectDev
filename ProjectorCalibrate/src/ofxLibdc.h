#pragma once

#include "ofMain.h"
#include "dc1394.h"

class ofxLibdc {
public:
	ofxLibdc();
	int getCameraCount();
	void setup(int cameraNumber = 0);
	~ofxLibdc();
	
	void setBrightness(unsigned int brightness);
	void setGamma(unsigned int gamma);
	void setGain(unsigned int gain);
	void setExposure(unsigned int exposure);
	void setShutter(unsigned int shutter);
	void setFeature(dc1394feature_t feature, unsigned int value);
	
	void getBrightnessRange(unsigned int* min, unsigned int* max);
	void getGammaRange(unsigned int* min, unsigned int* max);
	void getGainRange(unsigned int* min, unsigned int* max);
	void getExposureRange(unsigned int* min, unsigned int* max);
	void getShutterRange(unsigned int* min, unsigned int* max);
	void getFeatureRange(dc1394feature_t feature, unsigned int* min, unsigned int* max);
	
	void setBrightnessNorm(float gamma);
	void setGammaNorm(float gamma);
	void setGainNorm(float gain);
	void setExposureNorm(float exposure);
	void setShutterNorm(float shutter);
	void setFeatureNorm(dc1394feature_t feature, float value);
	
	float getShutterAbs();
	float getFeatureAbs(dc1394feature_t feature);
	
	void getOneShot(ofImage& img);
		
	unsigned int getWidth();
	unsigned int getHeight();
	
private:
	
	static dc1394_t* libdcContext;
	static int libdcCameras;
	static void startLibdcContext();
	static void stopLibdcContext();
	
	void setTransmit(bool transmit);
	void flush();
	
	dc1394camera_t* camera;
	unsigned int width, height;
};