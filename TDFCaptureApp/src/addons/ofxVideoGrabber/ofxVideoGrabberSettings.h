#ifndef _OFX_VIDEO_GRABBER_SETTINGS_H
#define _OFX_VIDEO_GRABBER_SETTINGS_H

#include "ofxControlPanel.h"

class ofxVideoGrabberFeature;
class ofxVideoGrabberSDK;

class ofxVideoGrabberSettings
{
    public:
        ofxVideoGrabberSettings();
        virtual ~ofxVideoGrabberSettings();

        virtual void setupVideoSettings(ofxVideoGrabberSDK* _videoGrabber);
        virtual void toggleSettingsGUI();
        virtual void mousePressed(ofMouseEventArgs & args);
        virtual void mouseDragged(ofMouseEventArgs & args);
        virtual void mouseReleased(ofMouseEventArgs & args);
		
		virtual void mouseDragged(int x, int y, int button);
		virtual void mousePressed(int x, int y, int button);	
		virtual void mouseReleased(int x, int y, int button);		

        virtual void enableGuiEvents();
		virtual void disableGuiEvents();
		
		virtual void setupGUI() = 0;
        virtual void update() = 0;
        virtual void draw() = 0;

        void setXMLFilename(string xmlFile);

        ofxControlPanel panel;
        ofxVideoGrabberSDK* videoGrabber;

    protected:
		bool bGuiEvents;


};

#endif // _OFX_VIDEO_GRABBER_SETTINGS_H
