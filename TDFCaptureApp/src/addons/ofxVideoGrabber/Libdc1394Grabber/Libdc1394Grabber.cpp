#include "Libdc1394Grabber.h"
#include "Libdc1394GrabberUtils.h"
#include "Libdc1394GrabberFramerateHelper.h"
#include "Libdc1394GrabberVideoFormatHelper.h"


dc1394_t* Libdc1394Grabber::dc1394 = NULL;

Libdc1394Grabber::Libdc1394Grabber()
{

	conversionNeeded = false;
	grabbedFirstImage = false;

	YUV_BYTE_ORDER = DC1394_BYTE_ORDER_UYVY; //DC1394_BYTE_ORDER_UYVY, DC1394_BYTE_ORDER_YUYV
	ISO_SPEED = DC1394_ISO_SPEED_400;

	bayerMethod = DC1394_BAYER_METHOD_BILINEAR;
	bayerPattern = DC1394_COLOR_FILTER_RGGB;

	availableFeatureAmount = 0;

	discardFrames = false;
	numCameras = 0;

	camera = NULL;
	pixels = NULL;
	cameraList = NULL;

	ROI_x=0;
	ROI_y=0;
	ROI_width=0;
	ROI_height=0;
	packet_size = 0;

    bChooseDevice = false;
	bSet1394bMode = false;
	bUseFormat7 = false;
	cameraUnit = 0;
	cameraIndex = -1;
	cameraGUID = 0;
	closeCamera = false;
}

Libdc1394Grabber::~Libdc1394Grabber()
{

	if (camera != NULL )
	{
		ofLog(OF_LOG_NOTICE ,"Stopping ISO transmission.");
		// Stop data transmission
		if (dc1394_video_set_transmission(camera,DC1394_OFF)!=DC1394_SUCCESS)
		{
			ofLog(OF_LOG_ERROR, "Could not stop ISO transmission!");
		}
		// Close camera
		cleanupCamera();
	}

}


void Libdc1394Grabber::close()
{

	if (camera != NULL )
	{
		cleanupCamera();
	}
}

void Libdc1394Grabber::cleanupCamera()
{
	closeCamera = true;
	ofxThread::stopThread();
	ofLog(OF_LOG_VERBOSE,"Stopped capture thread.");

	//this sleep seems necessary, at least on OSX, to avoid an occasional hang on exit
	ofSleepMillis(150);

	dc1394switch_t is_iso_on = DC1394_OFF;
	if(camera) {
        if (dc1394_video_get_transmission(camera, &is_iso_on)!=DC1394_SUCCESS) {
            is_iso_on = DC1394_ON; // try to shut ISO anyway
        }
        if (is_iso_on > DC1394_OFF) {
            if (dc1394_video_set_transmission(camera, DC1394_OFF)!=DC1394_SUCCESS) {
                ofLog(OF_LOG_ERROR, "Could not stop ISO transmission!");
            }
        }
	}
	ofLog(OF_LOG_VERBOSE,"Stopped ISO transmission.");

	/* cleanup and exit */
	if(cameraList)
        dc1394_camera_free_list (cameraList);
    if(camera) {
        dc1394_capture_stop(camera);
        dc1394_camera_free (camera);
        camera = NULL;
	}
	ofLog(OF_LOG_VERBOSE,"Stopped camera.");

	if(dc1394) {
		dc1394_free (dc1394);
		dc1394 = NULL;
	}

	if(pixels) {
		delete [] pixels;
		pixels = NULL;
	}

	printf("Closed!\n");
}


bool Libdc1394Grabber::init( int _width, int _height, int _format, int _targetFormat, int _frameRate, bool _bVerbose, int _deviceID )
{
    ofLog(OF_LOG_VERBOSE, "Input format: %s   TargetFormat: %s",videoFormatToString(_format).c_str(), videoFormatToString(_targetFormat).c_str());

	targetFormat = _targetFormat;

	if(_deviceID != -1) {
        setDeviceID(_deviceID);
	}

    /* initialise camera */
    bool result = false;

    if(bUseFormat7){
        dc1394color_coding_t desiredColorCoding = Libdc1394GrabberVideoFormatHelper::colorCodingFormat7FromParams(_format);
        result = initCamera( _width,_height, DC1394_VIDEO_MODE_FORMAT7_0, (dc1394framerate_t) _frameRate, desiredColorCoding);
    }else{
        dc1394video_mode_t desiredVideoMode	= Libdc1394GrabberVideoFormatHelper::videoFormatFromParams( _width, _height, _format );
        dc1394framerate_t  desiredFrameRate = Libdc1394GrabberFramerateHelper::numToDcLibFramerate( _frameRate );
        result = initCamera( _width, _height, desiredVideoMode, desiredFrameRate, (dc1394color_coding_t) 0);
    }

    if(!result) {
		return false;
	}

	initInternalBuffers();
	startThread(true, false); //blocking, verbose

	return true;
}

bool Libdc1394Grabber::initDc1394System()
{
    /* Initialise libdc1394 */
    if(!dc1394) {
        dc1394 = dc1394_new ();
    }
    if (!dc1394) {
        ofLog(OF_LOG_FATAL_ERROR,"Failed to initialise libdc1394.");
        return false;
    }

    return true;
}

