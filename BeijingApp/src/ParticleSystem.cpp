#include "ParticleSystem.h"

float ParticleSystem::sphereCoverage;
bool ParticleSystem::showAxes;

ParticleSystem::ParticleSystem() {
}

void ParticleSystem::setup(int radius) {
	this->radius = radius;
	
	int step = 255 / 5;
	int center = step / 2;
	newYork.setup(center + step * 0, "faces/01_New_York");
	london.setup(center + step * 1, "faces/02_London");
	saoPaolo.setup(center + step * 2, "faces/03_Sao_Paolo");
	seoul.setup(center + step * 3, "faces/04_Seoul");
	beijing.setup(center + step * 4, "faces/05_Beijing");

	cities.push_back(&newYork);
	cities.push_back(&london);
	cities.push_back(&saoPaolo);
	cities.push_back(&seoul);
	cities.push_back(&beijing);
}

ParticleSystem::~ParticleSystem() {
	for(int i = 0; i < particles.size(); i++)
		delete particles[i];
}

void ParticleSystem::drawAxes() const {
	float axisRadius = .1;

	ofPushStyle();

	glPushMatrix();
	glTranslatef(1, 0, 0);
	ofSetColor(255, 0, 0);
	glutSolidCube(axisRadius);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 1, 0);
	ofSetColor(0, 255, 0);
	glutSolidCube(axisRadius);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 1);
	ofSetColor(0, 0, 255);
	glutSolidCube(axisRadius);
	glPopMatrix();

	ofPopStyle();
}

void ParticleSystem::update() {
	// remove dead particles	
	for(int i = size() - 1; i >= 0; i--) {
		if(particles[i]->isDead()) {
			delete particles[i];
			particles.erase(particles.begin() + i);
		}
	}
	
	// and sum relative radii
	float radiiSum = 0;
	for(int i = 0; i < size(); i++) {
		particles[i]->updateRelativeRadius();
		radiiSum += particles[i]->relativeRadius;
	}

	// determine radius scale factor based on coverage and packing
	float maxRadius = acosf(1 - (PI / (radiiSum * sqrtf(3))));
	float radiusScale = maxRadius * sphereCoverage;

	// update all absolute radii using scale
	for(int i = 0; i < size(); i++)
		particles[i]->updateAbsoluteRadius(radiusScale);

	// compute forces between all particles
	for(int i = 0; i < size(); i++) {
		Particle* cur = particles[i];
		cur->clearForces();
		for(int j = 0; j < size(); j++)
			if(i != j)
				cur->addForce(*(particles[j]));
	}

	// apply forces to all particles
	for(int i = 0; i < particles.size(); i++)
		particles[i]->update();
}

void ParticleSystem::draw() {
	ofPushMatrix();
	ofPushStyle();
	glScalef(radius, radius, radius);
	ofNoFill();
	if(showAxes)
		drawAxes();
	for(int i = 0; i < size(); i++)
		particles[i]->draw();
	ofPopStyle();
	ofPopMatrix();
}

int ParticleSystem::size() const {
	return particles.size();
}

int ParticleSystem::livingSize() const {
	int n = 0;
	for(int i = 0; i < size(); i++)
		if(!particles[i]->dying)
			n++;
	return n;
}

void ParticleSystem::moveTowards(int target) {
	while(livingSize() != target) {
		// take one step towards the target number
		if(livingSize() < target)
			birth();
		else if(livingSize() > target)
			kill();
	}
}

void ParticleSystem::birth() {
	// add a new particle at a random position
	// make this informed by particles that have recently died
	particles.push_back(new Particle(ofRandom(-HALF_PI, HALF_PI), ofRandom(-PI, PI)));
	// associate with new york.
	// make this informed by the current city radius
	newYork.setupParticle(particles.back());
}

void ParticleSystem::kill() {
	// kill a particle that isn't already dying, starting at the end
	for(int i = size() - 1; i >= 0; i--)
		if(!particles[i]->dying) {
			particles[i]->kill();
			break;
		}
}