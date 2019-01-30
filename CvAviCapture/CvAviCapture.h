#ifndef CvAviCapture_H
#define CvAviCapture_H

#include <cv.h>
#include <highgui.h>

class CvAviCapture
{
private:
	    //OpenCv
		CvCapture *capture;
		IplImage *cvImage;

public:
	CvAviCapture();
	~CvAviCapture();
	IplImage* captureFrame(int pos);
};

#endif