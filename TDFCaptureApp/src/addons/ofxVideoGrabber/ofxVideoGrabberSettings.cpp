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
		ofRemoveListener(ofEvents.mousePressed, this, &ofxVideoGrabberSettings::mousePressed);
		ofRemoveListener(ofEvents.mouseDragged, this, &ofxVideoGrabberSettings::mouseDragged);
		ofRemoveListener(ofEvents.mouseReleased, this, &ofxVideoGrabberSettings::mouseReleased);	
		bGuiEvents = true;
	}
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::setupVideoSettings(ofxVideoGrabberSDK* _videoGrabber)
{
    videoGrabber = _videoGrabber;
    enableGuiEvents();
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
void ofxVideoGrabberSettings::mouseDragged(ofMouseEventArgs & args)
{
   if( !panel.hidden )panel.mouseDragged(args.x,args.y,args.button);
}

//--------------------------------------------------------------------
void ofxVideoGrabberSettings::mouseReleased(ofMouseEventArgs & args)
{
  if( !panel.hidden ) panel.mouseReleased();
}

void ofxVideoGrabberSettings::setXMLFilename(string xmlFile)
{
    panel.setXMLFilename(xmlFile);
}

