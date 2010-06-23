#include "testApp.h"

captureApp * cAppPtr = NULL;
decodeApp  * dAppPtr = NULL;
		
//--------------------------------------------------------------
void testApp::setup(){
	bCapture = true;
	dApp.setup();
	cApp.setup();
	
	cAppPtr = &cApp;
	dAppPtr = &dApp;
}

//--------------------------------------------------------------
void testApp::update(){
	if( bCapture ){
		cApp.update();
	}else{
		dApp.update();
	}	
}

//--------------------------------------------------------------
void testApp::draw(){
	if( bCapture ){
		cApp.draw();
	}else{
		dApp.draw();
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if( key == OF_KEY_RIGHT ){
		
		//update the folderList
		if( bCapture ){
			dApp.inputList.listDir(CAPTURE_MAIN_FOLDER);
			dApp.inputList.reverseOrder();
		}
		bCapture = false;
	}
	else if( key == OF_KEY_LEFT ){
		bCapture = true;
	}	
	else{
		if( bCapture ){
			cApp.keyPressed(key);
		}else{
			dApp.keyPressed(key);
		}
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	if( bCapture ){
		cApp.mouseDragged(x, y, button);
	}else{
		dApp.mouseDragged(x, y, button);
	}
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	if( bCapture ){
		cApp.mousePressed(x, y, button);
	}else{
		dApp.mousePressed(x, y, button);
	}
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	if( bCapture ){
		cApp.mouseReleased(x, y, button);
	}else{
		dApp.mouseReleased(x, y, button);
	}
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

