#ifndef FlyCapture_H
#define FlyCapture_H

#include <FlyCapture2.h>
#include <cv.h>

class FlyCapture
{
private:
	    //PT Grey
		FlyCapture2::Camera capturePT; 
		FlyCapture2::Error error;
		FlyCapture2::PGRGuid guid;
		FlyCapture2::BusManager busMgr;
		FlyCapture2::Image framePt;
		FlyCapture2::Image rawImage;
		FlyCapture2::Image convertedImage;
		//CV
		IplImage* cvImage;

public:
	FlyCapture();
	~FlyCapture();
	IplImage* captureFrame();
};

#endif