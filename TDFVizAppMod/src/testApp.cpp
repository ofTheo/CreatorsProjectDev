#include "testApp.h"

//---------------------------------------------------------------------------------
void testApp::setup(){
		
	// setup control panel
	panel.setup("control", 0, 0, 1023, 768);
	panel.loadSettings("control.xml");
	panel.addPanel("input", 4);
	panel.addPanel("misc", 4);

	panel.setWhichPanel("input");
	inputList.listDir("input");
	panel.addFileLister("input", &inputList, 200, 100);

	panel.addToggle("show face on ball", "showFaceOnBall", true);
	panel.addToggle("do shader", "doShader", 0);
	panel.addToggle("adaptive offset", "adaptiveOffset", true);
	
	panel.addSlider("adaptive scaling", "adaptiveScaling", 2, 0, 3, false);
	panel.addSlider("offset smoothing", "offsetSmoothing", 0, 0, 1, false);
	panel.addSlider("fixed offset", "fixedOffset", 0, 0, 255, false);

	panel.addSlider("face on ball z scale", "zPercent", 0.6, 0.0, 1.0, false);
	
	panel.addSlider("normal smooth amnt", "normalSmooth", 0, 0, 0.988, false);

	panel.addSlider("ball scale", "ballScale", 200.0, 90, 400, false);
	
	panel.addDrawableRect("alpha histogram", &SP.histogram, 200, 60);
	panel.addDrawableRect("depth histogram", &SP.histogramAfter, 200, 60);
	
	panel.setWhichColumn(1);
	panel.addDrawableRect("fbo", &SP.FBO, 720, 540);
	panel.addChartPlotter("fade value", guiStatVarPointer("fade", &SP.pctFadeIn, GUI_VAR_FLOAT, true, 2), 300, 40, 300, -0.2, 1.2);
	
	panel.setWhichPanel("misc");
	panel.addSlider("rotate x", "rotateX", 90, 0, 360, false);
	panel.addSlider("rotate y", "rotateY", 90, 0, 360, false);	
	panel.addSlider("mask x", "maskX", 0, -200, 200, false);
	panel.addSlider("mask y", "maskY", 0, -200, 200, false);
	panel.addSlider("mask w", "maskW", 1024, 800, 1900, false);
	panel.addSlider("mask h", "maskH", 768,  600, 1600, false);
	
	panel.addSlider("base color r", "baseColorR", .4, 0, 1, false);
	panel.addSlider("base color g", "baseColorG", .4, 0, 1, false);
	panel.addSlider("base color b", "baseColorB", .6, 0, 1, false);
	panel.addSlider("env ratio", "envRatio", .8, 0, 1, false);		
	panel.addSlider("solid ratio", "solidRatio", .8, 0, 1, false);		
	panel.addSlider("light x", "lightX", 0, -400, 400, false);		
	panel.addSlider("light y", "lightY", 200, -400, 400, false);		
	panel.addSlider("light z", "lightZ", 200, -400, 400, false);		
	
	panel.setWhichColumn(1);
	panel.addDrawableRect("fbo", &SP.FBO, 720, 540);

	
	panel.loadSettings("control.xml");
	
	SP.setup();
	SP.loadDirectory("input/export-capture-NYCDev-1277310206");
	
	SN.setup("network.xml");
	SN.enable();
}

//---------------------------------------------------------------------------------
testApp::~testApp() {

}

//---------------------------------------------------------------------------------
void testApp::update() {

	if( SN.update() ){
		if( SN.message == "TxStarted" && SN.folder != ""){
			SP.startFadeOut();
		}
		if( SN.message == "TxEnded" && SN.folder != "" ){
			printf("opening via OSC - %s\n", string("/Users/theo/Desktop/INCOMING_SCANS/"+SN.folder).c_str());
			SP.loadDirectory("/Users/theo/Desktop/INCOMING_SCANS/"+SN.folder);
			SN.clearData();
		}
	}
	
	panel.update();
	
	ofBackground(50,50,50);

	bool reload = inputList.selectedHasChanged();
	if (reload) {
		//setupInput();
		string name = inputList.getSelectedName();
		SP.loadDirectory("input/" + name);
		inputList.clearChangedFlag();
	}
	
	SP.update();
	panel.clearAllChanged();
}

//---------------------------------------------------------------------------------
void testApp::draw() {
	SP.draw();
	panel.draw();
}

//---------------------------------------------------------------------------------
void testApp::keyPressed(int key) {

	if( key == 'f'){
		ofToggleFullscreen();
		ofSetWindowPosition(0, 0);
		
	}
	

	//THEO
	if( key == 'e' ){
		SP.startFadeOut();
	}
	
	//THEO TODO: move this to transfer folder
	if( key == 'n' ){
		int num = inputList.listDir("input");
		if( num > 0 ){
			SP.loadDirectory(inputList.getPath( (int)ofRandom(0, (float)num * 0.999) ) );
		}
	}
	if( key == 'i' ){
		int num = inputList.listDir("/Users/theo/Desktop/INCOMING_SCANS/");
		if( num > 0 ){
			SP.loadDirectory(inputList.getPath( (int)ofRandom(0, (float)num * 0.999) ) );
		}
	}	
}

float preX;
void testApp::mousePressed(int x, int y, int button){
	if( !panel.mousePressed(x, y, button) || x > 300 ){
		preX = x;
	}
}

//---------------------------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
	if( !panel.mouseDragged(x, y, button) || x > 300 ){
		//SP.dx += 1.2 * ( (float)x-preX);
	}
	preX = x;
	
}

//---------------------------------------------------------------------------------
void testApp::mouseReleased(){
	panel.mouseReleased();
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
