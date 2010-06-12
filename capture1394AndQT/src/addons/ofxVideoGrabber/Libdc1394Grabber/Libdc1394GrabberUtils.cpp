#include "Libdc1394GrabberUtils.h"
#include "ofMain.h"

#include <iostream>
using namespace std;

/*-----------------------------------------------------------------------
 *  Prints the type of format to standard out
 *-----------------------------------------------------------------------*/
const char * Libdc1394GrabberUtils::print_format( uint32_t format )
{
#define print_case(A) case A: return(#A ""); break;

  switch( format ) {
    print_case(DC1394_VIDEO_MODE_160x120_YUV444);
    print_case(DC1394_VIDEO_MODE_320x240_YUV422);
    print_case(DC1394_VIDEO_MODE_640x480_YUV411);
    print_case(DC1394_VIDEO_MODE_640x480_YUV422);
    print_case(DC1394_VIDEO_MODE_640x480_RGB8);
    print_case(DC1394_VIDEO_MODE_640x480_MONO8);
    print_case(DC1394_VIDEO_MODE_640x480_MONO16);
    print_case(DC1394_VIDEO_MODE_800x600_YUV422);
    print_case(DC1394_VIDEO_MODE_800x600_RGB8);
    print_case(DC1394_VIDEO_MODE_800x600_MONO8);
    print_case(DC1394_VIDEO_MODE_1024x768_YUV422);
    print_case(DC1394_VIDEO_MODE_1024x768_RGB8);
    print_case(DC1394_VIDEO_MODE_1024x768_MONO8);
    print_case(DC1394_VIDEO_MODE_800x600_MONO16);
    print_case(DC1394_VIDEO_MODE_1024x768_MONO16);
    print_case(DC1394_VIDEO_MODE_1280x960_YUV422);
    print_case(DC1394_VIDEO_MODE_1280x960_RGB8);
    print_case(DC1394_VIDEO_MODE_1280x960_MONO8);
    print_case(DC1394_VIDEO_MODE_1600x1200_YUV422);
    print_case(DC1394_VIDEO_MODE_1600x1200_RGB8);
    print_case(DC1394_VIDEO_MODE_1600x1200_MONO8);
    print_case(DC1394_VIDEO_MODE_1280x960_MONO16);
    print_case(DC1394_VIDEO_MODE_1600x1200_MONO16);
    print_case(DC1394_VIDEO_MODE_EXIF);
    print_case(DC1394_VIDEO_MODE_FORMAT7_0);
    print_case(DC1394_VIDEO_MODE_FORMAT7_1);
    print_case(DC1394_VIDEO_MODE_FORMAT7_2);
    print_case(DC1394_VIDEO_MODE_FORMAT7_3);
    print_case(DC1394_VIDEO_MODE_FORMAT7_4);
    print_case(DC1394_VIDEO_MODE_FORMAT7_5);
    print_case(DC1394_VIDEO_MODE_FORMAT7_6);
    print_case(DC1394_VIDEO_MODE_FORMAT7_7);

  default:
    fprintf(stderr,"Unknown format\n");
  }

    return "";
}

/*-----------------------------------------------------------------------
 *  Returns the number of pixels in the image based upon the format
 *-----------------------------------------------------------------------*/
uint32_t Libdc1394GrabberUtils::get_num_pixels(dc1394camera_t *camera, uint32_t format ) {
  uint32_t w,h;

  dc1394_get_image_size_from_video_mode(camera, format,&w,&h);

  return w*h;
}

/*-----------------------------------------------------------------------
 *  Prints the type of color encoding
 *-----------------------------------------------------------------------*/
