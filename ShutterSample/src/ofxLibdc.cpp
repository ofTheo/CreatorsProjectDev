#include "ofxLibdc.h"

dc1394_t* ofxLibdc::libdcContext = NULL;
int ofxLibdc::libdcCameras = 0;

ofxLibdc::ofxLibdc() :
		camera(NULL),
		imageType(OF_IMAGE_COLOR) {
	startLibdcContext();
}

ofxLibdc::~ofxLibdc() {
	if(camera != NULL) {
		dc1394_capture_stop(camera);
		dc1394_camera_free(camera);
	}
	stopLibdcContext();
}

void ofxLibdc::startLibdcContext() {
	if(libdcCameras == 0)
		libdcContext = dc1394_new();
	libdcCameras++;
}

void ofxLibdc::stopLibdcContext() {
	libdcCameras--;
	if(libdcCameras == 0)
		dc1394_free(libdcContext);
}

int ofxLibdc::getOfImageType(dc1394color_coding_t libdcType) {
	switch(libdcType) {
		case DC1394_COLOR_CODING_MONO8: return OF_IMAGE_GRAYSCALE;
		case DC1394_COLOR_CODING_RGB8: return OF_IMAGE_COLOR;
		default: return OF_IMAGE_COLOR;
	}
}

dc1394color_coding_t ofxLibdc::getLibdcType(int ofImageType) {
	switch(ofImageType) {
		case OF_IMAGE_GRAYSCALE: return DC1394_COLOR_CODING_MONO8;
		case OF_IMAGE_COLOR: return DC1394_COLOR_CODING_RGB8;
		default: return DC1394_COLOR_CODING_RGB8;
	}
}

int ofxLibdc::getCameraCount() {
	dc1394camera_list_t* list;
	dc1394_camera_enumerate (libdcContext, &list);
	int cameraCount = list->num;
	dc1394_camera_free_list (list);
	return cameraCount;
}

void ofxLibdc::setup(int cameraNumber) {	
	// create camera struct
	dc1394camera_list_t * list;
	dc1394_camera_enumerate (libdcContext, &list);
	camera = dc1394_camera_new (libdcContext, list->ids[cameraNumber].guid);
	if (!camera) {
		stringstream error;
		error << "Failed to initialize camera " << cameraNumber << " with guid " << list->ids[0].guid;
		ofLog(OF_LOG_ERROR, error.str());
		return;
	} else {
		stringstream msg;
		msg << "Using camera with GUID " << camera->guid;
		ofLog(OF_LOG_VERBOSE, msg.str());
	}
	dc1394_camera_free_list(list);
	
	// select highest res mode:
	dc1394video_modes_t video_modes;
	dc1394_video_get_supported_modes(camera, &video_modes);
	dc1394video_mode_t video_mode;
	dc1394color_coding_t coding;
	dc1394color_coding_t targetCoding = getLibdcType(imageType);
	for (int i = video_modes.num - 1; i >= 0; i--) {
		if (!dc1394_is_video_mode_scalable(video_modes.modes[i])) {
			dc1394_get_color_coding_from_video_mode(camera, video_modes.modes[i], &coding);
			if (coding == targetCoding) {
				video_mode = video_modes.modes[i];
				break;
			}
		}
		if(i == 0) {
			ofLog(OF_LOG_ERROR, "Camera does not support the target mode.");
			return;
		}
	}
	dc1394_get_image_size_from_video_mode(camera, video_mode, &width, &height);
	 
	// get highest framerate
	dc1394framerates_t framerates;
	dc1394_video_get_supported_framerates(camera, video_mode, &framerates);
	dc1394framerate_t framerate = framerates.framerates[framerates.num - 1];
	
	// apply everything
	dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_400);
	dc1394_video_set_mode(camera, video_mode);
	dc1394_video_set_framerate(camera, framerate);
	dc1394_capture_setup(camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT);
	
	// print out features
	dc1394featureset_t features;
	dc1394_feature_get_all(camera, &features);
	dc1394_feature_print_all(&features, stdout);
}

void ofxLibdc::setBrightness(unsigned int brightness) {
	setFeature(DC1394_FEATURE_BRIGHTNESS, brightness);
}

