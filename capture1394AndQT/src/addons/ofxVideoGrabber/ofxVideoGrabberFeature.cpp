

#include "ofxVideoGrabberFeature.h"

#include <string>
using namespace std;

int stringToVideoFormat( string _formatString )
{
	if( _formatString == "VID_FORMAT_GREYSCALE" )
	{
		return VID_FORMAT_GREYSCALE;
	}
	else if ( _formatString == "VID_FORMAT_BGR" )
	{
		return VID_FORMAT_BGR;
	}
	else if ( _formatString == "VID_FORMAT_YUV411" )
	{
		return VID_FORMAT_YUV411;
	}
	else if ( _formatString == "VID_FORMAT_YUV422" )
	{
		return VID_FORMAT_YUV422;
	}
	else if ( _formatString == "VID_FORMAT_YUV444" )
	{
		return VID_FORMAT_YUV444;
	}
	else if ( _formatString == "VID_FORMAT_RGB" )
	{
		return VID_FORMAT_RGB;
	}
	else if ( _formatString == "VID_FORMAT_BGR" )
	{
		return VID_FORMAT_BGR;
	}
	else if ( _formatString == "VID_FORMAT_Y8" )
	{
		return VID_FORMAT_Y8;
	}
	else if ( _formatString == "VID_FORMAT_Y16" )
	{
		return VID_FORMAT_Y16;
	}
	else if ( _formatString == "VID_FORMAT_BGRA" )
	{
		return VID_FORMAT_BGRA;
	}
	else if ( _formatString == "VID_FORMAT_ARGB" )
	{
		return VID_FORMAT_ARGB;
	}
	else if ( _formatString == "VID_FORMAT_ABGR" )
	{
		return VID_FORMAT_ABGR;
	}
	else if ( _formatString == "VID_FORMAT_RGBA" )
	{
		return VID_FORMAT_RGBA;
	}

	// what should we return here?
	return VID_FORMAT_YUV422;
}



string videoFormatToString( int _format )
{
	if( _format == VID_FORMAT_GREYSCALE )
	{
		return "VID_FORMAT_GREYSCALE";
	}
	else if ( _format == VID_FORMAT_BGR )
	{
		return "VID_FORMAT_BGR";
	}
	else if ( _format == VID_FORMAT_YUV411 )
	{
		return "VID_FORMAT_YUV411";
	}
	else if ( _format == VID_FORMAT_YUV422 )
	{
		return "VID_FORMAT_YUV422";
	}
	else if ( _format == VID_FORMAT_YUV444 )
	{
		return "VID_FORMAT_YUV444";
	}
	else if ( _format == VID_FORMAT_RGB )
	{
		return "VID_FORMAT_RGB";
	}
	else if ( _format == VID_FORMAT_BGR )
	{
		return "VID_FORMAT_BGR";
	}
	else if ( _format == VID_FORMAT_Y8 )
	{
		return "VID_FORMAT_Y8";
	}
	else if ( _format == VID_FORMAT_Y16 )
	{
		return "VID_FORMAT_Y16";
	}
	else if ( _format == VID_FORMAT_BGRA )
	{
		return "VID_FORMAT_BGRA";
	}
	else if ( _format == VID_FORMAT_ARGB )
	{
		return "VID_FORMAT_ARGB";
	}
	else if ( _format == VID_FORMAT_ABGR )
	{
		return "VID_FORMAT_ABGR";
	}
	else if ( _format == VID_FORMAT_RGBA )
	{
		return "VID_FORMAT_RGBA";
	}

	return "videoFormatToString:: unknown video format";
}

string cameraFeatureToTitle( int _feature )
{
	if( _feature == FEATURE_BRIGHTNESS )
	{
		return "Brightness";
	}
	else if ( _feature == FEATURE_EXPOSURE )
	{
		return "Exposure";
	}
	else if ( _feature == FEATURE_SHARPNESS )
	{
		return "Sharpness";
	}
	else if ( _feature == FEATURE_WHITE_BALANCE )
	{
		return "White Balance";
	}
	else if ( _feature == FEATURE_HUE )
	{
		return "Hue";
	}
	else if ( _feature == FEATURE_SATURATION )
	{
		return "Saturation";
	}
	else if ( _feature == FEATURE_GAMMA )
	{
		return "Gamma";
	}
	else if ( _feature == FEATURE_SHUTTER )
	{
		return "Shutter";
	}
	else if ( _feature == FEATURE_GAIN )
	{
		return "Gain";
	}
	else if ( _feature == FEATURE_IRIS )
	{
		return "Iris";
	}
	else if ( _feature == FEATURE_FOCUS )
	{
		return "Focus";
	}
	else if ( _feature == FEATURE_TEMPERATURE )
	{
		return "Temperature";
	}
	else if ( _feature == FEATURE_TRIGGER )
	{
		return "Trigger";
	}
	else if ( _feature == FEATURE_TRIGGER_DELAY )
	{
		return "Trigger Delay";
	}
	else if ( _feature == FEATURE_WHITE_SHADING )
	{
		return "White Shading";
	}
	else if ( _feature == FEATURE_FRAME_RATE )
	{
		return "Frame rate";
	}
	else if ( _feature == FEATURE_ZOOM )
	{
		return "Zoom";
	}
	else if ( _feature == FEATURE_PAN )
	{
		return "Pan";
	}
	else if ( _feature == FEATURE_TILT )
	{
		return "Tilt";
	}
	else if ( _feature == FEATURE_OPTICAL_FILTER )
	{
		return "Optical Filter";
	}
	else if ( _feature == FEATURE_CAPTURE_SIZE )
	{
		return "Capture Size";
	}
	else if ( _feature == FEATURE_CAPTURE_QUALITY )
	{
		return "Capture Quality";
	}

	return "cameraFeatureToTitle:: unknown camera feature";
}
