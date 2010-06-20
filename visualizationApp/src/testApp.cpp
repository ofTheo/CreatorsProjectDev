#include "testApp.h"

#include <mach-o/dyld.h>




void testApp::setup() {
	
	// fix mac paths if necessary: 
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0){
		printf("executable path is %s\n", path);
		string pathStr = string(path);
		vector < string> pathBrokenUp = ofSplitString( pathStr, "/");
		string newPath = "/";
		for(int i = 0; i < pathBrokenUp.size()-1; i++){
			newPath += pathBrokenUp[i];
			newPath += "/";
		}
		cout << newPath << endl;
		system( "pwd" );
		chdir ( newPath.c_str() );
		system("pwd");
	} else
		printf("buffer too small; need size %u\n", size);
	
	
		// setup control panel
	panel.setup("control", 0, 0, 300, 720);
	panel.loadSettings("control.xml");
	panel.addPanel("input", 1);
	panel.addPanel("decode", 1);
	panel.addPanel("export", 1);
	panel.addPanel("misc", 1);
	panel.setWhichPanel("input");
	inputList.listDir("input");
	panel.addFileLister("input", &inputList, 240, 440);
	panel.addSlider("camera rate", "cameraRate", 1, 1, 6, true);
	panel.addSlider("camera offset", "cameraOffset", 0, 0, 5, true);
	panel.addSlider("play rate", "playRate", 1, 1, 60, true);
	panel.setWhichPanel("decode");
	panel.addToggle("stop motion", "stopMotion", false);
	panel.addToggle("play sequence", "playSequence", false);
	panel.addSlider("jump to", "jumpTo", 0, 0, 100, false);
	panel.addToggle("phase persistence", "phasePersistence", false);
	panel.addToggle("reset view ", "resetView", false);
	vector<string> styles;
	styles.push_back("cloud");
	styles.push_back("mesh");
	styles.push_back("none");
	panel.addMultiToggle("style", "style", 0, styles);
	vector<string> orientation;
	orientation.push_back("horizontal");
	orientation.push_back("vertical");
	panel.addMultiToggle("orientation", "orientation", 0, orientation);
	panel.addSlider("range threshold", "rangeThreshold", 40, 0, 255, true);
	panel.addSlider("depth scale", "depthScale", 130, -500, 500, false);
	panel.addSlider("depth skew", "depthSkew", -28, -50, 50, false);
	panel.addSlider("filter min", "filterMin", -1024, -1024, 1024, false);
	panel.addSlider("filter max", "filterMax", 1024, -1024, 1024, false);
	panel.setWhichPanel("export");
	vector<string> exportFormats;
	exportFormats.push_back(".obj");
	exportFormats.push_back(".ply");
	exportFormats.push_back(".png");
	panel.addMultiToggle("export format", "exportFormat", 0, exportFormats);

	panel.addToggle("export", "export", false);
	panel.addToggle("record", "record", false);
	panel.addToggle("render movie", "renderMovie", false);
	panel.addSlider("movie framerate", "movieFramerate", 60, 5, 60, true);
	panel.setWhichPanel("misc");
	panel.addSlider("gamma", "gamma", 1, 0.0, 1.0, false);
	panel.addToggle("hud", "hud", false);
	panel.addSlider("hudWidth", "hudWidth", 300.0, 0.0, 2000.0, false);
	panel.addSlider("hudHeightOffset", "hudHeightOffset", 0.0, 0.0, 1.0, false);
	panel.addSlider("maxPhase", "maxPhase", 10.0, 0.0, 100.0, false);
	panel.addSlider("maxDepth power", "maxDepth", 3.0, 0.0, 5.0, false);
	
	
	SP.setup();
	SP.loadDirectory("input/output3");
	
}



testApp::~testApp() {
}



void testApp::update() {
	
	

	float curDepthScale = panel.getValueF("depthScale");
	float curDepthSkew = panel.getValueF("depthSkew");
	int curRangeThreshold = panel.getValueI("rangeThreshold");
	int curOrientation = panel.getValueI("orientation");
	float curFilterMin = panel.getValueF("filterMin");
	float curFilterMax = panel.getValueF("filterMax");
	bool curPlaySequence = panel.getValueB("playSequence");
	int curPlayRate = panel.getValueI("playRate");
	float curJumpTo = panel.getValueF("jumpTo");
	bool curPhasePersistence = panel.getValueB("phasePersistence");
	int curCameraRate = panel.getValueI("cameraRate");
	int curCameraOffset = panel.getValueI("cameraOffset");
	bool curStopMotion = panel.getValueB("stopMotion");

	float gamma = panel.getValueF("gamma");

	bool curResetView = panel.getValueB("resetView");
	if (curResetView) {
		camera = ofxEasyCam();
		panel.setValueB("resetView", false);
	}

	bool reload = inputList.selectedHasChanged();
	if (reload) {
		//setupInput();
		string name = inputList.getSelectedName();
		SP.loadDirectory("input/" + name);
		inputList.clearChangedFlag();
	}
	
	SP.update();

}


