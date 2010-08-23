#pragma once

#include "Particle.h"
#include "ofxDirList.h"

// should use pointers to particles instead
// so it doesn't hurt so much when things are added/removed

class City {
public:
	unsigned char id;
	ofxDirList dirList;
	int dirSize;

	vector<Particle*> particles;

	City() {
	}
	~City() {
		for(int i = 0; i < particles.size(); i++)
			delete particles[i];
	}
	void setup(unsigned char id, string path) {
		this->id = id;
		dirSize = dirList.listDir(path);
	}
	void birth() {
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
	void kill() {
		// kill a particle that isn't already dying, starting at the end
		for(int i = size() - 1; i >= 0; i--)
			if(!particles[i]->dying) {
				particles[i]->kill();
				break;
			}
	}
	int size() const {
		return particles.size();
	}
	int livingSize() const {
		int n = 0;
		for(int i = 0; i < size(); i++)
			if(!particles[i]->dying)
				n++;
		return n;
	}
	void removeDead() {
		for(int i = size() - 1; i >= 0; i--) {
			if(particles[i]->isDead()) {
				delete particles[i];
				particles.erase(particles.begin() + i);
			}
		}
	}
	float sumRadii() {
		float sum = 0;
		for(int i = 0; i < size(); i++) {
			particles[i]->updateRelativeRadius();
			sum += particles[i]->relativeRadius;
		}
		return sum;
	}
	void updateAbsoluteRadius(float radiusScale) {
		for(int i = 0; i < size(); i++)
			particles[i]->updateAbsoluteRadius(radiusScale);
	}
	void draw() {
		for(int i = 0; i < size(); i++)
			particles[i]->draw();
	}
	void moveTowards(int target) {
		while(livingSize() != target) {
			// take one step towards the target number
			if(livingSize() < target)
				birth();
			else if(livingSize() > target)
				kill();
		}
	}
};
