#include "City.h"

bool City::showCities;

City::City() :
	latitude(0),
	longitude(0),
	color(0),
	expanding(false),
	age(0) {
}

City::~City() {
}

void City::setup(unsigned char color, string path, string cityName) {
	this->color = color;
	const float hsv[] = {color / 255., 1., 1.};
	convertHsvToRgb(hsv, rgb);
	dirSize = dirList.listDir(path);
	this->cityName = cityName;
}

void City::setPosition(float latitude, float longitude) {
	this->latitude = latitude;
	this->longitude = longitude;
	SphereCircle::setPosition(latitude, longitude);
}

void City::setupParticle(Particle* particle) {
	// associate it with a random directory
	int randomPosition = (int) ofRandom(0, dirSize);
	string randomPath = dirList.getPath(randomPosition);
	particle->setFace(color, randomPath);
	particle->city = this;
}

void City::setRadius(float radius) {
	bool curExpanding = radius > this->radius;
	if(curExpanding != expanding)
		age = 0;
	else
		age++;
	expanding = curExpanding;
	this->radius = radius;
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
		glColor4f(rgb[0], rgb[1], rgb[2], 1);
		glTranslatef(1, 0, 0);
		ofDrawBitmapString(cityName, 0, 0);
		ofPopMatrix();
	
		// place circles on edge of sphere
		glTranslatef(getDistanceFromCenter(), 0, 0);
		
		glRotatef(-90, 1, 0, 0); // rotate so y axis is "up"
		glRotatef(-90, 0, 1, 0); // make circles lay flat on surface
	
		// cartesian radius is different than spherical radius
		glColor4f(expanding ? 1 : 0, 1, 0, 1);
		ofCircle(0, 0, getCartesianRadius());
	}
	
	glPopMatrix();
}