//void testApp::drawAxes(float size) {
//	ofPushMatrix();
//	ofScale(size, size, size);
//	ofPushStyle();
//	ofSetLineWidth(2.0);
//	ofSetColor(255, 0, 0);
//	glBegin(GL_LINES);
//	glVertex3s(0, 0, 0);
//	glVertex3s(1, 0, 0);
//	glEnd();
//	ofSetColor(0, 255, 0);
//	glBegin(GL_LINES);
//	glVertex3s(0, 0, 0);
//	glVertex3s(0, 1, 0);
//	glEnd();
//	ofSetColor(0, 0, 255);
//	glBegin(GL_LINES);
//	glVertex3s(0, 0, 0);
//	glVertex3s(0, 0, 1);
//	glEnd();
//	ofPopStyle();
//	ofPopMatrix();
//}

void testApp::draw() {
	
	SP.draw();
}

void testApp::keyPressed(int key) {
	
}



//void testApp::drawCloud() {
//	bool* mask = threePhase->getMask();
//	float* depth = threePhase->getDepth();
//	byte* color = threePhase->getColor();
//	
//	int srcWidth = threePhase->getWidth();
//	int srcHeight = threePhase->getHeight();
//	
//	glEnable(GL_POINT_SIZE);
//	glPointSize(3);
//	glBegin(GL_POINTS);
//	int i = 0;
//	for (int y = 0; y < srcHeight; y++) {
//		for (int x = 0; x < srcWidth; x++) {
//			if (!mask[i]) {
//				
//				
//				depthSmoothed[i] = 0.98f * depthSmoothed[i] + 0.02f * depth[i];
//				glColor3ubv(&color[i * 3]);
//				glVertex3f(x, y, depthSmoothed[i]);
//				
//			}
//			i++;
//		}
//	}
//	glEnd();
//}
//
//
//void testApp::drawMesh() {
//	bool* mask = threePhase->getMask();
//	float* depth = threePhase->getDepth();
//	byte* color = threePhase->getColor();
//	
//	int srcWidth = threePhase->getWidth();
//	int srcHeight = threePhase->getHeight();
//	
//	glBegin(GL_TRIANGLES);
//	for (int y = 0; y < srcHeight - 1; y++) {
//		for (int x = 0; x < srcWidth - 1; x++) {
//			int nw = y * srcWidth + x;
//			int ne = nw + 1;
//			int sw = nw + srcWidth;
//			int se = ne + srcWidth;
//			if (!mask[nw] && !mask[se]) {
//				if (!mask[ne]) { // nw, ne, se
//					glColor3ubv(&color[nw * 3]);
//					glVertex3f(x, y, depth[nw]);
//					glColor3ubv(&color[ne * 3]);
//					glVertex3f(x + 1, y, depth[ne]);
//					glColor3ubv(&color[se * 3]);
//					glVertex3f(x + 1, y + 1, depth[se]);
//				}
//				if (!mask[sw]) { // nw, se, sw
//					glColor3ubv(&color[nw * 3]);
//					glVertex3f(x, y, depth[nw]);
//					glColor3ubv(&color[se * 3]);
//					glVertex3f(x + 1, y + 1, depth[se]);
//					glColor3ubv(&color[sw * 3]);
//					glVertex3f(x, y + 1, depth[sw]);
//				}
//			} else if (!mask[ne] && !mask[sw]) {
//				if (!mask[nw]) { // nw, ne, sw
//					glColor3ubv(&color[nw * 3]);
//					glVertex3f(x, y, depth[nw]);
//					glColor3ubv(&color[ne * 3]);
//					glVertex3f(x + 1, y, depth[ne]);
//					glColor3ubv(&color[sw * 3]);
//					glVertex3f(x, y + 1, depth[sw]);
//				}
//				if (!mask[se]) { // ne, se, sw
//					glColor3ubv(&color[ne * 3]);
//					glVertex3f(x + 1, y, depth[ne]);
//					glColor3ubv(&color[se * 3]);
//					glVertex3f(x + 1, y + 1, depth[se]);
//					glColor3ubv(&color[sw * 3]);
//					glVertex3f(x, y + 1, depth[sw]);
//				}
//			}
//		}
//	}
//	glEnd();
//}
