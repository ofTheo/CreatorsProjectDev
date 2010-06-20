#ifndef __LIBDC1394_GRABBER_FRAMERATE_HELPER_H
#define __LIBDC1394_GRABBER_FRAMERATE_HELPER_H


#include <dc1394/dc1394.h>

#include "../ofxVideoGrabberFeature.h"

class Libdc1394GrabberFramerateHelper
{

public:

	static dc1394framerate_t numToDcLibFramerate( int rateNum  );
	static const char* DcLibFramerateToString( dc1394framerate_t _framerate );

};

#endif
