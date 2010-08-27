#pragma once

#include "SphereCircle.h"
#include "Particle.h"
#include "ofxDirList.h"

class Particle;

class City : public SphereCircle {
protected:
	bool expanding;
public:	
	static bool showCities;
	
	unsigned char color;
	float rgb[3];
	ofxDirList dirList;
	int dirSize;
	string cityName;

	float latitude, longitude;
	int age;

	City();
	~City();
	void setup(unsigned char color, string path, string cityName);
	void setPosition(float latitude, float longitude);
	void setRadius(float radius);
	void setupParticle(Particle* particle);
	void draw();
};

inline void convertHsvToRgb(const float hsv[3], float rgb[3]) {
	if(hsv[2] == 0 || hsv[1] == 0) { // black and grays
		rgb[0] = rgb[1] = rgb[2] = hsv[2];
	} else {
		float h = hsv[0] * 6;
		int hCategory = (int) floorf(h);
		float hRemainder = h - hCategory;
		float pv = (1. - hsv[1]) * hsv[2];
		float qv = (1. - hsv[1] * hRemainder) * hsv[2];
		float tv = (1. - hsv[1] * (1. - hRemainder)) * hsv[2];
		switch(hCategory) {
			case 0: // r
				rgb[0] = hsv[2];
				rgb[1] = tv;
				rgb[2] = pv;
				break;
			case 1: // g
				rgb[0] = qv;
				rgb[1] = hsv[2];
				rgb[2] = pv;
				break;
			case 2:
				rgb[0] = pv;
				rgb[1] = hsv[2];
				rgb[2] = tv;
				break;
			case 3: // b
				rgb[0] = pv;
				rgb[1] = qv;
				rgb[2] = hsv[2];
				break;
			case 4:
				rgb[0] = tv;
				rgb[1] = pv;
				rgb[2] = hsv[2];
				break;
			case 5: // back to r
				rgb[0] = hsv[2];
				rgb[1] = pv;
				rgb[2] = qv;
				break;
		}
	}
}
