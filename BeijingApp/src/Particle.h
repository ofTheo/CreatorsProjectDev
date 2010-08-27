#pragma once

#include "SphereCircle.h"
#include "Face.h"
#include "ofMain.h"

class Particle : public SphereCircle {
public:
	static float velocityDamping, velocityMax, forceScale, hardness;
	static float meanderForce, meanderJitter, meanderRange;
	static float angleJitter, angleRange;
	static bool showCollisions, showCircles, showFaces, animateFaces;
	static int birthTime, deathTime;

	static const float baseJitter = 1. / 60.;

	float relativeRadius;

	ofxQuaternion velocity;
	ofxVec3f forceCentroid;
	int forceCount;

	float meanderDirection;
	float noiseOffset;

	int age;
	bool dying;
	int startedDying;

	Face face;

	Particle(float lat = 0, float lon = 0, float age = 0);
	void setFace(unsigned char id, string path);
	~Particle();

	void kill();
	bool isDead();

	void updateRelativeRadius();
	void updateAbsoluteRadius(float radiusScale);
	void clearForces();
	void addForce(const Particle* particle);
	void update();

	void draw();

	void setAngle(float angle);
};
