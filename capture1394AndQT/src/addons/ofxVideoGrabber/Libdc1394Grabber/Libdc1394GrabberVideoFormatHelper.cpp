

#ifndef __LIB_DC1394_VIDEO_FORMAT_HELPER_
#define __LIB_DC1394_VIDEO_FORMAT_HELPER_

#include "Libdc1394GrabberVideoFormatHelper.h"


dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams( int _width, int _height, int _vidFormat, bool _forceColor )
{

	if( _width == 160 && _height == 120 )
	{
		return videoFormatFromParams160x120(  _vidFormat,  _forceColor );
	}
	else if ( _width == 320 && _height == 240 )
	{
		return videoFormatFromParams320x240( _vidFormat, _forceColor );
	}
	else if ( _width == 640 && _height == 480 )
	{
		return videoFormatFromParams640x480( _vidFormat, _forceColor );
	}
	else if ( _width == 800 && _height == 600 )
	{
		return videoFormatFromParams800x600( _vidFormat, _forceColor );
	}
	else if ( _width == 1024 && _height == 768 )
	{
		return videoFormatFromParams1024x768( _vidFormat, _forceColor );
	}
	else if ( _width == 1280 && _height == 960 )
	{
		return videoFormatFromParams1280x960( _vidFormat, _forceColor );
	}
	else if ( _width == 1600 && _height == 1200 )
	{
		return videoFormatFromParams1600x1200( _vidFormat, _forceColor );
	}

	return DC1394_VIDEO_MODE_640x480_MONO8;
}

