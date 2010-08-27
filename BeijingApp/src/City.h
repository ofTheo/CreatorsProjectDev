#pragma once

#include "SphereCircle.h"
#include "Particle.h"
#include "ofxDirList.h"

class City : public SphereCircle {
protected:
	bool expanding;
public:	
	static bool showCities;
	
	unsigned char color;
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
