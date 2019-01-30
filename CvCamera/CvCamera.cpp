#include "CvCamera.h"

CvCamera::CvCamera(int capMode){
	//init camera
	captureMode = capMode;
	cameraInit();
}

void CvCamera::cameraInit() {
	//default is FlyCapture
   switch (captureMode) {
        case 0:
			//create fly capture object
			flyCapture = new FlyCapture();
			break;
		case 1:
			//create cvCapture object
			cvCapture = new CvCapture();
		case 2:
			//create cvAViCapture object
			cvAviCapture = new CvAviCapture();
			break;
		default:
			//create fly capture object
			flyCapture = new FlyCapture();
			break;
	}
}

IplImage* CvCamera::captureFrame(int pos) {
	//retrieve new frame
	switch (captureMode) {
        case 0:
			//capture using flyCapture
			cvImage = flyCapture->captureFrame();
			break;
		case 1:
			cvImage = cvCapture->captureFrame();
			break;
		case 2:
			cvImage = cvAviCapture->captureFrame(pos);
			break;
		default:
			//capture using flyCapture
			cvImage = flyCapture->captureFrame();
			break;
	}
	return cvImage;
}


CvCamera::~CvCamera() {
	cameraDeInit();
}

void CvCamera::cameraDeInit() {
	//default is FlyCapture
   switch (captureMode) {
        case 0:
			//destroy fly capture object
			flyCapture->~FlyCapture();
			break;
		case 1:
			//destroy cvCapture object
			cvCapture->~CvCapture();
			break;
		case 2:
			//destroy cvAviCapture object
			cvAviCapture->~CvAviCapture();
			break;
		default:
			//destroy fly capture object
			flyCapture->~FlyCapture();
			break;
	}
}