#ifndef _OFX_IIDC_SETTINGS_H
#define _OFX_IIDC_SETTINGS_H

#include "ofxControlPanel.h"
#include "ofxVideoGrabberSettings.h"

class ofxVideoGrabberSDK;

class ofxIIDCSettings :  public ofxVideoGrabberSettings
{
    public:
        ofxIIDCSettings();
        virtual ~ofxIIDCSettings();

        virtual void setupGUI();
        virtual void update();
        virtual void draw();

    protected:
        void parameterCallback(float param1, float param2, int param_mode, int callback_id);
        ParameterCallback<ofxIIDCSettings> callback;


};

#endif
