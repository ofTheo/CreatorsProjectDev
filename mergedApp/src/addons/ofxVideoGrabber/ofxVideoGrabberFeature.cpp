

#include "ofxVideoGrabberFeature.h"

#include <string>
using namespace std;

inline void spaceToUnderscore(std::string & myStr)
{
        /* replace all spaces in the name to satisfy XML*/
        size_t found;
        found=myStr.find_first_of(" ");
        while (found!=string::npos)
        {
            //myStr.erase(found,1);
            myStr[found]='_';

            found=myStr.find_first_of(" ",found+1);
        }
}


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

int titleToCameraFeature(string _feature )
{
	
	transform(_feature.begin(), _feature.end(), _feature.begin(), ::toupper);
	spaceToUnderscore(_feature);

	if( _feature == "BRIGHTNESS" )
	{
		return FEATURE_BRIGHTNESS;
	}
	else if ( _feature == "EXPOSURE" )
	{
		return FEATURE_EXPOSURE;
	}
	else if ( _feature == "SHARPNESS" )
	{
		return FEATURE_SHARPNESS;
	}
	else if ( _feature == "WHITE_BALANCE")
	{
		return FEATURE_WHITE_BALANCE;
	}
	else if ( _feature == "HUE" )
	{
		return FEATURE_HUE;
	}
	else if ( _feature ==  "SATURATION" )
	{
		return FEATURE_SATURATION;
	}
	else if ( _feature == "GAMMA" )
	{
		return FEATURE_GAMMA;
	}
	else if ( _feature == "SHUTTER" )
	{
		return FEATURE_SHUTTER;
	}
	else if ( _feature == "GAIN" )
	{
		return FEATURE_GAIN;
	}
	else if ( _feature == "IRIS" )
	{
		return FEATURE_IRIS;
	}
	else if ( _feature == "FOCUS" )
	{
		return FEATURE_FOCUS;
	}
	else if ( _feature == "TEMPERATURE" )
	{
		return FEATURE_TEMPERATURE;
	}
	else if ( _feature == "TRIGGER" )
	{
		return FEATURE_TRIGGER;
	}
	else if ( _feature == "TRIGGER_DELAY" )
	{
		return FEATURE_TRIGGER_DELAY;
	}
	else if ( _feature == "WHITE_SHADING" )
	{
		return FEATURE_WHITE_SHADING;
	}
	else if ( _feature == "FRAME_RATE" )
	{
		return FEATURE_FRAME_RATE;
	}
	else if ( _feature == "ZOOM" )
	{
		return FEATURE_ZOOM;
	}
	else if ( _feature == "PAN" )
	{
		return FEATURE_PAN;
	}
	else if ( _feature == "TILT" )
	{
		return FEATURE_TILT;
	}
	else if ( _feature == "OPTICAL_FILTER" )
	{
		return FEATURE_OPTICAL_FILTER;
	}
	else if ( _feature ==  "CAPTURE_SIZE" )
	{
		return FEATURE_CAPTURE_SIZE;
	}
	else if ( _feature == "CAPTURE_QUALITY" )
	{
		return FEATURE_CAPTURE_QUALITY;
	}

	return NULL_FEATURE;
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