void Libdc1394Grabber::enumerateDevices()
{
    dc1394error_t err;

    if(!dc1394) {
        initDc1394System();
    }

    /* List cameras */
    if(cameraList == NULL) {
        err=dc1394_camera_enumerate (dc1394, &cameraList);
        DC1394_WRN(err,"Failed to enumerate cameras");
        if(err == DC1394_SUCCESS) {
            numCameras = cameraList->num;
        }
    }
}

void Libdc1394Grabber::listDevices()
{
    enumerateDevices();

    /* Verify that we have at least one camera */
    if (cameraList->num > 0) {
        ofLog(OF_LOG_NOTICE, "Listing available capture devices:");
        ofLog(OF_LOG_NOTICE,"(Use GUID as unique ID for camera)");
        ofLog(OF_LOG_NOTICE, "-------------------------------------------------------");
        for (uint32_t index = 0; index < cameraList->num; index++) {
            ofLog(OF_LOG_NOTICE, "Video device %d:  GUID = %llx , Unit = %x ", index, cameraList->ids[index].guid, cameraList->ids[index].unit);
        }
        ofLog(OF_LOG_NOTICE, "-------------------------------------------------------");
    }

    ofLog(OF_LOG_NOTICE,"There were %d cameras found.", numCameras );
}

void Libdc1394Grabber::setDiscardFrames(bool bDiscard)
{
    discardFrames = bDiscard;
}

void Libdc1394Grabber::set1394bMode(bool mode)
{
	bSet1394bMode = mode;
}

void Libdc1394Grabber::setDeviceID(int _deviceID){
	cameraIndex	= _deviceID;
	ofLog(OF_LOG_NOTICE, "Trying to use Camera with Index %u",cameraIndex);
	bChooseDevice	= true;
}

void Libdc1394Grabber::setDeviceID(string _deviceGUID)
{

    /* find camera unit  if present*/
    size_t pos = _deviceGUID.find(":");
    unsigned int len = _deviceGUID.length();

    if (pos == len - 2) {

        string unit = _deviceGUID.substr(len-1,1);
        std::istringstream ss(unit);
        ss >> cameraUnit;

        _deviceGUID = _deviceGUID.substr(0,len-2);
    }

    /* get camera GUID */
    std::istringstream ss( _deviceGUID );
    ss >> hex >> cameraGUID;

	ofLog(OF_LOG_NOTICE, "Trying to use Camera with GUID %llx, unit %i",cameraGUID, cameraUnit);
	bChooseDevice = true;
}

void Libdc1394Grabber::setFormat7(enum VID_FORMAT7_MODES _format7_mode)
{
    bUseFormat7 = true;

    switch(_format7_mode)
    {
        case VID_FORMAT7_0:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_0;
            break;
        case VID_FORMAT7_1:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_1;
            break;
        case VID_FORMAT7_2:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_2;
            break;
        case VID_FORMAT7_3:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_3;
            break;
        case VID_FORMAT7_4:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_4;
            break;
        case VID_FORMAT7_5:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_5;
            break;
        case VID_FORMAT7_6:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_6;
            break;
        case VID_FORMAT7_7:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_7;
            break;
        default:
            video_mode = DC1394_VIDEO_MODE_FORMAT7_0;
            break;
    }

}

void Libdc1394Grabber::setROI(int x, int y, int width, int height){
    ROI_x = x;
	ROI_y = y;
	ROI_width = width;
	ROI_height = height;
}

