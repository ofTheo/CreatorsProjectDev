#pragma once

#include "Particle.h"
#include "ofxDirList.h"

class City {
public:	
	unsigned char id;
	ofxDirList dirList;
	int dirSize;

	float latitude, longitude;
	float radius;

	City();
	~City();
	void setup(unsigned char id, string path);
	void setPosition(float latitude, float longitude);
	void setupParticle(Particle* particle);
};
