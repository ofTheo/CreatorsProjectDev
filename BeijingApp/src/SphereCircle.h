#pragma once

#include "ofxVectorMath.h"

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

class SphereCircle {
protected:
	ofxQuaternion position;
	float radius;
public:
	void setPosition(float latitude, float longitude) {		
		position.makeRotate(0, xunit,
												latitude, yunit,
												longitude, zunit);
	}
		float getDistanceFromCenter() const {
		return cosf(radius);
	}
	float getCartesianRadius() const {
		return sinf(radius);
	}
	ofxVec3f getVectorPosition() const {
		return position * xunit;
	}
	float getDistance(const SphereCircle* sphereCircle) const {
		ofxVec3f na = getVectorPosition();
		ofxVec3f nb = sphereCircle->getVectorPosition();
		return acosf(na.dot(nb));
	}
	bool contains(const SphereCircle* sphereCircle) const {
		return getDistance(sphereCircle) < radius;
	}
};