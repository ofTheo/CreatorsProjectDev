#include "PatternGenerator.h"

void PatternGenerator::allocateSequence(int n) {
	sequence.clear();
	for(int i = 0; i < n; i++) {
		sequence.push_back(ofImage());
		sequence.back().allocate(width, height, OF_IMAGE_COLOR);
	}
}

PatternGenerator::PatternGenerator() :
	width(0), height(0),
	reverse(false) {
}

void PatternGenerator::applyLut(string filename) {
	LutFilter lut;
	lut.setup(filename);

	for(int i = 0; i < size(); i++) {
		ofImage& cur = sequence[i];
		// currently LUT is grayscale
		cur.setImageType(OF_IMAGE_GRAYSCALE);
		lut.filter(cur);
		cur.update();
	}
}

void PatternGenerator::setSize(int width, int height) {
	this->width = width;
	this->height = height;
}

void PatternGenerator::setReverse(bool reverse) {
	this->reverse = reverse;
}

int PatternGenerator::getWidth() {
	return width;
}

int PatternGenerator::getHeight() {
	return height;
}

int PatternGenerator::size() {
	return sequence.size();
}

ofImage& PatternGenerator::get(int i) {
	i %= size();
	if(reverse) {
		return sequence[size() - i - 1];
	} else {
		return sequence[i];
	}
}
