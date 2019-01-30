#ifndef CvCamera_H
#define CvCamera_H

#include <cv.h>

#include "../CvFlyCapture/FlyCapture.h"
#include "../CvCapture/CvCapture.h"
#include "../CvAviCapture/CvAviCapture.h"

class CvCamera
{
private:
		int captureMode;
	    //OpenCv
		IplImage *cvImage;

public:
	CvCamera(int capMode);
	~CvCamera();
	IplImage* captureFrame(int pos = 0);

private:
	void cameraInit();
	void cameraDeInit();

private:
	FlyCapture* flyCapture; 
	CvCapture* cvCapture;
	CvAviCapture* cvAviCapture;

};

#endif