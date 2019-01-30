#ifndef IMAGEHUI_H
#define IMAGEHUI_H

#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopWidget>

#include <cv.h>
#include <highgui.h>

#include "ui_imageUI.h"

#include "../CvCamera/CvCamera.h"
#include "../TADDengine/Engine.h"


class imageUI : public QMainWindow
{
	Q_OBJECT

	private:
	int captureMode;
	IplImage *cvImage;
	int pos;

	int selectedClass;
	bool isSelecting;
	bool isClassify;
	IplImage *cvOverlay; QPoint clickPos;


	public:
	imageUI(QMainWindow *parent = 0, Qt::WFlags flags = 0);
	~imageUI();

	protected:
	void timerEvent(QTimerEvent*); 
	bool eventFilter(QObject *object, QEvent *event);
	void cvOverlayWidgetEvent(QMouseEvent *event);


	private:
	void centerWidgetOnScreen(QWidget * widget);


	protected slots:
	void prevAction();
	void nextAction();

	void class1Action();
	void class2Action();
	void class3Action();
	void class4Action();
	void classifyAction();
	void trainAction();



private:
	Ui::imageUI ui;
	CvCamera* cvCamera;
	TADDengine* TODDeng;
};

#endif // SPLASHUI_H
