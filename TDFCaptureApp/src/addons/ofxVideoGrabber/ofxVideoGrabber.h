#ifndef __A_VIDEO_GRABBER_H
#define __A_VIDEO_GRABBER_H

#include "ofTypes.h"
#include "ofTexture.h"

#include "ofxVideoGrabberFeature.h"
#include "ofxVideoGrabberSettings.h"
#include "ofxVideoGrabberSDK.h"

/* SDKs */
#include "Libdc1394Grabber/Libdc1394Grabber.h"
#include "ofxIIDCSettings.h"


class ofxVideoGrabber: public ofBaseVideo
{
	public:

		ofxVideoGrabber();
		virtual ~ofxVideoGrabber();

		bool initGrabber( int _width = 320,
				  int _height = 240,
				  int _format = VID_FORMAT_RGB,
				  int _targetFormat = VID_FORMAT_RGB,
				  int _frameRate = 15,
				  bool _useTexture = true,
				  ofxVideoGrabberSDK* sdk = NULL,
				  ofxVideoGrabberSettings* settings = new ofxIIDCSettings);

		void 			listDevices();
		bool            isFrameNew();
		void            grabFrame();
        void            close();
		void			videoSettings() {settings->toggleSettingsGUI();};
		unsigned char*  getPixels();
		ofTexture &     getTextureReference();
		void 			setVerbose(bool bTalkToMe);
		void			setDeviceID(int _deviceID);
		void            setUseTexture(bool bUse);
		void            draw(float x, float y);
		void            draw(float x, float y, float w, float h);
        void            update();
		float 		    getHeight();
		float 		    getWidth();






	protected:
        ofxVideoGrabberSDK *        videoGrabber;
        ofxVideoGrabberSettings *   settings;

        bool bGrabberInited;
        bool bUseTexture;
        bool bVerbose;
        ofTexture tex;
        bool bIsFrameNew;
        int width;
        int height;
        unsigned int bpp;
        unsigned char* pixels;
        int targetFormat;
        int deviceID;

};


#endif
