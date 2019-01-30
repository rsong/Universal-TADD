#include "CvCapture.h"

CvCapture::CvCapture(){

	VI.setupDevice(0,1280,720,4); 

	int width = VI.getWidth(0);
    int height = VI.getHeight(0);

	cvImage = cvCreateImage(cvSize(1280,720),IPL_DEPTH_8U, 3);

	
	/*
	VI.setVideoSettingCamera(0,VI.propBrightness,0, 2); 
	VI.setVideoSettingCamera(0,VI.propContrast, 0, 2); 
	VI.setVideoSettingCamera(0,VI.propSaturation, 0, 2); 
	VI.setVideoSettingCamera(0,VI.propWhiteBalance,0, 2); 
	VI.setVideoSettingCamera(0,VI.propBacklightCompensation,0, 2); 

	*/



	//init camera
	//capture = cvCaptureFromCAM(0);

	//cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH ,1280);
 
	//cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT ,720);

	//Get one image to be able to calculate the OpenCV image size
	//cvImage = cvQueryFrame(capture);
}

IplImage* CvCapture::captureFrame() {

	//capture new frame
	//cvImage = cvQueryFrame(capture);

	if(VI.isFrameNew(0))
      VI.getPixels(0, (unsigned char *)cvImage->imageData, false, true); 

	//cvShowImage("hi",cvImage);

	return cvImage;
}

CvCapture::~CvCapture(){
	VI.stopDevice(0);
}