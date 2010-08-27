#pragma once

#include "City.h"

class ParticleSystem {
public:
	static float sphereCoverage;
	static bool showAxes;

	float radius;

	City newYork, london, saoPaolo, seoul, beijing;
	vector<City*> cities;
	vector<Particle*> particles;

	ParticleSystem();
	void setup(int radius);
	~ParticleSystem();
	void drawAxes() const;
	void update();
	void draw();
	int size() const;
	int livingSize() const;
	void moveTowards(int target);
	void birth();
	void kill();
};	
