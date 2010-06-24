#include "testApp.h"
extern "C" {
#include "EnvMap.h"
}

//--------------------------------------------------------------
void testApp::setup(){
	doSomething();
	
	light.pointLight(255,255,0, 100,100,100);
	
	panel.setup("settings", 0, 0, 300, 500);
	panel.addPanel("misc", 1, false);
	panel.addSlider("ripple speed", "rippleSpeed", 0.05, 0.01, 2.0, false);
	panel.addSlider("ripple amnt", "rippleAmount", 0.05, 0.0, 2.0, false);
	panel.addSlider("ripple scale x", "scaleX", 0.05, 0.01, 2.0, false);
	panel.addSlider("ripple scale y", "scaleY", 0.05, 0.0, 2.0, false);
	
	panel.loadSettings("settings.xml");
}

//--------------------------------------------------------------
void testApp::update(){
	panel.update();
	
}

//--------------------------------------------------------------
void testApp::draw(){

	ofxLightsOn();
	light.on();
	
	/*
	ofSetRectMode(OF_RECTMODE_CENTER);
	ofPushMatrix();
	ofTranslate(400,300);
	ofRotateZ(ofGetElapsedTimef()*10);
	ofRect(0,0,20,20);
	ofPopMatrix();
	
	
	string temp = "blah";
	string temp1 = "";
	temp1 += temp[0];
	ofDrawBitmapString(temp1 ,90,90);
	ofSetColor(255, 255, 255);
	glPushMatrix();
	glTranslatef(500, 500, -20);
	glScalef(500, 500, 500);
	ofRotate(ofGetElapsedTimef()*30, 1,1,0.2);
	doSomthingElse();
	glPopMatrix();
	*/
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);
	
	startShader();
	
	glPushMatrix();
	glTranslatef(ofGetWidth()/2, ofGetHeight()/2, -20);
	
	//glScalef(200, 200, 200);
	//ofRotate(ofGetElapsedTimef()*100, 1,1,0.2);
	//ofRotate(180, 0, 1, 0);
	//ofRotate(90, 1, 0, 0);
	
	float theta,nextTheta, phi, x, y, z;
	int sphereResolution = 120;
	
	float scale = 190.0;
	float relief = 0.0;
	
	float amnt = panel.getValueF("rippleAmount");
	float speed = panel.getValueF("rippleSpeed");
	float scaleX = panel.getValueF("scaleX");
	float scaleY = panel.getValueF("scaleY");
	
	for(int i=0; i<sphereResolution; i++){
		theta = (float)i/(sphereResolution-1) * TWO_PI;
		nextTheta = (float)((i+1)%sphereResolution)/(sphereResolution-1) * TWO_PI;
		glBegin(GL_QUAD_STRIP);
		for(int j=0; j<sphereResolution; j++){
		
			phi		= (float)j/(sphereResolution-1) * PI;
			relief	= 1.0 + amnt * ofSignedNoise(phi*scaleX, theta*scaleY, ofGetElapsedTimef() * speed);
			
			x		= cos(theta) * sin(phi) * relief;
			y		= sin(theta) * sin(phi) * relief;
			z		= cos(phi) * relief;
			
			float noise = ofNoise(x/3.0,y/3.0,z/3.0,ofGetElapsedTimef()*0.2);
			noise = 2*noise - 1;
			noise = fabs(noise) ;
			//noise = powf(noise, 1);
			
			glNormal3f(x + x*noise*0.5, y + y*noise*0.5, z + z*noise*0.5);
			glVertex3f( (x + x*noise*0.5) * scale, (y + y*noise*0.5) * scale, (z + z*noise*0.5) * scale);
			
			relief =  1.0 + amnt * ofSignedNoise(phi*scaleX, nextTheta*scaleY, ofGetElapsedTimef() * speed );
			
			x = cos(nextTheta) * sin(phi) * relief;
			y = sin(nextTheta) * sin(phi) * relief;
			z = cos(phi) * relief;
			
			noise = ofNoise(x/3.0,y/3.0,z/3.0,ofGetElapsedTimef()*0.2);
			noise = 2*noise - 1;
			noise = fabs(noise) ;
			//noise = powf(noise, 1);
			
			glNormal3f(x + x*noise*0.5, y + y*noise*0.5, z + z*noise*0.5);
			glVertex3f( (x + x*noise*0.5) * scale, (y + y*noise*0.5) * scale, (z + z*noise*0.5) * scale);
		}
		glEnd();
	}
	endShader();
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	ofPopMatrix();
	
	ofxLightsOff();
	panel.draw();
		
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	
}





//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