void Libdc1394Grabber::initInternalBuffers()
{
	if( targetFormat == VID_FORMAT_GREYSCALE || targetFormat == VID_FORMAT_Y8 )
        { bpp = VID_BPP_GREY; }
	else if( targetFormat == VID_FORMAT_RGB || targetFormat == VID_FORMAT_BGR )
        { bpp = VID_BPP_RGB; }
	else {
	    ofLog(OF_LOG_ERROR, " *** Libdc1394Grabber: Unsupported output format! ***");
    }

	finalImageDataBufferLength = width*height*bpp;
	pixels = new unsigned char[finalImageDataBufferLength];
	memset(pixels,0,finalImageDataBufferLength);

}
bool Libdc1394Grabber::initCamera( int _width, int _height, dc1394video_mode_t _videoMode, dc1394framerate_t _frameRate, dc1394color_coding_t _coding )
{
    dc1394error_t err;

    if(!initDc1394System()) {
        return false;
    }

    if(!cameraList) {
        enumerateDevices();
    }

    if(numCameras == 0) {
        return false;
    }

	/*-----------------------------------------------------------------------
	*  Select camera
	*-----------------------------------------------------------------------*/
    if( bChooseDevice ) {
		if(cameraGUID != 0){

			bool bFound = false;
			for (uint32_t index = 0; index < cameraList->num; index++) {
				if (cameraList->ids[index].guid == cameraGUID) {
				    if(cameraUnit >= 0) {
				        if(cameraUnit == cameraList->ids[index].unit) {
                            bFound = true;
                            cameraIndex = index;
                            ofLog(OF_LOG_NOTICE, "Found video device %i with GUID %llx, Unit %i",cameraIndex, cameraGUID, cameraUnit);
                            break;
				        }
				    } else {
                        bFound = true;
                        cameraIndex = index;
                        cameraUnit = cameraList->ids[index].unit;
                        ofLog(OF_LOG_NOTICE, "Found video device %i with GUID %llx",cameraIndex, cameraGUID);
                        break;
				    }

				}
			}

			if (!bFound) {
				ofLog(OF_LOG_WARNING,"Device Index for unit %u not found, using first device\n", cameraGUID);
				cameraUnit = -1;
				cameraGUID = cameraList->ids[0].guid;
			}
		}
		else if(cameraIndex != -1){
			bool bFound = false;

			for (uint32_t index = 0; index < cameraList->num; index++) {
				if ((int) index == cameraIndex) {
					bFound = true;
					cameraGUID = cameraList->ids[index].guid;
					ofLog(OF_LOG_NOTICE, "Found video device %i, it has GUID %llx", cameraIndex, cameraGUID);
					break;
				}
			}
			if (!bFound) {
				ofLog(OF_LOG_WARNING,"Video device %i not found, using first device\n", cameraIndex);
				cameraIndex = 0;
				cameraUnit = -1;
			}
		}
	} else if(cameraList->num > 0) {
        cameraGUID = cameraList->ids[0].guid;
        cameraIndex = 0;
    } else {
        ofLog(OF_LOG_ERROR, "No cameras found");
    }

	/*-----------------------------------------------------------------------
	*  Initialise camera
	*-----------------------------------------------------------------------*/
	if(cameraUnit == -1) {
        camera = dc1394_camera_new(dc1394, cameraGUID);
	}
    else {
        camera = dc1394_camera_new_unit(dc1394, cameraGUID, cameraUnit);
    }

    if (!camera) {
        dc1394_log_error("Failed to initialize camera with GUID %llx", cameraGUID);
        return false;
    }

	ofLog(OF_LOG_NOTICE, "Using video device %i with GUID %llx",cameraIndex,camera->guid);


    // These methods would cleanup the mess left behind by other processes,
    // but as of (libdc1394 2.0.0 rc9) this is not supported for the Juju stack
#ifdef TARGET_OSX
        dc1394_iso_release_bandwidth(camera, INT_MAX);
        for (int channel = 0; channel < 64; ++channel) {
            dc1394_iso_release_channel(camera, channel);
        }
#endif

#ifdef TARGET_LINUX
	// This is rude, but for now needed (Juju)...
	dc1394_reset_bus(camera);
#endif

	/* Select camera transfer mode */
    if ((camera->bmode_capable > 0) && (bSet1394bMode)) {
        err = dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
        speed = DC1394_ISO_SPEED_800;
        ofLog(OF_LOG_NOTICE,"1394B detected! Trying ISO Speed 800");
        if(err!=DC1394_SUCCESS){
            ofLog( OF_LOG_ERROR, "Failed to set ISO Speed 800");
        }
    } else {
        speed = DC1394_ISO_SPEED_400;
    }


	/*-----------------------------------------------------------------------
	*  Get video modes
	*-----------------------------------------------------------------------*/
	if (dc1394_video_get_supported_modes(camera,&video_modes) != DC1394_SUCCESS)
	{
	    ofLog(OF_LOG_FATAL_ERROR, "Can't get video modes");
	    cleanupCamera();
	    return false;
    }

    if(bVerbose) {
        ofLog(OF_LOG_VERBOSE, "Listing Modes");
        for(unsigned int i = 0; i < video_modes.num; i++ )
        {
            dc1394video_mode_t mode = video_modes.modes[i];
            Libdc1394GrabberUtils::print_mode_info( camera , mode );
        }
    }

    /* Search the list for our preferred video mode */
	bool foundVideoMode = false;
    for (int i = video_modes.num-1; i>=0; i--)
    {
        if( video_modes.modes[i] == _videoMode )
        {
            foundVideoMode = true;
            break;
        }
    }

	if(!bUseFormat7) {
	    if(foundVideoMode) {
            video_mode = _videoMode;
		} else {
		    ofLog(OF_LOG_WARNING, "Could not find desired video mode, using default");
            video_mode = video_modes.modes[0];
		}
	}


	/*-----------------------------------------------------------------------
	*  Get color coding
	*-----------------------------------------------------------------------*/
	if(bUseFormat7) {
        coding = _coding;
	}
	else {
        dc1394_get_color_coding_from_video_mode(camera, video_mode, &coding);
	}

	ofLog(OF_LOG_NOTICE,"Chosen color coding: %s",Libdc1394GrabberUtils::print_color_coding(coding));

	uint32_t bits = 0;
	dc1394_get_color_coding_data_depth(coding,&bits);
	ofLog(OF_LOG_VERBOSE, "Color coding data depth: %i bits",bits);
	uint32_t bit_size;
	dc1394_get_color_coding_bit_size(coding,&bit_size);
	ofLog(OF_LOG_VERBOSE, "Color coding bit-space: %i bits",bit_size);

	sourceFormatLibDC = coding;
	sourceFormat = Libdc1394GrabberVideoFormatHelper::libcd1394ColorFormatToVidFormat(  coding );


	/*-----------------------------------------------------------------------
	*  Get framerate / packet size
	*-----------------------------------------------------------------------*/

	if(bUseFormat7) {
        packet_size = DC1394_USE_MAX_AVAIL;
        if(ROI_height==0) ROI_height = _height;
        if(ROI_width==0)  ROI_width  = _width;
        if(_frameRate!=-1) {
            float bus_period;
			if(speed == DC1394_ISO_SPEED_800) {
				bus_period = 0.0000625;
			}
			else {
				bus_period = 0.000125;
			}

            int num_packets = (int)(1.0/(bus_period*_frameRate)+0.5);
            packet_size = ((ROI_width - ROI_x)*(ROI_height - ROI_y)*bit_size + (num_packets*8) - 1) / (num_packets*8);
        }
	}
	else {
        if (dc1394_video_get_supported_framerates(camera,video_mode,&framerates) != DC1394_SUCCESS)
        {
            ofLog(OF_LOG_FATAL_ERROR, "Can't get framerates");
            cleanupCamera();
        }

        // search the list for our preferred framerate
        bool foundFramerate = false;
        for(unsigned int i = 0; i < framerates.num; i++ )
        {
            if( framerates.framerates[i] == _frameRate )
            {
                framerate = framerates.framerates[i];
                foundFramerate = true;
                break;
            }
        }

        // if we didn't find it, select the highest one.
        if( !foundFramerate ) framerate=framerates.framerates[framerates.num-1];
	}

	/*-----------------------------------------------------------------------
	*  Setup capture
	*-----------------------------------------------------------------------*/
	ofLog(OF_LOG_VERBOSE,"Setting up capture.");

	ofLog(OF_LOG_NOTICE,"Setting ISO Speed %s",Libdc1394GrabberVideoFormatHelper::libcd1394ISOFormatToString(speed));
	err = dc1394_video_set_iso_speed(camera, speed);
	if(err!=DC1394_SUCCESS){
		ofLog( OF_LOG_ERROR, "Failed to set iso speed");
		return false;
	}

	ofLog(OF_LOG_NOTICE, "Setting video mode");
	err = dc1394_video_set_mode(camera, video_mode);
	if(err!=DC1394_SUCCESS){
	    ofLog( OF_LOG_ERROR, "Failed to set video mode");
        return false;
	}

    if(bUseFormat7) {
        ofLog(OF_LOG_NOTICE, "Setting Format 7 color coding");
        err = dc1394_format7_set_color_coding(camera, video_mode, coding);
        if(err!=DC1394_SUCCESS){
            ofLog( OF_LOG_ERROR, "Failed to set Format 7 color coding");
            return false;
        }

        ofLog(OF_LOG_NOTICE, "Setting framerate / packet size");
        err = dc1394_format7_set_packet_size(camera, video_mode, packet_size);
        if(err!=DC1394_SUCCESS){
            ofLog( OF_LOG_ERROR, "Failed to set framerate / packet size");
            return false;
        }

        ofLog(OF_LOG_NOTICE, "Setting ROI");
        err = dc1394_format7_set_roi(camera, video_mode, coding, packet_size, ROI_x,ROI_y,ROI_width,ROI_height);
        if(err!=DC1394_SUCCESS){
            ofLog( OF_LOG_ERROR, "Failed to set ROI");
            return false;
        }
    } else {
        dc1394_video_set_framerate(camera, framerate);
    }



	if (dc1394_capture_setup(camera,4,DC1394_CAPTURE_FLAGS_DEFAULT)!=DC1394_SUCCESS)
	{
		ofLog(OF_LOG_FATAL_ERROR,"Unable to setup camera!\n\t - check that the video mode and framerate are supported by your camera.");
		cleanupCamera();
		return false;
	}

	ofLog(OF_LOG_NOTICE,"Chosen video format\t: %s",Libdc1394GrabberUtils::print_format( video_mode ));
	if(!bUseFormat7) {
        ofLog(OF_LOG_NOTICE,"Chosen frame rate\t: %s",Libdc1394GrabberFramerateHelper::DcLibFramerateToString(framerate));
	}


	/*-----------------------------------------------------------------------
	*  Start camera sending data
	*-----------------------------------------------------------------------*/
	if (dc1394_video_set_transmission(camera, DC1394_ON) !=DC1394_SUCCESS)
	{
		ofLog(OF_LOG_FATAL_ERROR, "Unable to start camera iso transmission");
		cleanupCamera();
	}


	/*-----------------------------------------------------------------------
	*  Sleep until the camera has a transmission
	*-----------------------------------------------------------------------*/
	dc1394switch_t status = DC1394_OFF;

	int counter = 0;
	while( status == DC1394_OFF && counter++ < 5 )
	{
		usleep(50000);
		if (dc1394_video_get_transmission(camera, &status)!=DC1394_SUCCESS)
		{
			ofLog(OF_LOG_FATAL_ERROR,"Unable to get transmission status.");
			cleanupCamera();
		}
	}

	if( counter == 5 )
	{
		ofLog(OF_LOG_FATAL_ERROR,"Camera doesn't seem to want to turn on!");
		cleanupCamera();
	}

    if(bUseFormat7) {
        dc1394_format7_get_image_size(camera, video_mode, &width, &height);
    }
    else {
        dc1394_get_image_size_from_video_mode(camera, video_mode, &width, &height);
    }
    outputImageWidth = width;
    outputImageHeight = height;

	/*-----------------------------------------------------------------------
	*  Initialized and list features
	*-----------------------------------------------------------------------*/
	initFeatures();

	return true;
}

