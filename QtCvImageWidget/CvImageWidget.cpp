#include "CvImageWidget.h"

CvImageWidget::CvImageWidget(QWidget *parent)
	: QWidget(parent)
{
	/**
	*	Sets up CvImageWidget and sets the event listener to its grandparent, which should be WebcamUI. 
	*	Be careful making changes in QT Designer, overlayWidget sits on top of videoWidget_Interactive, but 
	*	is NOT its child. It is easy to accidentally make it so, breaking the link to WebcamUI.
	*/
	ui.setupUi(this);
	scale = 1;
	offx = 0;
	offy = 0;
	qDataSize = 1;
	qData = new uchar[1]; // cheap way to ensure we don't free an invalid pointer
	ui.imageLabel->setGeometry(QRect(0, 0, this->width(), this->height()));
}

void CvImageWidget::RetriveProperties(IplImage *cvImage) {	
	/**
	*	Retrieves the width, height, number of channels, depth, widthStep and a pointer to the image data of cvOverlay.
	*
	*	Stores these in class variables for later access.
	*
	*	Also sets up local storage for a QImage, which does not get dealloacated until either this function resizes it, 
	*	or the destructor.
	*/
	imgwidth		=	cvImage->width;
	imgheight		=	cvImage->height;
	nChannels	=	cvImage->nChannels;
	step		=	cvImage->widthStep;
	depth		=   cvImage->depth;
	cvData	    =	(uchar*)cvImage->imageData; 

	ui.imageLabel->setGeometry(QRect(0, 0, this->width(), this->height()));

	//QImage
	if (qDataSize < imgwidth*imgheight*nChannels)
	{
		free(qData);
		qDataSize = imgwidth*imgheight*nChannels;
		qData = new uchar[qDataSize];
	}
}

void CvImageWidget::resetInputContext(){
	ui.imageLabel->clear();
}

void CvImageWidget::drawImage(IplImage *cvImage) {
	/**
	*	This function is called to change the image.
	*
	*	This no longer covers redrawing the frame since the overlay is no longer the exact same
	*	image as used to update the display. This change allows independent changes of view.
	*
	*	This function then calls Redraw() instead of redrawing itself.
	*/
	//Retrive Properties
	RetriveProperties(cvImage);

	switch (depth) {
        case IPL_DEPTH_8U:
            switch (nChannels) {
                case 3:
					Concurrency::parallel_for (int (0), imgheight, [&](int h) {
						for (int w = 0; w < imgwidth; w++) {
							qData[(w+imgwidth*h)*3 + 0] = cvData[h*step + w*nChannels + 2];  //R
							qData[(w+imgwidth*h)*3 + 1] = cvData[h*step + w*nChannels + 1]; //G
							qData[(w+imgwidth*h)*3 + 2] = cvData[h*step + w*nChannels + 0]; //B
						}
						});
					 whole_image = QImage(qData,imgwidth,imgheight, QImage::Format_RGB888); //finaly move data.
					 break;
                default:
					 msgBox.setText("This number of channels is not supported.");
					 msgBox.exec();
                    break;
            }
            break;
        default:
			msgBox.setText("This type of IplImage is not implemented in CvImageWidget");
			msgBox.exec();
            break;
    }
    //ui.imageLabel->setPixmap(QPixmap::fromImage(image));  

	Redraw();

	//Free QImage data
	//free(qData);
}

int CvImageWidget::Redraw()
{
	/** 
	*	Converts whole_overlay into a scaled subimage of whole_overlay. Does this first by converting the
	*	X,Y and Scale values into a box, then resizing that box to fit.
	*/
	//if (this->objectName()=="videoWidget_Interactive")
	//{
	//	msgBox.setText(this->objectName());
	//	msgBox.exec();
	//}
	QRect subimg;

	subimg.setX(offx);
	subimg.setY(offy);
	
	subimg.setWidth( ((double)this->width()) / scale);
	subimg.setHeight( ((double)this->height()) / scale);
	
	if (scale==1)
	{
		ui.imageLabel->setPixmap(QPixmap::fromImage(whole_image.copy(subimg)));
	}
	else
	{
		ui.imageLabel->setPixmap(QPixmap::fromImage(whole_image.copy(subimg).scaled(this->size())));
	}	
	return true;
}

int CvImageWidget::setXY(int x, int y)
{
	/**
	*	Overloaded function to set the position of the top left corner of the visible area when scrolled.
	*/
	if (x<0 || x>whole_image.width() || y<0 || y>whole_image.height())
		return false;

	offx = x;
	offy = y;
	return Redraw();
}
int CvImageWidget::setXY(QPoint pnt)
{
	return setXY(pnt.x(),pnt.y());
}

int CvImageWidget::setX(int x)
{
	/**
	*	Overloaded function to set the position of the left hand edge of the visible area when scrolled.
	*/
	return setXY(x,offy);
}
int CvImageWidget::setY(int y)
{	
	/**
	*	Overloaded function to set the position of the top edge of the visible area when scrolled.
	*/
	return setXY(offx,y);
}

int CvImageWidget::setScale(double sc, int hoff, int voff)
{
	///
	///Handles the scaling of the displayed image
	//msgBox.setText(QString::number(sc));
	//msgBox.exec();
	if (sc>0)
	{
		scale = sc;
		/*if (hfrac>=0)
		{
			double xmax, ymax;
			xmax = (whole_image.width() * scale) - this->width();
			ymax = (whole_image.height() * scale) - this->height();
			offx = (int)(xmax * hfrac);
			offy = (int)(ymax * vfrac);
		}*/

		if (hoff>=0)
		{
			offx = hoff;
			offy = voff;
		}

		return Redraw();
	}
	else
		return false;
}


CvImageWidget::~CvImageWidget()
{
	free(qData);
}
