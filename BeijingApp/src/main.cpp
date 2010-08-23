#include "ofMain.h"
#include "testApp.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1024, 256 * 3, OF_WINDOW);
	ofRunApp(new testApp());
}
