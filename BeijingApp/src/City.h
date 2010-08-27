#pragma once

#include "Particle.h"
#include "ofxDirList.h"

class City {
private:
	bool expanding;
	float radius;
public:	
	static bool showCities;
	
	unsigned char color;
	ofxDirList dirList;
	int dirSize;
	string cityName;

	float latitude, longitude;

	City();
	~City();
	void setup(unsigned char color, string path, string cityName);
	void setPosition(float latitude, float longitude);
	void setRadius(float radius);
	void setupParticle(Particle* particle);
	bool getExpanding();
	void draw();
	
	float getDistanceFromCenter();
	float getCartesianRadius();
};
