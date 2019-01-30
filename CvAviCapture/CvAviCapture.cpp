#include "CvAviCapture.h"

CvAviCapture::CvAviCapture(){

	//init avi
	capture = cvCaptureFromAVI( "../white.avi" );

	//Get one image to be able to calculate the OpenCV image size
	cvImage = cvQueryFrame(capture);
}

IplImage* CvAviCapture::captureFrame(int pos) {

	//set capture property
	cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, pos);

	//capture new frame
	cvImage = cvQueryFrame(capture);

	IplImage *cvImageSmall = cvCreateImage(cvSize(768, 512), IPL_DEPTH_8U, 3); 

	cvResize(cvImage, cvImageSmall);

	return cvImageSmall;
}

CvAviCapture::~CvAviCapture(){
	cvReleaseCapture( &capture );
}