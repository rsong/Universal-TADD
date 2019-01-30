#include "CvOverlayWidget.h"

CvOverlayWidget::CvOverlayWidget(QWidget *parent)
	: QWidget(parent)
{
	/**
	*	Sets up CvOverlayWidget and sets the event listener to its grandparent, which should be WebcamUI. 
	*	Be careful making changes in QT Designer, overlayWidget sits on top of videoWidget_Interactive, but 
	*	is NOT its child. It is easy to accidentally make it so, breaking the link to WebcamUI.
	*/
	offx = offy = 0;
	scale = oldscale = 1;

	qData = new uchar[1];
	qDataSize = 1;

	hScroll = NULL; vScroll = NULL;

	ui.setupUi(this);
	this->installEventFilter(parent->parent());
	ui.overlayLabel->setGeometry(QRect(0, 0, this->width(), this->height()));
}

void CvOverlayWidget::RetriveProperties(IplImage *cvOverlay) {

	/**
	*	Retrieves the width, height, number of channels, depth, widthStep and a pointer to the image data of cvOverlay.
	*
	*	Stores these in class variables for later access.
	*
	*	Also sets up local storage for a QImage, which does not get dealloacated until either this function resizes it, 
	*	or the destructor.
	*/
	_width		=	cvOverlay->width;
	_height		=	cvOverlay->height;
	nChannels	=	cvOverlay->nChannels;
	step		=	cvOverlay->widthStep;
	depth		=   cvOverlay->depth;
	cvData	    =	(uchar*)cvOverlay->imageData; 

	ui.overlayLabel->setGeometry(QRect(0, 0, this->width(), this->height()));

	//QImage
	//qData = new uchar[_width*_height*(nChannels+1)]; //+1 for alpha
	if (qDataSize < _width*_height*(nChannels+1))
	{
		free(qData);
		qDataSize = _width*_height*(nChannels+1);
		qData = new uchar[qDataSize];
	}
}

void CvOverlayWidget::drawOverlay(IplImage *cvOverlay) {

	/**
	*	This function is called to change the image displayed as the overlay.
	*
	*	This no longer covers redrawing the frame since the overlay is no longer the exact same
	*	image as used to update the display. This change allows independent changes of view.
	*
	*	This function then calls Redraw() instead of redrawing itself.
	*/
	//Retrive Properties
	RetriveProperties(cvOverlay);
	int widthx = _width;

	switch (depth) {
        case IPL_DEPTH_8U:
            switch (nChannels) {
                case 3:
					tbb::parallel_for (int (0), _height, [&, widthx](int h) {
						for (int w = 0; w < widthx; w++) {
							if(cvData[h*step + w*nChannels + 2] == 0 & cvData[h*step + w*nChannels + 1] == 0 & cvData[h*step + w*nChannels + 0] == 0)
							{
								qData[(w+widthx*h)*4 + 3] = 0;  //A
							}
							else
							{
							qData[(w+widthx*h)*4 + 0] = cvData[h*step + w*nChannels + 0];  //R
							qData[(w+widthx*h)*4 + 1] = cvData[h*step + w*nChannels + 1]; //G
							qData[(w+widthx*h)*4 + 2] = cvData[h*step + w*nChannels + 2]; //B;
							qData[(w+widthx*h)*4 + 3] = 127;  //A
							}
						}
					});
					overlay = QImage(qData,widthx,_height, QImage::Format_ARGB32); //finaly move data.
					 break;
                default:
					 msgBox.setText("This number of channels is not supported.");
					 msgBox.exec();
                    break;
            }
            break;
        default:
			msgBox.setText("This type of IplImage is not implemented in CvOverlayWidget");
			msgBox.exec();
            break;
    }

    //ui.overlayLabel->setPixmap(QPixmap::fromImage(overlay));  
	Redraw();
	//Free QImage data
	//free(qData);
}


