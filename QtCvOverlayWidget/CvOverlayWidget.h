#ifndef CVOVERLAYWIDGET_H
#define CVOVERLAYWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QImage>
#include <QtGui/QMessageBox>
#include <QtGui/qscrollbar.h>
#include <cv.h>

//#include <ppl.h>
#include <tbb\parallel_for.h>

#include "GeneratedFiles/ui_CvOverlayUI.h"
///
///CvOverlayWidget and CvImageWidget are relatively simple controls which add some functionality to simple labels used for displaying images.

class CvOverlayWidget : public QWidget
{

	Q_OBJECT
private:
	//Image properties
	int _width, _height, nChannels, step, depth, qDataSize;
	uchar *cvData, *qData;
	QImage overlay;
	//output error and debug
	QMessageBox msgBox;
	QString msg;

public:
	CvOverlayWidget(QWidget *parent = 0);
	~CvOverlayWidget();
	void drawOverlay(IplImage *cvOverlay);
	int setScale(double scale, int hoff = 0, int voff = 0);
	int setXY(int x, int y);
	int setXY(QPoint pnt);
	int setX(int x);
	int setY(int y);

	QPoint convertPoint(QPoint in);
	int setupScrollbars(QScrollBar *v, QScrollBar *h);

private:
	void RetriveProperties(IplImage *cvOverlay);
	int Redraw();
	int offx,offy;
private:
	Ui::CvOverlayWidget ui;
	
	// Scrolling controller
public:
	int getHOff(double sc);
	int getVOff(double sc);

private:
	double hScrollFrac, vScrollFrac;
	double scale,oldscale;
	QScrollBar *hScroll, *vScroll;
};

#endif // CVOVERLAYWIDGET_H
