#ifndef __LIB_DC1394_GRABBER_H
#define __LIB_DC1394_GRABBER_H


#include "ofConstants.h"
#include "ofAppRunner.h"
#include "ofxThread.h"

#include "dc1394/dc1394.h"

#include "Libdc1394GrabberFeatureHelper.h"
#include "ofxVideoGrabberFeature.h"
#include "ofxVideoGrabberSDK.h"


class Libdc1394Grabber : public ofxVideoGrabberSDK, public ofxThread
{
	public:


		Libdc1394Grabber();
		virtual ~Libdc1394Grabber();

		bool init(
            int _width = 320,
            int _height = 240,
            int _format = VID_FORMAT_GREYSCALE,
            int _targetFormat = VID_FORMAT_BGR ,
            int _frameRate = -1,
            bool _bVerbose = false,
            int _deviceID = -1);
        void close();
        void listDevices();

		unsigned char* getPixels();
		bool grabFrame(unsigned char ** _pixels);
		bool haveNewFrame();


        /* SDK specific functions */
        void setDiscardFrames(bool bDiscard);
        void setFormat7(enum VID_FORMAT7_MODES _format7_mode = VID_FORMAT7_0);
		void setROI(int x, int y, int width, int height);
		void set1394bMode(bool mode);
        void setDeviceID(string _deviceGUID);

		/* Features */
		void initFeatures();
		void setFeaturesOnePushMode();
		void getAllFeatureValues();
		void getFeatureValues( ofxVideoGrabberFeature* _feature );
		void setFeatureMode( int _val, int _feature );
		bool setFeatureOnOff( bool _val, int _feature);
		void setFeatureAbsoluteValue( float _val, int _feature );
		void setFeatureValue( float _val, int _feature );
        void setFeatureValue( float _val1, float _val2, int _feature );

		int stringToFeature( string _featureName );

        bool discardFrames;
		bool bSet1394bMode;

	private:

		bool initDc1394System();
		bool initCamera( int _width, int _height, dc1394video_mode_t _videoMode, dc1394framerate_t _frameRate, dc1394color_coding_t _coding );
		void threadedFunction();
		void captureFrame();
		void processCameraImageData( unsigned char* _cameraImageData );
        void setDeviceID(int _deviceID);
		void setBayerPatternIfNeeded();
		void initInternalBuffers();
		void enumerateDevices();
		void cleanupCamera();

        static dc1394_t* dc1394;

		dc1394camera_t* camera;
		dc1394video_frame_t* frame;
		dc1394speed_t speed;

		dc1394framerate_t framerate;
		dc1394video_mode_t video_mode;

		dc1394featureset_t features;
		dc1394framerates_t framerates;
		dc1394video_modes_t video_modes;

		dc1394color_coding_t coding;
        dc1394color_coding_t sourceFormatLibDC;

		dc1394speed_t ISO_SPEED;

		bool grabbedFirstImage;
		unsigned int tmpCount;

        /* Frame pixels */
		int finalImageDataBufferLength;
		int outputImageWidth;
		int outputImageHeight;

		/* Bayer patterns */
		bool conversionNeeded;
		dc1394color_filter_t bayerPattern;
		dc1394bayer_method_t bayerMethod;

		int YUV_BYTE_ORDER;

        /* video device handling */
        uint32_t numCameras;
        dc1394camera_list_t * cameraList;
        bool bChooseDevice;
        uint64_t cameraGUID;
        int cameraIndex;
        int cameraUnit;
		bool closeCamera;

        /* Format 7 and ROI */
        bool bUseFormat7;
        int ROI_x, ROI_y;
        int ROI_width, ROI_height;
        int packet_size;

};


#endif

