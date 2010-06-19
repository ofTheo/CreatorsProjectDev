

#ifndef __LIBDC1394_GRABBER_FEATURE_HELPER_H
#define __LIBDC1394_GRABBER_FEATURE_HELPER_H



#include <dc1394/dc1394.h>

#include "../ofxVideoGrabberFeature.h"


class Libdc1394GrabberFeatureHelper
{

public:

	static dc1394feature_t AVidFeatureToLibdcFeature( int _avidFeatureEnum );
	static int libdcFeatureToAVidFeature( dc1394feature_t _libdcFeatureEnum );


};

#endif

