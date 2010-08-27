#include "Particle.h"

float Particle::velocityDamping, Particle::velocityMax, Particle::forceScale, Particle::hardness;
float Particle::meanderForce, Particle::meanderJitter, Particle::meanderRange;
float Particle::angleJitter, Particle::angleRange;
bool Particle::showCollisions, Particle::showFaces, Particle::showCircles, Particle::animateFaces;
int Particle::birthTime, Particle::deathTime;

Particle::Particle(float lat, float lon, float age) :
		age(age),
		forceCount(0),
		dying(false),
		startedDying(0) {
	setPosition(lat, lon);
	noiseOffset = ofRandom(0, 1024);
	meanderDirection = ofRandom(-PI, PI);
}

void Particle::setFace(unsigned char id, string path) {
	face.setup(id, path);
}

Particle::~Particle() {
}

void Particle::kill() {
	if(!dying) {
		dying = true;
		startedDying = age;
		if(age < birthTime) // not yet finished being born
			startedDying -= ofMap(age, 0, birthTime, deathTime, 0);
	}
}

bool Particle::isDead() {
	return dying && age - startedDying > deathTime;
}

void Particle::clearForces() {
	forceCentroid.set(0, 0, 0);
	forceCount = 0;
}

void Particle::updateRelativeRadius() {
	if(dying)
		relativeRadius = 1 - ofClamp(age - startedDying, 0, deathTime) / deathTime;
	else
		relativeRadius = ofClamp(age, 0, birthTime) / birthTime;
	relativeRadius = relativeRadius * relativeRadius * (3 - 2 * relativeRadius); // smooth tween in
}

void Particle::updateAbsoluteRadius(float radiusScale) {
	radius = relativeRadius * radiusScale;
}

void Particle::addForce(const Particle* particle) {
	float distance = getDistance(particle);
	float minDistance = radius + particle->radius;
	if(distance < minDistance) {
		float w = 1 / powf(distance / minDistance, hardness);
		w *= -forceScale;
		ofxVec3f target = (xunit * (1 - w)) * position;
		target += (xunit * w) * particle->position;
		target.normalize();
		forceCentroid += target;
		forceCount++;
	}
}

void Particle::update() {
	ofxQuaternion force, bforce, mforce, dforce;

	// normalization is just for visualization
	forceCentroid /= forceCount; // find the spherical centroid
	bforce.makeRotate(getVectorPosition(), forceCentroid);
	bforce.slerp(relativeRadius, ofxQuaternion(), bforce); // reduces add-glitches
	force *= bforce;

	float curDirection = meanderDirection + ofSignedNoise(noiseOffset + meanderJitter * baseJitter * age) * PI * meanderRange;
	mforce.makeRotate(
		0, xunit,
		sinf(curDirection) * meanderForce, yunit,
		cosf(curDirection) * meanderForce, zunit);
	force *= mforce;

	dforce.slerp(-velocityDamping, ofxQuaternion(), velocity);
	force *= dforce;

	velocity *= force; // mass = 1
	velocity /= velocity.length(); // keep velocity stable

	float velocityMagnitude = acosf(xunit.dot(velocity * xunit));
	if(velocityMagnitude > velocityMax)
		velocity.slerp(velocityMax / velocityMagnitude, ofxQuaternion(), velocity);

	float faceAngle = ofSignedNoise(noiseOffset + angleJitter * baseJitter * age) * PI * angleRange;
	setAngle(faceAngle);

	// apply velocity to position
	position *= velocity;
	position /= position.length(); // keep position stable

	if(animateFaces) {
		face.next();
		
		// kill faces before they loop
		if(face.curImage > (face.dirSize - deathTime))
			kill();
	}

	age++;
}

void Particle::setAngle(float angle) {
	ofxVec2f latlon = convertToLatitutdeLongitude(position);
	position.makeRotate(
		angle, xunit,
		latlon.x, yunit,
		latlon.y, zunit);
}

void Particle::draw() {
	glPushMatrix();

	if(forceCount > 0 && showCollisions)
		glColor4f(1, 0, 0, 1);
	else
		glColor4f(1, 1, 1, 1);

	// use position to place circle
	float a, x, y, z;
	position.getRotate(a, x, y, z);
	glRotatef(ofRadToDeg(a), x, y, z);

	// place circles on edge of sphere
	glTranslatef(getDistanceFromCenter(), 0, 0);

	glRotatef(-90, 1, 0, 0); // rotate so y axis is "up"
	glRotatef(-90, 0, 1, 0); // make circles lay flat on surface

	if(showCircles) {
		if(City::showCities) {
			float* rgb = city->rgb;
			glColor4f(rgb[0], rgb[1], rgb[2], 1);
		}
		// cartesian radius is different than spherical radius
		ofCircle(0, 0, getCartesianRadius());
		// draw line from center of circle to (0, -1) i.e., up
		ofLine(0, 0, 0, -getCartesianRadius());
	}

	if(showFaces) {
		glPushMatrix();
		glColor4f(1, relativeRadius, 1, 1);
		glScalef(getCartesianRadius(), getCartesianRadius(), 1);
		face.draw();
		glPopMatrix();
	}

	glPopMatrix();
	
	ofxVec3f nvector = getVectorPosition();
	if(showCircles && forceCount > 0) {
		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(nvector.x, nvector.y, nvector.z);
		glVertex3f(forceCentroid.x, forceCentroid.y, forceCentroid.z);
		glEnd();
	}
}

