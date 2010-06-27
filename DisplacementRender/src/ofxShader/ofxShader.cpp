#include "ofxShader.h"

ofxShader::ofxShader() :
	bLoaded(false) {
}

ofxShader::~ofxShader() {
	unload();
}

void ofxShader::setup(string shaderName) {
	unload();
	string fragmentName = shaderName + ".frag";
	string vertexName = shaderName + ".vert";
	setup(fragmentName, vertexName);
}

void ofxShader::setup(string fragmentName, string vertexName) {
	bLoaded = false;
	if (GLEE_ARB_shader_objects) {
		vertexShader = (GLhandleARB) glCreateShader(GL_VERTEX_SHADER);
		fragmentShader = (GLhandleARB) glCreateShader(GL_FRAGMENT_SHADER);

		string vs = loadShaderText(vertexName);
		string fs = loadShaderText(fragmentName);
		const char* vsptr = vs.c_str();
		const char* fsptr = fs.c_str();
		int vssize = vs.size();
		int fssize = fs.size();

		glShaderSourceARB(vertexShader, 1, &vsptr, &vssize);
		glShaderSourceARB(fragmentShader, 1, &fsptr, &fssize);

		glCompileShaderARB(vertexShader);

		//please add compile status check in:
		//GLint compileStatus = 0;
		//glGetObjectParameterivARB( vertexShader, GL_COMPILE_STATUS, &compileStatus );
		//printf("%i \n", compileStatus);

		char infobuffer[1000];
		GLsizei infobufferlen = 0;

		glGetInfoLogARB(vertexShader, 999, &infobufferlen, infobuffer);
		if (infobufferlen != 0) {
			infobuffer[infobufferlen] = 0;
			printf("vertexShader reports: %s \n", infobuffer);
			return;
		}

		glCompileShaderARB(fragmentShader);

		//glGetObjectParameterivARB( fragmentShader, GL_COMPILE_STATUS, &compileStatus );
		//printf("%i \n", compileStatus);

		glGetInfoLogARB(fragmentShader, 999, &infobufferlen, infobuffer);
		if (infobufferlen != 0) {
			infobuffer[infobufferlen] = 0;
			printf("fragmentShader reports: %s \n", infobuffer);
			return;
		}

		shader = glCreateProgramObjectARB();
		glAttachObjectARB(shader, vertexShader);
		glAttachObjectARB(shader, fragmentShader);
		glLinkProgramARB(shader);

		bLoaded = true;
	} else {
		cout << "Sorry, it looks like you can't run 'ARB_shader_objects'." << endl;
		cout << "Please check the capabilites of your graphics card." << endl;
		cout << "http://www.ozone3d.net/gpu_caps_viewer/)" << endl;
	}
}

void ofxShader::unload() {
	if(bLoaded) {
		if (vertexShader) {
			glDetachObjectARB(shader, vertexShader);
			glDeleteObjectARB(vertexShader);
			vertexShader = 0;
		}
		if (fragmentShader) {
			glDetachObjectARB(shader, fragmentShader);
			glDeleteObjectARB(fragmentShader);
			fragmentShader = 0;
		}
		if (shader) {
			glDeleteObjectARB(shader);
			shader = 0;
		}
	}
	bLoaded = false;
}

void ofxShader::begin() {
	if (bLoaded == true)
		glUseProgramObjectARB(shader);
}

void ofxShader::end() {
	if (bLoaded == true)
		glUseProgramObjectARB(0);
}

void ofxShader::setSampler2d(string name, ofImage& img, int textureLocation) {
	if(bLoaded) {
		GLint uniformLocation = glGetUniformLocation((GLuint) shader, name.c_str());
		glUniform1i(uniformLocation, textureLocation);
		glActiveTexture(GL_TEXTURE0 + textureLocation);
		img.getTextureReference().bind();
	}
}

void ofxShader::setUniform1f (string name, float value) {
	if(bLoaded)
		glUniform1fARB(glGetUniformLocationARB(shader, name.c_str()), value);
}
void ofxShader::setUniform1i (string name, int value) {
	if(bLoaded)
		glUniform1iARB(glGetUniformLocationARB(shader, name.c_str()), value);
}
void ofxShader::setUniform1fv (string name, int count, float * value) {
	if(bLoaded)
		glUniform1fvARB(glGetUniformLocationARB(shader, name.c_str()), count, value);
}
void ofxShader::setUniform3fv (string name, int count, float * value) {
	if(bLoaded)
		glUniform3fvARB(glGetUniformLocationARB(shader, name.c_str()), count, value);
}
void ofxShader::setUniform4fv (string name, int count, float * value) {
	if(bLoaded)
		glUniform4fvARB(glGetUniformLocationARB(shader, name.c_str()), count, value);
}
void ofxShader::setUniform2f (string name, float value, float value2) {
	if(bLoaded)
		glUniform2fARB(glGetUniformLocationARB(shader, name.c_str()), value, value2);
}
void ofxShader::setUniform3f (string name, float value, float value2, float value3) {
	if(bLoaded)
		glUniform3fARB(glGetUniformLocationARB(shader, name.c_str()), value, value2, value3);
}

string ofxShader::loadShaderText(string filename) {
	ifstream file;
	file.open(ofToDataPath(filename).c_str());
	string text;
	while(!file.eof()) {
		string line;
		getline(file, line);
		text += line + '\n';
	}
	file.close();
	text += '\0';
	return text;
}

void ofxShader::logError() {
	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		const GLubyte* errString = gluErrorString(err);
		ofLog(OF_LOG_ERROR, (const char*) errString);
	}
}
