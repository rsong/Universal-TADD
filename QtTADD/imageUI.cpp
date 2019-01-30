#include "imageUI.h"

imageUI::imageUI(QMainWindow *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	centerWidgetOnScreen(this);

	//init camera
	captureMode = 2; //set mode avi
	cvCamera = new CvCamera(captureMode);

	pos = 0;

	//Get a frame from the camera to set up TADDengine
	cvImage = cvCamera->captureFrame(pos);
	TADDeng = new TODDengine(cvImage->width, cvImage->height);

	cvImage = TADDeng->SLIC(cvImage,0);
	ui.videoWidget->drawImage(cvImage);

   //get classifed overlay
	cvOverlay = TADDeng->classifyOverlayAdaboost(isClassify);

	isSelecting = false; //init off
	isClassify = false;
	selectedClass = 1;
	startTimer(34);  //30 fps

	TADDeng->initOverlay();


}

void imageUI::timerEvent(QTimerEvent*) {
	    
    	if(isClassify){
			cvOverlay = TADDeng->classifyOverlayAdaboost(isClassify);
		}

		//draw overlay
		ui.overlayWidget->drawOverlay(cvOverlay);
}

bool imageUI::eventFilter(QObject *object, QEvent *event) {
	if (object == ui.overlayWidget && event->type() == QEvent::MouseButtonPress)
     {
         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
         cvOverlayWidgetEvent(mouseEvent); 
         return QObject::eventFilter(object, event);
     }
	if (object == ui.overlayWidget && event->type() == QEvent::MouseMove)
     {
         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
         cvOverlayWidgetEvent(mouseEvent); 
         return QObject::eventFilter(object, event);
     }
     return QObject::eventFilter(object, event);
}

void imageUI::cvOverlayWidgetEvent(QMouseEvent *event){


	if (event->button() == Qt::LeftButton) {
		isSelecting = true;
		clickPos = event->pos();
		//updateOverlay
		cvOverlay = TADDeng->updateOverlay(clickPos,selectedClass);
	}
		if (event->button() == Qt::RightButton) {
		isSelecting = false;
		clickPos = event->pos();
		//updateOverlay
		cvOverlay = TADDeng->updateOverlay(clickPos,0); //off
	}
		if(event->button() == Qt::NoButton && isSelecting) {
		clickPos = event->pos();
		//updateOverlay
		cvOverlay = TADDeng->updateOverlay(clickPos,selectedClass); 
	}

		if(event->button() == Qt::NoButton && !isSelecting) {
		clickPos = event->pos();
		//updateOverlay
		cvOverlay = TADDeng->updateOverlay(clickPos,0); 
	}
}


void imageUI::centerWidgetOnScreen(QWidget * widget) {

     QRect rect = QApplication::desktop()->availableGeometry(widget);

     widget->move(rect.center() - widget->rect().center());
}

void imageUI::nextAction(){

	//add max frame condition
	pos++;

	//Get a frame from the camera 
	cvImage = cvCamera->captureFrame(pos);
	
	cvImage = TADDeng->SLIC(cvImage,0);
	ui.videoWidget->drawImage(cvImage);

	TADDeng->initOverlay();
}

void imageUI::prevAction(){


	if(pos >= 1)
	   pos--;

	//Get a frame from the camera 
	cvImage = cvCamera->captureFrame(pos);
	
	cvImage = TADDeng->SLIC(cvImage,0);
	ui.videoWidget->drawImage(cvImage);

	TADDeng->initOverlay();

}

void imageUI::trainAction(){
	TADDeng->refAdaboost();
}

void imageUI::classifyAction(){

	isClassify = !isClassify;
}


//Radio buttons
void imageUI::class1Action(){
	selectedClass =1;
}
void imageUI::class2Action(){
	selectedClass=2;
}
void imageUI::class3Action(){
	selectedClass=3;
}
void imageUI::class4Action(){
	selectedClass=4;
}

imageUI::~imageUI()
{

}
