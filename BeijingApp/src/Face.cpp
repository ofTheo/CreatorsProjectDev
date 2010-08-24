#include "Face.h"

float Face::faceScale = 1;

Face::Face() :
		curImage(0),
		dirSize(0),
		countingUp(true),
		needsUpdating(true) {
}

void Face::setup(unsigned char id, string path) {
	this->id = id;
	dirSize = dirList.listDir(path);
}

void Face::update() {
	img.loadImage(dirList.getPath(curImage));
	w = img.getWidth();
	h = img.getHeight();
	fillId();
	analyzeImage();

	needsUpdating = false;
}

void Face::next() {
	curImage++;
	if(curImage == dirSize)
		curImage = 0;

	needsUpdating = true;
}

void Face::fillId() {
	unsigned char* pixels = img.getPixels();
	const int channels = 4;
	const int alphaOffset = 3;
	int n = w * h * channels;
	for(int i = 0; i < n; i += channels) {
		/*
		if(pixels[i + alphaOffset] > 128)
			pixels[i] = id;
		else
			pixels[i] = 0;
		*/
		pixels[i] = id;
	}
	img.update();
}

void Face::analyzeImage() {
	unsigned char* pixels = img.getPixels();
	const int channels = 4;
	const int alphaOffset = 3;

	// compute weighted sum of mean using alphas
	float xSum = 0;
	float ySum = 0;
	float weights = 0;
	for(int y = 0; y < h; y++) {
		int i = (y * w) * channels;
		for(int x = 0; x < w; x++) {
			unsigned char alpha = pixels[i + alphaOffset];
			xSum += x * alpha;
			ySum += y * alpha;
			weights += alpha;
			i += channels;
		}
	}
	xMean = xSum / weights;
	yMean = ySum / weights;

	/*
	// compute standard deviation
	float xSumSq = 0;
	float ySumSq = 0;
	for(int y = 0; y < h; y++) {
		int i = (y * w) * channels;
		for(int x = 0; x < w; x++) {
			unsigned char alpha = pixels[i + alphaOffset];
			float xDiff = x - xMean;
			float yDiff = y - yMean;
			xSumSq += alpha * xDiff * xDiff;
			ySumSq += alpha * yDiff * yDiff;
			i += channels;
		}
	}
	xStdDev = sqrtf(xSumSq / weights);
	yStdDev = sqrtf(ySumSq / weights);
	*/
}

void Face::draw() {
	if(needsUpdating)
		update();

	glPushMatrix();
	glScalef(faceScale / w, faceScale / h, 1.);
	glTranslatef(-xMean, -yMean, 0);
	img.draw(0, 0);
	glPopMatrix();
}