const char * Libdc1394GrabberUtils::print_color_coding( uint32_t color_id )
{
    static char buf[255];

    switch( color_id ) {
    case DC1394_COLOR_CODING_MONO8:
        sprintf(buf,"MONO8");
        break;
    case DC1394_COLOR_CODING_YUV411:
        sprintf(buf,"YUV411");
        break;
    case DC1394_COLOR_CODING_YUV422:
        sprintf(buf,"YUV422");
        break;
    case DC1394_COLOR_CODING_YUV444:
        sprintf(buf,"YUV444");
        break;
    case DC1394_COLOR_CODING_RGB8:
        sprintf(buf,"RGB8");
        break;
    case DC1394_COLOR_CODING_MONO16:
        sprintf(buf,"MONO16");
        break;
    case DC1394_COLOR_CODING_RGB16:
        sprintf(buf,"RGB16");
        break;
    case DC1394_COLOR_CODING_MONO16S:
        sprintf(buf,"MONO16S");
        break;
    case DC1394_COLOR_CODING_RGB16S:
        sprintf(buf,"RGB16S");
        break;
    case DC1394_COLOR_CODING_RAW8:
        sprintf(buf,"RAW8");
        break;
    case DC1394_COLOR_CODING_RAW16:
        sprintf(buf,"RAW16");
        break;

  default:
    sprintf(buf,"Unknown color coding (%d).",color_id);
  }

  return buf;
}


/*-----------------------------------------------------------------------
 *  Prints various information about the mode the camera is in
 *-----------------------------------------------------------------------*/
void Libdc1394GrabberUtils::print_mode_info( dc1394camera_t *camera , dc1394video_mode_t mode )
{


    ofLog(OF_LOG_NOTICE,"Mode: %s",print_format(mode));

	switch (mode) {
		case DC1394_VIDEO_MODE_FORMAT7_0:
		case DC1394_VIDEO_MODE_FORMAT7_1:
		case DC1394_VIDEO_MODE_FORMAT7_2:
		case DC1394_VIDEO_MODE_FORMAT7_3:
		case DC1394_VIDEO_MODE_FORMAT7_4:
		case DC1394_VIDEO_MODE_FORMAT7_5:
		case DC1394_VIDEO_MODE_FORMAT7_6:			
		case DC1394_VIDEO_MODE_FORMAT7_7:	
			return;
		default:
			break;
	};
	
    dc1394framerates_t framerates;
    if(dc1394_video_get_supported_framerates(camera,mode,&framerates) != DC1394_SUCCESS) {
        ofLog(OF_LOG_ERROR, "Can't get frame rates.");
    }

    for(unsigned int j = 0; j < framerates.num; j++ )
    {
        dc1394framerate_t rate = framerates.framerates[j];
        float f_rate;
        dc1394_framerate_as_float(rate,&f_rate);
        ofLog(OF_LOG_NOTICE,"  [%d] frame rate = %.2f",j,f_rate );
    }

}


dc1394error_t Libdc1394GrabberUtils::getBayerTile( dc1394camera_t* camera, dc1394color_filter_t* bayerPattern )
{

    uint32_t value;
    dc1394error_t err;

    // query register 0x1040
    // This register is an advanced PGR register called BAYER_TILE_MAPPING
    // For more information check the PGR IEEE-1394 Digital Camera Register Reference
    // err = GetCameraControlRegister( camera, 0x1040, &value );
    err = dc1394_get_register( camera, 0x1040, &value );

    if ( err != DC1394_SUCCESS )
    {
        return err;
    }

    // Ascii R = 52 G = 47 B = 42 Y = 59
    switch( value )
    {
        case 0x52474742:	// RGGB
            *bayerPattern = DC1394_COLOR_FILTER_RGGB;
            ofLog(OF_LOG_VERBOSE,"Using GGB Bayer pattern");
            break;
        case 0x47425247:	// GBRG
            *bayerPattern = DC1394_COLOR_FILTER_GBRG;
            ofLog(OF_LOG_VERBOSE,"Using GBRG Bayer pattern");
            break;
        case 0x47524247:	// GRBG
            *bayerPattern = DC1394_COLOR_FILTER_GRBG;
            ofLog(OF_LOG_VERBOSE,"Using GRBG Bayer pattern");
            break;
        case 0x42474752:	// BGGR
            *bayerPattern = DC1394_COLOR_FILTER_BGGR;
            ofLog(OF_LOG_VERBOSE, "Using BGGR Bayer pattern");
            break;
        case 0x59595959:	// YYYY
        default:
            ofLog(OF_LOG_VERBOSE,"Using default Bayer pattern.");
            break;
    }

    return err;
}
