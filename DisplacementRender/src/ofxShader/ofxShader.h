#pragma once

#include "ofMain.h"
#include <fstream>

class ofxShader {
public:
	ofxShader();
	~ofxShader();

	void setup(string shaderName);
	void setup(string fragmentShaderName, string vertexShaderName);
	void logError();

	void begin();
	void end();

	void setSampler2d(string name, ofImage& img, int textureLocation);

	void setUniform1f(string name, float value);
	void setUniform1i(string name, int value);
	void setUniform2f(string name, float value, float value2);
	void setUniform3f(string name, float value, float value2, float value3);
	void setUniform1fv(string name, int count, float* value);
	void setUniform3fv(string name, int count, float* value);
	void setUniform4fv(string name, int count, float* value);

	GLhandleARB vertexShader;
	GLhandleARB fragmentShader;
	GLhandleARB shader;

protected:
	string loadShaderText(string filename);
	void unload();

	bool bLoaded;
};
