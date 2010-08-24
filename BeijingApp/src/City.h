#pragma once

#include "Particle.h"
#include "ofxDirList.h"

class City {
public:
	static float birthRadius;
	
	unsigned char id;
	ofxDirList dirList;
	int dirSize;

	vector<Particle*> particles;
	float latitude, longitude;

	City();
	~City();
	void setup(unsigned char id, string path);
	void setPosition(float latitude, float longitude);
	void birth();
	void kill();
	int size() const;
	int livingSize() const;
	void removeDead();
	float sumRadii();
	void updateAbsoluteRadius(float radiusScale);
	void draw();
	void moveTowards(int target);
};
