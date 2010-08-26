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
	// remove dead particles and sum relative radii
	float radiiSum = 0;
	for(int i = 0; i < cities.size(); i++) {
		cities[i]->removeDead();
		radiiSum += cities[i]->sumRadii();
	}

	// determine radius scale factor based on coverage and packing
	float maxRadius = acosf(1 - (PI / (radiiSum * sqrtf(3))));
	float radiusScale = maxRadius * sphereCoverage;

	// update all absolute radii using scale
	for(int i = 0; i < cities.size(); i++)
		cities[i]->updateAbsoluteRadius(radiusScale);

	// make a single vector of all the particles
	vector<Particle*> particles;
	for(int i = 0; i < cities.size(); i++) {
		vector<Particle*>& cur = cities[i]->particles;
		for(int j = 0; j < cur.size(); j++)
			particles.push_back(cur[j]);
	}

	// compute forces between all particles
	for(int i = 0; i < particles.size(); i++) {
		Particle* cur = particles[i];
		cur->clearForces();
		for(int j = 0; j < particles.size(); j++)
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
	for(int i = 0; i < cities.size(); i++)
		cities[i]->draw();
	ofPopStyle();
	ofPopMatrix();
}

void ParticleSystem::moveTowards(int target) {
	/*
	while(livingSize() != target) {
		// take one step towards the target number
		if(livingSize() < target)
			birth();
		else if(livingSize() > target)
			kill();
	}*/
}
