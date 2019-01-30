#ifndef CvCapture_H
#define CvCapture_H

#include <cv.h>
#include <highgui.h>
#include <videoInput.h>

#include "../videoInput/videoInput.h"

class CvCapture
{
private:
	    //OpenCv
		CvCapture *capture;
		videoInput VI; 
		IplImage *cvImage;

public:
	CvCapture();
	~CvCapture();
	IplImage* captureFrame();
};

#endif