void Libdc1394Grabber::threadedFunction()
{
    while( 1 )
    {
		if( !threadRunning ) return;	
        captureFrame();
        ofSleepMillis(2);
    }
    return;
}

unsigned char* Libdc1394Grabber::getPixels()
{
    return pixels;
}


bool Libdc1394Grabber::grabFrame(unsigned char ** _pixels)
{
    lock();
    if(bHasNewFrame)
    {
        bHasNewFrame = false;
        memcpy(*_pixels,pixels,width*height*bpp);
        unlock();
        return true;
    }
    else {
    	unlock();
    	return false;
    }
}

void Libdc1394Grabber::captureFrame()
{
    lock();
	if( !bHasNewFrame && (camera != NULL ))
	{
	    unlock();
	    if(discardFrames)
	    {
            /*---------------------------------------------------------------------------
             *  make sure DMA buffer is fresh by dropping frames if we're lagging behind
             *--------------------------------------------------------------------------*/

            bool bufferEmpty = false;
            dc1394video_frame_t* frameToDiscard;
            while (!bufferEmpty){
                if (dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frameToDiscard) == DC1394_SUCCESS){
                    if(frameToDiscard != NULL){
                        dc1394_capture_enqueue(camera, frameToDiscard);
                        ofLog(OF_LOG_WARNING, "discarded a frame");
                    } else {
                        bufferEmpty = true;
                    }
                } else {
                    bufferEmpty = true;
                }
            }
	    }

		/*-----------------------------------------------------------------------
		*  capture one frame
		*-----------------------------------------------------------------------*/
		if (dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame) != DC1394_SUCCESS)
		{
			fprintf(stderr, "unable to capture a frame\n");
			cleanupCamera();
		}

		if( !grabbedFirstImage )
		{
			setBayerPatternIfNeeded();
			grabbedFirstImage = true;
		}

		processCameraImageData( frame->image );

		dc1394_capture_enqueue(camera, frame);

        lock();
		bHasNewFrame = true;
		unlock();
	} else {
        unlock();
	}

}

