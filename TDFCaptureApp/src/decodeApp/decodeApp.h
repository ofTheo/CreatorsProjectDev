/*
	model (ofxStructuredLight)
		good
	view (vbo based rendering)
		only call decode() once per 3d frame unless told to update()
	controller (ofxControlPanel)
		implement connections to m+v -- maybe import/export just in xml now, decode in control panel?
		implement importer + exporter wrappers
		eventually add file dialog... for now specify via xml
*/

#pragma once

#include "ofMain.h"
#include "ofxEasyCam.h"
#include "ofxStructuredLight.h"
#include "ofxAutoControlPanel.h"
#include "ofxQtVideoSaver.h"
#include "ofxFileHelper.h"
#include "ofxNumericalDirList.h"
#include <fstream>

#include "appConstants.h"

class decodeApp : public ofBaseApp {

public:
	~decodeApp();
	void setup();
	void update();
	void draw();

	//--------------------------------------------------------------
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);	
	void mouseReleased(int x, int y, int button);
	void keyPressed(int key);
	
	void initDecoder(int width, int height);
	void updateDecoderSettings();
	
	void handlePlayback();
	void handleExport();
		void pipeline(unsigned char * pixels, int numChannelsIn, int frameIndex);
	
	void processFrame();
	
	void drawCloud();
	void drawMesh();

	void getBounds(ofxPoint3f& min, ofxPoint3f& max);
	void boxOutline(ofxPoint3f min, ofxPoint3f max);
	void drawAxes(float size);

	void nextFrame();
	void jumpTo(unsigned frame);
	void setupInput();

	bool reload;

	ofxEasyCam camera;

	LutFilter lut;

	FastThreePhase* threePhase;
	
	string inputDir;

	simpleFileLister inputList;
	ofxNumericalDirList imageList;
	ofVideoPlayer movieInput;
	bool usingDirectory;
	unsigned totalImages;

	ofImage phase;

	ofxAutoControlPanel panel;
	vector<string> styles, exportFormats;
	bool hidden;

	/// visualizations
	bool redraw;
	ofImage rangeIm;
	ofImage phaseWrapped;
	ofImage phaseUnwrapped;
	ofImage depthIm;
	ofImage unwrapOrderIm;
	ofImage phaseWrappedPlot;
	ofImage sourceImagesPlot;

	int sequenceFrame;

	ofxQtVideoSaver movieOutput;
	ofxQtVideoSaver expPngMovieSaver;
	ofTexture rgbaTex, wrappedPhaseTex;
	string currentName;
	ofImage screenCapture;
	

	static const unsigned char scol[8][3];

	/// expects a 0-1.0 float
	ofColor makeColor(float f);
	
	static const int filterMin = -320;
	static const int filterMax = 320;
};
