#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 2048, 768, OF_FULLSCREEN);
	ofRunApp(new testApp());
}