int CvOverlayWidget::Redraw()
{
	/* *
	*	Converts whole_overlay into a scaled subimage of whole_overlay. Does this first by converting the
	*	X,Y and Scale values into a box, then resizing that box to fit.
	*
	*	Ensures that scrollbars are on the correct scale.
	*/
	
	if (hScroll != NULL && vScroll != NULL)
	{

		if (overlay.width() * scale > this->width())
		{
			if (oldscale==scale)
			{
				hScroll->setMaximum(((overlay.width() * scale) - this->width())/scale);
				hScroll->setEnabled(true);
				//hScrollFrac = (double)(hScroll->value()) / (double)(hScroll->maximum());
				hScroll->setSingleStep((double)(hScroll->maximum()) / 10);
				hScroll->setPageStep((double)(hScroll->maximum()) / 5);
			}
			else
			{
				// This should be the only one to actually set the maximum.
				hScroll->setMaximum(((overlay.width() * scale) - this->width())/scale);
				//hScroll->setValue((double)(hScroll->maximum()) * hScrollFrac);
				hScroll->setEnabled(true);
				hScroll->setSingleStep((double)(hScroll->maximum()) / 10);
				hScroll->setPageStep((double)(hScroll->maximum()) / 5);
			}
		}
		else
		{
			hScroll->setMaximum(0);
			hScroll->setEnabled(false);
		}
		if (overlay.height() * scale > this->height())
		{
			if (oldscale==scale)
			{
				vScroll->setMaximum(((overlay.height() * scale) - this->height())/scale);
				vScroll->setEnabled(true);
				//vScrollFrac = (double)(vScroll->value()) / (double)(vScroll->maximum());
			}
			else
			{
				vScroll->setMaximum(((overlay.height() * scale) - this->height())/scale);
				vScroll->setSingleStep((double)(vScroll->maximum()) / 10);
				vScroll->setPageStep((double)(vScroll->maximum()) / 5);

				//vScroll->setValue((double)(vScroll->maximum()) * vScrollFrac);
				vScroll->setEnabled(true);
			}
		}
		else
		{
			vScroll->setMaximum(0);
			vScroll->setEnabled(false);
		}

		if (scale != oldscale)
		{
			oldscale = scale;
			//return setXY(hScroll->value(),vScroll->value());
			//return 1;
		}
	
	if (offx>hScroll->maximum())
		offx = hScroll->maximum();

	if (offy>hScroll->maximum())
		offy = hScroll->maximum();
	}

	QRect subimg;

	subimg.setX(offx);
	subimg.setY(offy);
	
	subimg.setWidth( ((double)this->width()) / scale);
	subimg.setHeight( ((double)this->height()) / scale);

	//msgBox.setText(this->objectName() + " " + QString::number(this->x()) + " " + QString::number(this->y()) + " " + QString::number(this->height()) + " " + QString::number(this->width()));  
	//msgBox.exec();

	
	ui.overlayLabel->setPixmap(QPixmap::fromImage(overlay.copy(subimg).scaled(this->width(),this->height())));


	return true;
}

QPoint CvOverlayWidget::convertPoint(QPoint in)
{
	/**
	*	takes the x,y coordinates of a point on the displayed image and returns the equivalent x,y coordinates for the same point within the original image.
	*/
	in.setX((double)(in.x()/scale)+offx); 
	in.setY((double)(in.y()/scale)+offy); 
	return in;
}

int CvOverlayWidget::setXY(int x, int y)
{
	/**
	*	Sets the position of the top left corner of the visible area when scrolled.
	*/
	if (x<0 || x>overlay.width() || y<0 || y>overlay.height())
		return false;

	offx = x;
	offy = y;
	return Redraw();
}
int CvOverlayWidget::setXY(QPoint pnt)
{
	/**
	*	Overloaded function to set the position of the top left corner of the visible area when scrolled.
	*/
	return setXY(pnt.x(),pnt.y());
}
int CvOverlayWidget::setX(int x)
{
	/**
	*	Overloaded function to set the position of the left hand edge of the visible area when scrolled.
	*/
	return setXY(x,offy);
}
int CvOverlayWidget::setY(int y)
{
	/**
	*	Overloaded function to set the position of the top edge of the visible area when scrolled.
	*/
	return setXY(offx,y);
}

int CvOverlayWidget::getHOff(double sc)
{
	///
	/// Interaction with scrollbars during zooming
	double hScrollFrac = (double)(hScroll->value()) / (double)(hScroll->maximum());
	double output = ((double)(overlay.width()) * scale) - ((this->width())/scale);
	output = output * hScrollFrac;
	return (int) output;
}

int CvOverlayWidget::getVOff(double sc)
{
	///
	/// Interaction with scrollbars during zooming
	double vScrollFrac = (double)(vScroll->value()) / (double)(vScroll->maximum());
	double output = ((double)(overlay.height()) * scale) - ((this->height())/scale);
	output = output * vScrollFrac;
	return (int) output;
}


int CvOverlayWidget::setScale(double sc, int hoff, int voff)
{
	/**
	*	Handles zooming
	*/
	if (sc>0)
	{
		scale = sc;
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

int CvOverlayWidget::setupScrollbars(QScrollBar *h, QScrollBar *v)
{
	/**
	*	This class handles the bulk of the calculations required for scrolling, including setting the ranges for the scrollbars. This function passes it the handles for both scrollbars.
	*/
	vScroll = v;
	hScroll = h;
	return true;
}

CvOverlayWidget::~CvOverlayWidget()
{

}