static void rgb2bgr(const unsigned char *src, unsigned char *dest, unsigned long long int NumPixels)
{
	register int i = NumPixels - 1;
	register int j = NumPixels - 1;
	while (i > 0) {
		dest[j--] = src[i--];
		dest[j--] = src[i--];
		dest[j--] = src[i--];
	}
}

void Libdc1394Grabber::processCameraImageData( unsigned char* _cameraImageData )
{

	if( sourceFormatLibDC == DC1394_COLOR_CODING_RAW8 || sourceFormatLibDC == DC1394_COLOR_CODING_MONO8 )
	{

		if( targetFormat == VID_FORMAT_GREYSCALE || targetFormat == VID_FORMAT_Y8 )
		{
            lock();
			memcpy(pixels, _cameraImageData, width * height * bpp);
			unlock();
		}
		else if( targetFormat == VID_FORMAT_RGB )
		{
		    lock();
			dc1394_bayer_decoding_8bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod );
			unlock();
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
		    lock();
			dc1394_bayer_decoding_8bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod ); // we should really be converting this
			unlock();
		}
		else
		{
			ofLog(OF_LOG_ERROR, "Unsupported target format %s from DC1394_COLOR_CODING_RAW8 or DC1394_COLOR_CODING_MONO8 ",videoFormatToString( targetFormat ).c_str());
		}

	}
	else if(  sourceFormatLibDC == DC1394_COLOR_CODING_MONO16 || sourceFormatLibDC == DC1394_COLOR_CODING_RAW16 )
	{
	    // These are not implemented yet....
		if( targetFormat == VID_FORMAT_RGB)
		{
		    ofLog(OF_LOG_ERROR, "Unsupported target format VID_FORMAT_RGB from DC1394_COLOR_CODING_MONO16 or DC1394_COLOR_CODING_RAW16 ");
		    lock();
			//dc1394_bayer_decoding_16bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod, 16 );
			dc1394_convert_to_RGB8(_cameraImageData, pixels, width, height, 0, sourceFormatLibDC, 16);
			unlock();
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
		    ofLog(OF_LOG_ERROR, "Unsupported target format VID_FORMAT_BGR from DC1394_COLOR_CODING_MONO16 or DC1394_COLOR_CODING_RAW16 ");
		    //lock();
			//dc1394_bayer_decoding_16bit( _cameraImageData, pixels, width, height,  bayerPattern, bayerMethod ); // we should really be converting this
            //unlock();
		}
		else if( targetFormat == VID_FORMAT_GREYSCALE)
		{
		    lock();
			dc1394_convert_to_MONO8(_cameraImageData, pixels, width, height, 0, sourceFormatLibDC, 16);
			unlock();
		}
		else
		{
		    ofLog(OF_LOG_ERROR, "Unsupported target format %s from DC1394_COLOR_CODING_MONO16 or DC1394_COLOR_CODING_RAW16 ",videoFormatToString( targetFormat ).c_str());
		}
	}
	else if(  sourceFormatLibDC == DC1394_COLOR_CODING_YUV411 || sourceFormatLibDC == DC1394_COLOR_CODING_YUV422 || sourceFormatLibDC == DC1394_COLOR_CODING_YUV444 )
	{
		if( targetFormat == VID_FORMAT_RGB )
		{
		    lock();
			dc1394_convert_to_RGB8( _cameraImageData, pixels, width, height, YUV_BYTE_ORDER, sourceFormatLibDC, 16);
			unlock();
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
		    lock();
			dc1394_convert_to_RGB8( _cameraImageData, pixels, width, height, YUV_BYTE_ORDER, sourceFormatLibDC, 16); // we should really be converting this
			unlock();
		}
		else
		{
		    ofLog(OF_LOG_ERROR, "Unsupported target format %s from DC1394_COLOR_CODING_YUV411, DC1394_COLOR_CODING_YUV422 or DC1394_COLOR_CODING_YUV444 ",videoFormatToString( targetFormat ).c_str());
		}

	}
	else if(  sourceFormatLibDC == DC1394_COLOR_CODING_RGB8 )
	{
		if( targetFormat == VID_FORMAT_RGB )
		{
            lock();
			memcpy(pixels, _cameraImageData, width * height * bpp);
			unlock();
		}
		else if ( targetFormat == VID_FORMAT_BGR )
		{
            lock();
			memcpy(pixels, _cameraImageData, width * height * bpp);
			unlock();
		}
		else
		{
            ofLog(OF_LOG_ERROR, "Unsupported target format %s from DC1394_COLOR_CODING_RGB8",videoFormatToString( targetFormat ).c_str());
		}
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Libdc1394Grabber::processCameraImageData() : Unsupported source format!");
	}

}

