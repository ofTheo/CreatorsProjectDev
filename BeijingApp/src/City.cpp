#include "City.h"

bool City::showCities;

City::City() :
	radius(0),
	latitude(0),
	longitude(0),
	color(0),
	expanding(false) {
}

City::~City() {
}

void City::setup(unsigned char color, string path, string cityName) {
	this->color = color;
	dirSize = dirList.listDir(path);
	this->cityName = cityName;
}

void City::setPosition(float latitude, float longitude) {
	this->latitude = latitude;
	this->longitude = longitude;
}

void City::setupParticle(Particle* particle) {
	// associate it with a random directory
	int randomPosition = (int) ofRandom(0, dirSize);
	string randomPath = dirList.getPath(randomPosition);
	particle->setFace(color, randomPath);
}

void City::setRadius(float radius) {
	expanding = radius > this->radius;
	this->radius = radius;
}

bool City::getExpanding() {
	return expanding;
}

void City::draw() {
	glPushMatrix();
	
	// use orientation to place circle
	ofxQuaternion orientation;
	orientation.makeRotate(
												 0, xunit,
												 latitude, yunit,
												 longitude, zunit);
	float a, x, y, z;
	orientation.getRotate(a, x, y, z);
	glRotatef(ofRadToDeg(a), x, y, z);
	
	if(showCities) {
		ofPushMatrix();
		glColor4f(1, 1, 1, 1);
		glTranslatef(1, 0, 0);
		ofDrawBitmapString(cityName, 0, 0);
		ofPopMatrix();
	
		// place circles on edge of sphere
		glTranslatef(getDistanceFromCenter(), 0, 0);
		
		glRotatef(-90, 1, 0, 0); // rotate so y axis is "up"
		glRotatef(-90, 0, 1, 0); // make circles lay flat on surface
	
		// cartesian radius is different than spherical radius
		glColor4f(0, 1, 0, 1);
		ofCircle(0, 0, getCartesianRadius());
	}
	
	glPopMatrix();
}

float City::getDistanceFromCenter() {
	return cosf(radius);
}

float City::getCartesianRadius() {
	return sinf(radius);
}