void ofxLibdc::setGamma(unsigned int gamma) {
	setFeature(DC1394_FEATURE_GAMMA, gamma);
}

void ofxLibdc::setGain(unsigned int gain) {
	setFeature(DC1394_FEATURE_GAIN, gain);
}

void ofxLibdc::setExposure(unsigned int exposure) {
	setFeature(DC1394_FEATURE_EXPOSURE, exposure);
}

void ofxLibdc::setShutter(unsigned int shutter) {
	setFeature(DC1394_FEATURE_SHUTTER, shutter);
}

void ofxLibdc::setBrightnessNorm(float brightness) {
	setFeatureNorm(DC1394_FEATURE_BRIGHTNESS, brightness);
}

void ofxLibdc::setGammaNorm(float gamma) {
	setFeatureNorm(DC1394_FEATURE_GAMMA, gamma);
}

void ofxLibdc::setGainNorm(float gain) {
	setFeatureNorm(DC1394_FEATURE_GAIN, gain);
}

void ofxLibdc::setExposureNorm(float exposure) {
	setFeatureNorm(DC1394_FEATURE_EXPOSURE, exposure);
}

void ofxLibdc::setShutterNorm(float shutter) {
	setFeatureNorm(DC1394_FEATURE_SHUTTER, shutter);
}

void ofxLibdc::setFeature(dc1394feature_t feature, unsigned int value) {
	dc1394_feature_set_power(camera, feature, DC1394_ON);
	dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, feature, value);
}

void ofxLibdc::setFeatureNorm(dc1394feature_t feature, float value) {
	unsigned int min, max;
	getFeatureRange(feature, &min, &max);	
	dc1394_feature_set_power(camera, feature, DC1394_ON);
	dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, feature, (unsigned int) (value * (max - min) + min));
}

void ofxLibdc::getBrightnessRange(unsigned int* min, unsigned int* max) {
	getFeatureRange(DC1394_FEATURE_BRIGHTNESS, min, max);
}

void ofxLibdc::getGammaRange(unsigned int* min, unsigned int* max) {
	getFeatureRange(DC1394_FEATURE_GAMMA, min, max);
}

void ofxLibdc::getGainRange(unsigned int* min, unsigned int* max) {
	getFeatureRange(DC1394_FEATURE_GAIN, min, max);
}

void ofxLibdc::getExposureRange(unsigned int* min, unsigned int* max) {
	getFeatureRange(DC1394_FEATURE_EXPOSURE, min, max);
}

void ofxLibdc::getShutterRange(unsigned int* min, unsigned int* max) {
	getFeatureRange(DC1394_FEATURE_SHUTTER, min, max);
}

void ofxLibdc::getFeatureRange(dc1394feature_t feature, unsigned int* min, unsigned int* max) {
	dc1394_feature_get_boundaries(camera, feature, min, max);	
}

float ofxLibdc::getShutterAbs() {
	return getFeatureAbs(DC1394_FEATURE_SHUTTER);
}

float ofxLibdc::getFeatureAbs(dc1394feature_t feature) {
	float value;
	dc1394_feature_get_absolute_value(camera, feature, &value);
	return value;
}

unsigned int ofxLibdc::getWidth() const {
	return width;
}

unsigned int ofxLibdc::getHeight() const {
	return height;
}

int ofxLibdc::getImageType() const {
	return imageType;
}

void ofxLibdc::setImageType(int imageType) {
	this->imageType = imageType;
}

void ofxLibdc::setTransmit(bool transmit) {
	dc1394_video_set_transmission(camera, transmit ? DC1394_ON : DC1394_OFF);
}

void ofxLibdc::getOneShot(ofImage& img) {
	setTransmit(false);
	flush();
	dc1394_video_set_one_shot(camera, DC1394_ON);
	dc1394video_frame_t *frame;
	dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
	img.setFromPixels(frame->image, width, height, imageType);
	dc1394_capture_enqueue(camera, frame);
}

void ofxLibdc::flush() {
	dc1394video_frame_t *frame;
	do {
		dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame);
		if(frame != NULL)
			dc1394_capture_enqueue(camera, frame);
	} while (frame != NULL);
}