void Libdc1394Grabber::setBayerPatternIfNeeded()
{
	if( sourceFormatLibDC == DC1394_COLOR_CODING_RAW8 || sourceFormatLibDC == DC1394_COLOR_CODING_MONO8 || sourceFormatLibDC == DC1394_COLOR_CODING_MONO16 || sourceFormatLibDC == DC1394_COLOR_CODING_RAW16 )
	{
		if( targetFormat == VID_FORMAT_RGB || targetFormat == VID_FORMAT_BGR || targetFormat == VID_FORMAT_GREYSCALE )
		{

//		if( dc1394_format7_get_color_filter(camera, video_mode, &bayerPattern) != DC1394_SUCCESS )
//		{
//			ofLog(OF_LOG_ERROR, "Libdc1394Grabber::setBayerPatternIfNeeded(), Failed to get the dc1394_format7_get_color_filter.");
//		}
//		else
//		{
//			ofLog(OF_LOG_ERROR, "Libdc1394Grabber::setBayerPatternIfNeeded(), We got a pattern, it was: %i", bayerPattern);
//		}


			if ( Libdc1394GrabberUtils::getBayerTile( camera, &bayerPattern ) != DC1394_SUCCESS )
			{
				ofLog(OF_LOG_WARNING,"Could not get bayer tile pattern from camera" );
			} else {
			    ofLog(OF_LOG_WARNING,"Grabbed a bayer pattern from the camera");
            }
		}

	}
}


/*-----------------------------------------------------------------------
 *  Feature Methods
 *-----------------------------------------------------------------------*/
void Libdc1394Grabber::initFeatures()
{

	dc1394featureset_t tmpFeatureSet;

	// get camera features ----------------------------------
	if ( dc1394_feature_get_all(camera,&tmpFeatureSet) !=DC1394_SUCCESS)
	{
        ofLog(OF_LOG_ERROR, "Unable to get camera feature set.");
    }
	else
	{

		int tmpAvailableFeatures = 0;
		for( int i = 0; i < TOTAL_CAMERA_FEATURES; i++ )
		{
			if( tmpFeatureSet.feature[i].available ) { tmpAvailableFeatures++; }
		}

		availableFeatureAmount = tmpAvailableFeatures;

		featureVals = new ofxVideoGrabberFeature[availableFeatureAmount];

		tmpAvailableFeatures = 0;

        if(bVerbose) {
            cout << "------------------------------------------------------------" << endl;
            cout << "Feature Name\tAbs.\tCurr.\tCurr2.\tMin.\tMax." << endl;
        }
		for( int i = 0; i < TOTAL_CAMERA_FEATURES; i++ )
		{
			if(tmpFeatureSet.feature[i].available)
			{
				featureVals[tmpAvailableFeatures].feature			= Libdc1394GrabberFeatureHelper::libdcFeatureToAVidFeature( tmpFeatureSet.feature[i].id );
				featureVals[tmpAvailableFeatures].name				= cameraFeatureToTitle( featureVals[tmpAvailableFeatures].feature );

				featureVals[tmpAvailableFeatures].isPresent			= tmpFeatureSet.feature[i].available;
				featureVals[tmpAvailableFeatures].isReadable		= tmpFeatureSet.feature[i].readout_capable;

				featureVals[tmpAvailableFeatures].hasAbsoluteMode	= tmpFeatureSet.feature[i].absolute_capable;
				featureVals[tmpAvailableFeatures].absoluteModeActive = tmpFeatureSet.feature[i].abs_control;

                for(unsigned int j = 0; j < tmpFeatureSet.feature[i].modes.num; j++)
                {
                    if(tmpFeatureSet.feature[i].modes.modes[j] == DC1394_FEATURE_MODE_MANUAL)
                    {
                        featureVals[tmpAvailableFeatures].hasManualMode	= true;
                    }
                    else if(tmpFeatureSet.feature[i].modes.modes[j] == DC1394_FEATURE_MODE_AUTO)
                    {
                        featureVals[tmpAvailableFeatures].hasAutoMode = true;
                    }
                    else if(tmpFeatureSet.feature[i].modes.modes[j] == DC1394_FEATURE_MODE_ONE_PUSH_AUTO)
                    {
                        featureVals[tmpAvailableFeatures].hasOnePush = true;
                    }
                }

				featureVals[tmpAvailableFeatures].isOnOffSwitchable	= tmpFeatureSet.feature[i].on_off_capable;
				featureVals[tmpAvailableFeatures].isOn				= tmpFeatureSet.feature[i].is_on;

                if(tmpFeatureSet.feature[i].current_mode == DC1394_FEATURE_MODE_MANUAL)
                {
                    featureVals[tmpAvailableFeatures].hasManualActive	= true;
                }
                else if(tmpFeatureSet.feature[i].current_mode == DC1394_FEATURE_MODE_AUTO)
                {
                    featureVals[tmpAvailableFeatures].hasAutoModeActive	= true;
                }
                else if(tmpFeatureSet.feature[i].current_mode == DC1394_FEATURE_MODE_ONE_PUSH_AUTO)
                {
                    featureVals[tmpAvailableFeatures].hasOnePushActive = true;
                }


				if( featureVals[tmpAvailableFeatures].hasAbsoluteMode )
				{
					dc1394_feature_set_absolute_control( camera, tmpFeatureSet.feature[i].id, DC1394_OFF );

/* Not using Abs values (yet?) */
//					featureVals[tmpAvailableFeatures].currVal = tmpFeatureSet.feature[i].abs_value;
//
//					featureVals[tmpAvailableFeatures].minVal  = tmpFeatureSet.feature[i].abs_min;
//					featureVals[tmpAvailableFeatures].maxVal  = tmpFeatureSet.feature[i].abs_max;

				}
				else
				{
                    if(tmpFeatureSet.feature[i].id == DC1394_FEATURE_WHITE_BALANCE)
                    {
                        featureVals[tmpAvailableFeatures].currVal = (float)tmpFeatureSet.feature[i].BU_value;
                        featureVals[tmpAvailableFeatures].currVal2 = (float)tmpFeatureSet.feature[i].RV_value;
                        featureVals[tmpAvailableFeatures].minVal  = (float)tmpFeatureSet.feature[i].min;
                        featureVals[tmpAvailableFeatures].maxVal  = (float)tmpFeatureSet.feature[i].max;
                    }
                    else {
                        featureVals[tmpAvailableFeatures].currVal = (float)tmpFeatureSet.feature[i].value;
                        featureVals[tmpAvailableFeatures].minVal  = (float)tmpFeatureSet.feature[i].min;
                        featureVals[tmpAvailableFeatures].maxVal  = (float)tmpFeatureSet.feature[i].max;
                    }
				}

                if(bVerbose) {
                    /* print out feature values */
                    cout << setw(13) << featureVals[tmpAvailableFeatures].name
                    << " :\t" << featureVals[tmpAvailableFeatures].hasAbsoluteMode
                    << "\t" << featureVals[tmpAvailableFeatures].currVal;

                    if(tmpFeatureSet.feature[i].id == DC1394_FEATURE_WHITE_BALANCE)
                    {
                        cout << "\t" << featureVals[tmpAvailableFeatures].currVal2;
                    }
                    else
                    {
                        cout << "\t";
                    }

                    cout << "\t" << featureVals[tmpAvailableFeatures].minVal
                    << "\t" << featureVals[tmpAvailableFeatures].maxVal << endl;
                }

				tmpAvailableFeatures++;
			}
		}
		if(bVerbose) {
            cout << "------------------------------------------------------------" << endl;
		}

	}

}

