#include "City.h"

City::City() {
}

City::~City() {
}

void City::setup(unsigned char id, string path) {
	this->id = id;
	dirSize = dirList.listDir(path);
}

void City::setPosition(float latitude, float longitude) {
	this->latitude = latitude;
	this->longitude = longitude;
}

void City::setupParticle(Particle* particle) {
	// associate it with a random directory
	int randomPosition = (int) ofRandom(0, dirSize);
	string randomPath = dirList.getPath(randomPosition);
	particle->setFace(id, randomPath);
}