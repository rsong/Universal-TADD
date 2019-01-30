#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QImage>
#include <QtGui/QMessageBox>

#include <cv.h>

#include <ppl.h>
#include "GeneratedFiles/ui_CvImageUI.h"
///
///CvOverlayWidget and CvImageWidget are relatively simple controls which add some functionality to simple labels used for displaying images.

class CvImageWidget : public QWidget
{
	
	Q_OBJECT

private:
	//Image properties
	int imgwidth, imgheight, nChannels, step, depth,qDataSize;
	uchar *cvData, *qData;
	QImage image,whole_image;
	//output error and debug
	QMessageBox msgBox;
	QString msg;

public:
	CvImageWidget(QWidget *parent = 0);
	~CvImageWidget();
	void drawImage(IplImage *cvImage);
	void resetInputContext();
	int setScale(double scale, int hoff = 0, int voff = 0);
	int setXY(int x, int y);
	int setXY(QPoint pnt);
	int setX(int x);
	int setY(int y);

private:
	void RetriveProperties(IplImage *cvImage);
	int Redraw();
	int offx,offy;
	double scale,oldscale;

private:
	Ui::CvImageWidget ui;

};

#endif // CVIMAGEWIDGET_H
