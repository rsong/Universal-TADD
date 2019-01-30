#ifndef WEBCAMHUI_H
#define WEBCAMHUI_H

#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopWidget>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <QString>

#include <qscrollarea.h>

#include <cv.h>
#include <boost/filesystem.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>

#include "ui_webcamUI.h"

#include "../CvCamera/CvCamera.h"
#include "../TADDengine/Engine.h"

#include <boost/algorithm/string.hpp>
#include <qdesktopservices.h>
#include <qurl.h>


class webcamUI : public QMainWindow
{
	Q_OBJECT
private:
	int imageIdx, nextImageIdx;
	int captureMode;
	IplImage *cvImage;IplImage *cvImage2;	int selectedClass;
	IplImage *cvImageMini;
	IplImage *cvCaptured;
	IplImage *cvCapturedbg;
	IplImage *cvMiniOverlayImage;
	IplImage *cvMiniOverlay;

	IplImage *cvOrginal;
	IplImage *cvMap;
	QStringList classNames;

	unsigned char* cvImageBucket;
	unsigned char* cvOverlayBucket;

	bool isSelecting;
	bool isClassify;
	bool isCaptured;
	bool isEdit;
	bool isUnselected;
	bool isBackground;
	bool isLearned;
	bool isLearning;
	bool storedData;
	bool isMultipleImageClassifier;
	bool NextisHit;
	bool Previousishit;
	int NumberOfImages;
	int framesBG;
	int latestIMGIDX;
	//int nextIMGIDX;

	int moved;
	int potatoCounter;

	IplImage *cvOverlay; QPoint clickPos;

	//background adjuster
	int thresholdValue;
	int areasizeValue;

	//output error and debug
	QMessageBox msgBox;
	QString msg;

	bool pause;
	bool truthToggle;
	vector<boost::filesystem::path> fileNames;
	vector<std::string> fileNamesC;

	int picCount,picCount1,picCount2;

	QScrollArea *widgetScroll;

public:
	webcamUI(QMainWindow *parent = 0, Qt::WFlags flags = 0);
	~webcamUI();
	
	void scrollXY(int x, int y);

protected:
	void timerEvent(QTimerEvent*); 
	bool eventFilter(QObject *object, QEvent *event);
	void cvOverlayWidgetEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *keypress);

private:
	void centerWidgetOnScreen(QWidget * widget);
	void zoomin();
	void zoomout();
	void loadFolder(QString folder);
	void clearFolder();
	void clearFolderTempl();
	bool autoloading;
	void updateClassLabels();
	void updateClassColours();
	QPixmap logoPixmap;

	
	bool logging;
	QString LogName;
	QString LogPrefix;
	QString LogFolder;
	int LogIdx;
	bool openLog(QString fname);
	void updateLog();

private:
	Ui::webcamUI ui;
	CvCamera* cvCamera;
	TADDengine* TADDeng;

protected slots:
	void pauseAction();
	void captureAction();
	void captureAction_NEW();

	void class1Action();
	void class2Action();
	void class3Action();
	void class4Action();
	void class5Action();
	void class6Action();

	void trainAction();
	void trainAllAction();
	void classifyAction();

	void removeBackgroundAction();

	void BGs1Action();
	void BGs2Action();

	void Reset();

	void previousAction();
	void nextAction();
	void nextAction_NEW();
	void editAction();

	void saveAction();

	void saveBGBigAction();
	void saveBigAction();
	
	void saveData();
	void loadData();

	void groundTruth();
	void nameChange(QString txt);

	void loadImage();

	void updateThreshold(int value);
	void updateArea(int value);

	void scrollX(int value);
	void scrollY(int value);
	void scaleslot(int value);

	void openFolder();
	void logButton();
	void logFolderButton();
};

#endif // SPLASHUI_H

