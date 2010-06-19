
#ifndef __LIBDC1394_GRABBER_UTILS_H
#define __LIBDC1394_GRABBER_UTILS_H


#include <dc1394/dc1394.h>

class Libdc1394GrabberUtils
{

public:

	static const char * print_format( uint32_t format );
	static uint32_t get_num_pixels(dc1394camera_t *camera, uint32_t format );
	static const char * print_color_coding( uint32_t color_id );
	static void print_mode_info( dc1394camera_t *camera , dc1394video_mode_t mode );
	static dc1394error_t getBayerTile( dc1394camera_t* camera, dc1394color_filter_t* bayerPattern );

};


#endif