void Libdc1394Grabber::getAllFeatureValues()
{

	if( availableFeatureAmount == 0 ) return;

	for( int i = 0; i < availableFeatureAmount; i++ )
	{
		if(featureVals[i].isPresent)
		{
			getFeatureValues( &featureVals[i] );
		}
	}

}


void Libdc1394Grabber::getFeatureValues( ofxVideoGrabberFeature* _feature )
{

	dc1394feature_info_t tmpFeatureVals;
	tmpFeatureVals.id = Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature->feature );

	dc1394_feature_get( camera, &tmpFeatureVals);

/* Not using Abs values (yet?) */
//	if( _feature->hasAbsoluteMode )
//	{
//		_feature->currVal = tmpFeatureVals.abs_value;
//		_feature->minVal = tmpFeatureVals.abs_min;
//		_feature->maxVal = tmpFeatureVals.abs_max;
//	}
//	else
	{
        if(tmpFeatureVals.id == DC1394_FEATURE_WHITE_BALANCE)
        {
            _feature->currVal = (float)tmpFeatureVals.BU_value;
            _feature->currVal2 = (float)tmpFeatureVals.RV_value;
            _feature->minVal = (float)tmpFeatureVals.min;
            _feature->maxVal = (float)tmpFeatureVals.max;
        }
        else {
            _feature->currVal = (float)tmpFeatureVals.value;
            _feature->minVal = (float)tmpFeatureVals.min;
            _feature->maxVal = (float)tmpFeatureVals.max;
        }
	}

	if( _feature->isOnOffSwitchable )
	{
		_feature->isOn = tmpFeatureVals.is_on;
	}

	if( _feature->hasManualMode )
	{
        if(tmpFeatureVals.current_mode == DC1394_FEATURE_MODE_MANUAL)
        {
            _feature->hasManualActive = true;
        }
        else
        {
            _feature->hasManualActive = false;
        }
	}

	if( _feature->hasAutoMode )
	{
        if(tmpFeatureVals.current_mode == DC1394_FEATURE_MODE_AUTO)
        {
            _feature->hasAutoModeActive = true;
        }
        else
        {
            _feature->hasAutoModeActive = false;
        }
	}

    if( _feature->hasOnePush )
	{
        if(tmpFeatureVals.current_mode == DC1394_FEATURE_MODE_ONE_PUSH_AUTO)
        {
            _feature->hasOnePushActive = true;
        }
        else
        {
            _feature->hasOnePushActive = false;
        }
	}

}

