#pragma once

#include "ofMain.h"
#include "ofxVectorMath.h"
#include "Face.h"

const ofxVec3f
	xunit(1, 0, 0),
	yunit(0, 1, 0),
	zunit(0, 0, 1);

inline ofxVec2f convertToLatitutdeLongitude(ofxQuaternion quaternion) {
	ofxVec3f nvec = xunit * quaternion;
	ofxVec2f latlon;
	latlon.x = -atan2(nvec.z, sqrtf(nvec.x * nvec.x + nvec.y * nvec.y));
	latlon.y = atan2(nvec.y, nvec.x);
	return latlon;
}

class Particle {
public:
	static float velocityDamping, velocityMax, forceScale, hardness;
	static float meanderForce, meanderJitter, meanderRange;
	static float angleJitter, angleRange;
	static bool showCollisions, showCircles, showFaces, animateFaces;
	static int birthTime, deathTime;

	static const float baseJitter = 1. / 60.;

	float relativeRadius, absoluteRadius;

	ofxQuaternion orientation, velocity;
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
	void addForce(const Particle& particle);
	void update();

	void draw();

	float getDistanceFromCenter() const;
	float getCartesianRadius() const;
	void setAngle(float angle);
	ofxVec3f getPosition() const;
	float getDistance(const Particle& particle) const;
};