dc1394color_coding_t  Libdc1394GrabberVideoFormatHelper::colorCodingFormat7FromParams( int _vidFormat, bool _forceColor ){
   if( _vidFormat == VID_FORMAT_RGB )				{ return DC1394_COLOR_CODING_RGB8; }
	else if( _vidFormat == VID_FORMAT_YUV411 )		{ return DC1394_COLOR_CODING_YUV411; }
	else if( _vidFormat == VID_FORMAT_YUV422 )		{ return DC1394_COLOR_CODING_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV444 )		{ return DC1394_COLOR_CODING_YUV422; }
	else if( _vidFormat == VID_FORMAT_GREYSCALE )	{ return DC1394_COLOR_CODING_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y8 )			{ return DC1394_COLOR_CODING_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y16 )			{ return DC1394_COLOR_CODING_MONO16; }

	return DC1394_COLOR_CODING_RGB8;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams160x120( int _vidFormat, bool _forceColor )
{
	return DC1394_VIDEO_MODE_160x120_YUV444;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams320x240( int _vidFormat, bool _forceColor )
{
	return DC1394_VIDEO_MODE_320x240_YUV422;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams640x480( int _vidFormat, bool _forceColor )
{
	if( _vidFormat == VID_FORMAT_RGB )				{ return DC1394_VIDEO_MODE_640x480_RGB8; }
	else if( _vidFormat == VID_FORMAT_YUV411 )		{ return DC1394_VIDEO_MODE_640x480_YUV411; }
	else if( _vidFormat == VID_FORMAT_YUV422 )		{ return DC1394_VIDEO_MODE_640x480_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV444 )		{ return DC1394_VIDEO_MODE_640x480_YUV422; }
	else if( _vidFormat == VID_FORMAT_GREYSCALE )	{ return DC1394_VIDEO_MODE_640x480_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y8 )			{ return DC1394_VIDEO_MODE_640x480_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y16 )			{ return DC1394_VIDEO_MODE_640x480_MONO16; }

	return DC1394_VIDEO_MODE_640x480_YUV422;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams800x600( int _vidFormat, bool _forceColor )
{
	if( _vidFormat == VID_FORMAT_RGB )				{ return DC1394_VIDEO_MODE_800x600_RGB8; }
	else if( _vidFormat == VID_FORMAT_YUV411 )		{ return DC1394_VIDEO_MODE_800x600_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV422 )		{ return DC1394_VIDEO_MODE_800x600_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV444 )		{ return DC1394_VIDEO_MODE_800x600_YUV422; }
	else if( _vidFormat == VID_FORMAT_GREYSCALE )	{ return DC1394_VIDEO_MODE_800x600_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y8 )			{ return DC1394_VIDEO_MODE_800x600_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y16 )			{ return DC1394_VIDEO_MODE_800x600_MONO16; }

	return DC1394_VIDEO_MODE_800x600_YUV422;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams1024x768( int _vidFormat, bool _forceColor )
{
	if( _vidFormat == VID_FORMAT_RGB )				{ return DC1394_VIDEO_MODE_1024x768_RGB8; }
	else if( _vidFormat == VID_FORMAT_YUV411 )		{ return DC1394_VIDEO_MODE_1024x768_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV422 )		{ return DC1394_VIDEO_MODE_1024x768_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV444 )		{ return DC1394_VIDEO_MODE_1024x768_YUV422; }
	else if( _vidFormat == VID_FORMAT_GREYSCALE )	{ return DC1394_VIDEO_MODE_1024x768_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y8 )			{ return DC1394_VIDEO_MODE_1024x768_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y16 )			{ return DC1394_VIDEO_MODE_1024x768_MONO16; }

	return DC1394_VIDEO_MODE_1024x768_YUV422;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams1280x960( int _vidFormat, bool _forceColor )
{
	if( _vidFormat == VID_FORMAT_RGB )				{ return DC1394_VIDEO_MODE_1280x960_RGB8; }
	else if( _vidFormat == VID_FORMAT_YUV411 )		{ return DC1394_VIDEO_MODE_1280x960_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV422 )		{ return DC1394_VIDEO_MODE_1280x960_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV444 )		{ return DC1394_VIDEO_MODE_1280x960_YUV422; }
	else if( _vidFormat == VID_FORMAT_GREYSCALE )	{ return DC1394_VIDEO_MODE_1280x960_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y8 )			{ return DC1394_VIDEO_MODE_1280x960_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y16 )			{ return DC1394_VIDEO_MODE_1280x960_MONO16; }

	return DC1394_VIDEO_MODE_1280x960_YUV422;
}

dc1394video_mode_t  Libdc1394GrabberVideoFormatHelper::videoFormatFromParams1600x1200( int _vidFormat, bool _forceColor )
{
	if( _vidFormat == VID_FORMAT_RGB )				{ return DC1394_VIDEO_MODE_1600x1200_RGB8; }
	else if( _vidFormat == VID_FORMAT_YUV411 )		{ return DC1394_VIDEO_MODE_1600x1200_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV422 )		{ return DC1394_VIDEO_MODE_1600x1200_YUV422; }
	else if( _vidFormat == VID_FORMAT_YUV444 )		{ return DC1394_VIDEO_MODE_1600x1200_YUV422; }
	else if( _vidFormat == VID_FORMAT_GREYSCALE )	{ return DC1394_VIDEO_MODE_1600x1200_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y8 )			{ return DC1394_VIDEO_MODE_1600x1200_MONO8;}
	else if( _vidFormat == VID_FORMAT_Y16 )			{ return DC1394_VIDEO_MODE_1600x1200_MONO16; }

	return DC1394_VIDEO_MODE_1600x1200_YUV422;
}


int Libdc1394GrabberVideoFormatHelper::libcd1394ColorFormatToVidFormat(  uint32_t _colorFormatID )
{
	switch( _colorFormatID )
	{
		case DC1394_COLOR_CODING_MONO8:
			return VID_FORMAT_GREYSCALE;
		break;

		case DC1394_COLOR_CODING_YUV411:
			return VID_FORMAT_YUV411;
		break;

		case DC1394_COLOR_CODING_YUV422:
			return VID_FORMAT_YUV422;
		break;

		case DC1394_COLOR_CODING_YUV444:
			return VID_FORMAT_YUV444;
		break;

		case DC1394_COLOR_CODING_RGB8:
			return VID_FORMAT_RGB;
		break;

		case DC1394_COLOR_CODING_MONO16:
			return VID_FORMAT_Y16;
		break;

		case DC1394_COLOR_CODING_RGB16:
			return VID_FORMAT_RGB; // we should handle this differently
		break;

		case DC1394_COLOR_CODING_MONO16S:
			return VID_FORMAT_Y16;
		break;

		case DC1394_COLOR_CODING_RGB16S:
			return VID_FORMAT_RGB; // we should handle this differently;
		break;

		case DC1394_COLOR_CODING_RAW8:
			return VID_FORMAT_Y8; // ?
		break;

		case DC1394_COLOR_CODING_RAW16:
			return VID_FORMAT_Y16; // ?
		break;

		default:
		fprintf(stderr,"Unknown color coding = %d\n", _colorFormatID );
	}

	return -1;
}

const char * Libdc1394GrabberVideoFormatHelper::libcd1394ISOFormatToString(dc1394speed_t _ISOspeed)
{
	switch (_ISOspeed)
	{
		case DC1394_ISO_SPEED_100:
			return "100";
			break;
		case DC1394_ISO_SPEED_200:
			return "200";
			break;
		case DC1394_ISO_SPEED_400:
			return "400";
			break;
		case DC1394_ISO_SPEED_800:
			return "800";
			break;
		case DC1394_ISO_SPEED_1600:
			return "1600";
			break;
		case DC1394_ISO_SPEED_3200:
			return "3200";
			break;
		default:
			return "";
	}
}

/*
	VID_FORMAT_GREYSCALE  =	0,
	VID_FORMAT_BGR		  =	1,

	VID_FORMAT_YUV411	  =	2,
	VID_FORMAT_YUV422	  = 3,
	VID_FORMAT_YUV444	  =	4,

	VID_FORMAT_RGB		  =	5,

	VID_FORMAT_Y8		  = 6,
	VID_FORMAT_Y16		  = 7
*/

#endif

