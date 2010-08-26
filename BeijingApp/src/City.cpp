#include "City.h"

City::City() {
}

City::~City() {
	for(int i = 0; i < particles.size(); i++)
		delete particles[i];
}

void City::setup(unsigned char id, string path) {
	this->id = id;
	dirSize = dirList.listDir(path);
}

void City::setPosition(float latitude, float longitude) {
	this->latitude = latitude;
	this->longitude = longitude;
}

void City::birth() {
	// add a new particle at a random position
	particles.push_back(new Particle(
		ofDegToRad(ofRandom(-90, 90)),
		ofDegToRad(ofRandom(-180, 180))));
	
	// associate it with a random directory
	// (you have to pick random elements like this or you lose the last item)
	int randomPosition = (int) ofClamp(ofRandom(0, dirSize), 0, dirSize - 1);
	string randomPath = dirList.getPath(randomPosition);
	particles.back()->setFace(id, randomPath);
}

void City::kill() {
	// kill a particle that isn't already dying, starting at the end
	for(int i = size() - 1; i >= 0; i--)
		if(!particles[i]->dying) {
			particles[i]->kill();
			break;
		}
}

int City::size() const {
	return particles.size();
}

int City::livingSize() const {
	int n = 0;
	for(int i = 0; i < size(); i++)
		if(!particles[i]->dying)
			n++;
	return n;
}

void City::removeDead() {
	for(int i = size() - 1; i >= 0; i--) {
		if(particles[i]->isDead()) {
			delete particles[i];
			particles.erase(particles.begin() + i);
		}
	}
}

float City::sumRadii() {
	float sum = 0;
	for(int i = 0; i < size(); i++) {
		particles[i]->updateRelativeRadius();
		sum += particles[i]->relativeRadius;
	}
	return sum;
}

void City::updateAbsoluteRadius(float radiusScale) {
	for(int i = 0; i < size(); i++)
		particles[i]->updateAbsoluteRadius(radiusScale);
}

void City::draw() {
	for(int i = 0; i < size(); i++)
		particles[i]->draw();
}
