#include "FlyCapture.h"

FlyCapture::FlyCapture() {

	//init camerea
		//Getting the GUID of the cam
		error = busMgr.GetCameraFromIndex(0, &guid);
		if (error != FlyCapture2::PGRERROR_OK)
		{
			error.PrintErrorTrace();
		}


	// Connect to a camera
		error = capturePT.Connect(&guid);
		if (error != FlyCapture2::PGRERROR_OK)
		{
			error.PrintErrorTrace();
		}

		 //Starting the capture
		error = capturePT.StartCapture();
		if (error != FlyCapture2::PGRERROR_OK)
		{
			error.PrintErrorTrace();
		}

	 //Get one raw image to be able to calculate the OpenCV image size
		capturePT.RetrieveBuffer(&rawImage);
		cvImage = cvCreateImage(cvSize(rawImage.GetCols(), rawImage.GetRows()), IPL_DEPTH_8U, 3); 
}

IplImage* FlyCapture::captureFrame(){
	// Capture new Fframe
		capturePT.RetrieveBuffer(&rawImage);

	//bayer to BGR
		rawImage.Convert( FlyCapture2::PIXEL_FORMAT_BGR, &convertedImage );

	//Copy the image into the IplImage of OpenCV
		memcpy(cvImage->imageData, convertedImage.GetData(), convertedImage.GetDataSize());

   return cvImage;
}

FlyCapture::~FlyCapture(){
	// Stop capturing images
    error = capturePT.StopCapture();
    if (error != FlyCapture2::PGRERROR_OK)
    {
        error.PrintErrorTrace();
    }      
    
    // Disconnect the camera
    error = capturePT.Disconnect();
    if (error != FlyCapture2::PGRERROR_OK)
    {
        error.PrintErrorTrace();
    }
}