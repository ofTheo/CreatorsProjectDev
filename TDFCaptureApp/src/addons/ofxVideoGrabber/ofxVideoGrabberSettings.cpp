#include "ofxVideoGrabberSettings.h"
#include "ofxVideoGrabberSDK.h"
#include "ofxVideoGrabberFeature.h"

ofxVideoGrabberSettings::ofxVideoGrabberSettings(){
	bGuiEvents = false;
}

ofxVideoGrabberSettings::~ofxVideoGrabberSettings(){
	disableGuiEvents();
    //dtor
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::disableGuiEvents(){
	if( bGuiEvents ){
		ofRemoveListener(ofEvents.mousePressed, this, &ofxVideoGrabberSettings::mousePressed);
		ofRemoveListener(ofEvents.mouseDragged, this, &ofxVideoGrabberSettings::mouseDragged);
		ofRemoveListener(ofEvents.mouseReleased, this, &ofxVideoGrabberSettings::mouseReleased);	
		bGuiEvents = false;
	}
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::enableGuiEvents(){
	if( !bGuiEvents ){
		ofAddListener(ofEvents.mousePressed, this, &ofxVideoGrabberSettings::mousePressed);
		ofAddListener(ofEvents.mouseDragged, this, &ofxVideoGrabberSettings::mouseDragged);
		ofAddListener(ofEvents.mouseReleased, this, &ofxVideoGrabberSettings::mouseReleased);	
		bGuiEvents = true;
	}
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::setupVideoSettings(ofxVideoGrabberSDK* _videoGrabber)
{
    videoGrabber = _videoGrabber;
    setupGUI();
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::toggleSettingsGUI()
{
    if(panel.hidden)
        panel.show();
    else
        panel.hide();
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mousePressed(ofMouseEventArgs & args)
{
    if( !panel.hidden )panel.mousePressed(args.x,args.y,args.button);
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mousePressed(int x, int y, int button)
{
    if( !panel.hidden )panel.mousePressed(x, y, button);
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mouseDragged(ofMouseEventArgs & args)
{
   if( !panel.hidden )panel.mouseDragged(args.x,args.y,args.button);
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mouseDragged(int x, int y, int button)
{
   if( !panel.hidden )panel.mouseDragged(x, y, button);
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mouseReleased(ofMouseEventArgs & args)
{
  if( !panel.hidden ) panel.mouseReleased();
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mouseReleased(int x, int y, int button)
{
  if( !panel.hidden ) panel.mouseReleased();
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::setXMLFilename(string xmlFile)
{
    panel.setXMLFilename(xmlFile);
}