void Libdc1394Grabber::setFeatureAbsoluteValue( float _val, int _feature )
{
	dc1394error_t err;

	//cout << " Libdc1394Grabber::setFeatureAbsoluteVal " << _val << ", " << _feature << endl;

	err = dc1394_feature_set_absolute_value( camera, Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature ), _val );
	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		//cout << dc1394_error_strings[err] << endl;
		cout << "Libdc1394Grabber::setFeatureAbsoluteVal, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
}

void Libdc1394Grabber::setFeatureValue( float _val1, float _val2, int _feature)
{
	dc1394error_t err = DC1394_FAILURE;

    if(_feature == FEATURE_WHITE_BALANCE) {
        // _val = u_b, _val2 = v_r for whitebalance
        err = dc1394_feature_whitebalance_set_value( camera, (unsigned int) _val1, (unsigned int) _val2 );
    }
	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureVal, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
}

void Libdc1394Grabber::setFeatureValue( float _val, int _feature )
{
	dc1394error_t err;

	//cout << " Libdc1  394Grabber::setFeatureVal " << _val << ", " << _feature << endl;

	err = dc1394_feature_set_value( camera, Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature ), (unsigned int)_val );


	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureVal, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}

}

//void Libdc1394Grabber::getFeatureModes()
//{
//    dc1394error_t err;
//    err = dc1394_feature_has_auto_mode(camera, DC1394_FEATURE_MODE_MANUAL, &value);
//    if( (err == DC1394_SUCCESS) && value )
//    {
//
//    }
//}

void Libdc1394Grabber::setFeatureMode(int _featureMode, int _feature )
{
    dc1394error_t err = DC1394_FAILURE;
    dc1394feature_t tmpFeature = Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature );
    int index = -1;

	for( int i = 0; i < availableFeatureAmount; i++ )
	{
		if(featureVals[i].feature == _feature)
		{
                index = i;
		}
	}

	if (_featureMode == FEATURE_MODE_MANUAL)
	{
	    if(featureVals[index].isOnOffSwitchable) {
            setFeatureOnOff(true,_feature);
	    }
	    err = dc1394_feature_set_mode( camera, tmpFeature, DC1394_FEATURE_MODE_MANUAL );
//	    if(err == DC1394_SUCCESS) {
//            err = dc1394_feature_set_absolute_control( camera, tmpFeature, DC1394_OFF );
//	    }
	}
	else if(_featureMode == FEATURE_MODE_AUTO)
	{
	    if(featureVals[index].isOnOffSwitchable) {
            setFeatureOnOff(true,_feature);
	    }
	    err = dc1394_feature_set_mode( camera, tmpFeature, DC1394_FEATURE_MODE_AUTO );
//	    if(err == DC1394_SUCCESS) {
//            err = dc1394_feature_set_absolute_control( camera, tmpFeature, DC1394_OFF );
//	    }
	}
	else if(_featureMode == FEATURE_MODE_ONE_PUSH_AUTO)
	{
	    if(featureVals[index].isOnOffSwitchable) {
            setFeatureOnOff(true,_feature);
	    }
	    err = dc1394_feature_set_mode( camera, tmpFeature, DC1394_FEATURE_MODE_ONE_PUSH_AUTO );
//	    if(err == DC1394_SUCCESS) {
//            err = dc1394_feature_set_absolute_control( camera, tmpFeature, DC1394_OFF );
//	    }
	}

    if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureMode, failed to set mode for feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
}

bool Libdc1394Grabber::setFeatureOnOff( bool _val, int _feature )
{
	dc1394error_t err = DC1394_FAILURE;
	dc1394feature_t tmpFeature = Libdc1394GrabberFeatureHelper::AVidFeatureToLibdcFeature( _feature );

    if( _val ) {
        err = dc1394_feature_set_power( camera, tmpFeature, DC1394_ON );
    }
    else {
        err = dc1394_feature_set_power( camera, tmpFeature, DC1394_OFF );
    }
    return true;

	if( err != DC1394_SUCCESS )
	{
		cout << "*******************" << endl;
		cout << "Libdc1394Grabber::setFeatureStateOnOff, failed to set feature: " << cameraFeatureToTitle( _feature ) << endl;
		cout << "*******************" << endl;
	}
	return false;
}



void Libdc1394Grabber::setFeaturesOnePushMode()
{
	setFeatureMode( FEATURE_SHUTTER, FEATURE_MODE_MANUAL );
	dc1394_feature_set_mode( camera, DC1394_FEATURE_SHUTTER,	DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	//dc1394_feature_set_mode( camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	setFeatureMode( FEATURE_GAIN, FEATURE_MODE_MANUAL );
	dc1394_feature_set_mode( camera, DC1394_FEATURE_GAIN,		DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	//dc1394_feature_set_mode( camera, DC1394_FEATURE_IRIS,		DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	setFeatureMode( FEATURE_EXPOSURE, FEATURE_MODE_MANUAL );
	dc1394_feature_set_mode( camera, DC1394_FEATURE_EXPOSURE,	DC1394_FEATURE_MODE_ONE_PUSH_AUTO );

	//dc1394_feature_set_mode( camera, DC1394_FEATURE_BRIGHTNESS, DC1394_FEATURE_MODE_ONE_PUSH_AUTO );
}

int Libdc1394Grabber::stringToFeature( string _featureName )
{
	return -1;
}