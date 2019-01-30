#include "webcamUI.h"

webcamUI::webcamUI(QMainWindow *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	/**
	*	Constructor
	*/
	ui.setupUi(this);
	centerWidgetOnScreen(this);

	//init camera
	captureMode = 1; //set mode to 1=CvCapture 52 = Canon 99=files
	cvCamera = new CvCamera(captureMode);
	//Get a frame from the camera to set up TADDengine
	cvImage = cvCamera->captureFrame();
	
	TADDeng = new TADDengine(cvImage->width, cvImage->height); //SLIC and KNN
	
	cvImageMini = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);
	cvCaptured = cvCreateImage( cvSize( cvImage->width, cvImage->height ), IPL_DEPTH_8U, 3);
	cvCapturedbg = cvCreateImage( cvSize( cvImage->width, cvImage->height ), IPL_DEPTH_8U, 3);
	cvMiniOverlayImage = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);
	cvMiniOverlay = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);

	//setup vars // quick hack
	//cvImage = cvCamera->captureFrame();
	//cvImage = TADDeng->SLIC(cvImage,1);

	//Timer
	pause = false; //init on false
	isCaptured = false;
	isSelecting = false; //init off
	isClassify = false;
	isUnselected = true;
	isBackground = false;
	isLearned = false;
	isLearning = false;
	isMultipleImageClassifier=false;
	NextisHit=false;
	Previousishit=false;
	selectedClass = 1;
	startTimer(1000); //now 1fps //30 fps
    isEdit=0;////////////////////////////////////////////////
	int size[] = { 576, 324, 3, 100 };
	//cvImageBucket = cvCreateMatND(3, size, CV_8U);
	//cvOverlayBucket = cvCreateMatND(3, size, CV_8U);

	cvImageBucket = (unsigned char*)malloc(1280*720*3*sizeof(unsigned char)*100);
	cvOverlayBucket = (unsigned char*)malloc(1280*720*3*sizeof(unsigned char)*100);

	storedData = false;

	moved = -1;

	picCount = 1;
	picCount1 = 1;
	picCount2 = 1;

	truthToggle = false;

	thresholdValue = 40;
	areasizeValue = 7000;

	// Unique ID for each image in a set.
	imageIdx = -1;
	latestIMGIDX=-1;
	nextImageIdx=0;

	ui.overlayWidget->setupScrollbars(ui.hScroll,ui.vScroll);
	
	connect(ui.vScroll,SIGNAL(valueChanged(int)),this,SLOT(scrollY(int)));
	connect(ui.hScroll,SIGNAL(valueChanged(int)),this,SLOT(scrollX(int)));
	connect(ui.zoomSlider,SIGNAL(valueChanged(int)),this,SLOT(scaleslot(int)));
	connect(ui.pushButton_Edit,SIGNAL(clicked()),this,SLOT(editAction()));

	connect(ui.EditClass1,SIGNAL(textEdited(QString)),this,SLOT(nameChange(QString)));
	connect(ui.EditClass2,SIGNAL(textEdited(QString)),this,SLOT(nameChange(QString)));
	connect(ui.EditClass3,SIGNAL(textEdited(QString)),this,SLOT(nameChange(QString)));
	connect(ui.EditClass4,SIGNAL(textEdited(QString)),this,SLOT(nameChange(QString)));
	connect(ui.EditClass5,SIGNAL(textEdited(QString)),this,SLOT(nameChange(QString)));
	connect(ui.EditClass6,SIGNAL(textEdited(QString)),this,SLOT(nameChange(QString)));

	connect(ui.btnLog,SIGNAL(clicked()),this,SLOT(logButton()));
	connect(ui.btnLogFolder,SIGNAL(clicked()),this,SLOT(logFolderButton()));
	connect(ui.btnOpenFolder,SIGNAL(clicked()),this,SLOT(openFolder()));

	logging = false;
	LogIdx = 0;
	ui.txtLogPrefix->setText(tr("Log_") + QString::number(QDate::currentDate().month())+"_"
		+QString::number(QDate::currentDate().day()));
	ui.txtLogFolder->setText(qApp->applicationDirPath() + "/Logs");

	autoloading = false;
	QString asfname=qApp->applicationDirPath()+"/Workspace/autosave.xml";
	QFile autosave(asfname);
	if (autosave.exists())
	{
		QMessageBox autoload;
		autoload.setText("Continue previous session?");
		autoload.setInformativeText("Warning: selecting No will lose unsaved data");
		autoload.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		int choice = autoload.exec();
		if (choice==QMessageBox::No)
		{
			clearFolder();
			clearFolderTempl();
		}
		else
		{
			autoloading = true;
			loadData();
		}
	}

	int thresh = TADDeng->loadSettings(qApp->applicationDirPath() + "/settings/settings.ini"); // empty string = use default file
	if (thresh>-1)
	{
		thresholdValue = thresh;
		ui.thresholdSlider->setValue(thresh);
	}

	
	logoPixmap.load(qApp->applicationDirPath() +"/Settings/Logos.png");

	ui.statusLabel->setPixmap(logoPixmap);

	updateClassLabels();
	updateClassColours();
}///////////////1

void webcamUI::keyPressEvent(QKeyEvent *keypress)
{	
	///
	/// Processes keypress events passed along from eventfilter().
	/// Passes unhandled keypresses on to the system. Used to grab undo and redo hotkeys.
		if (keypress->key() == Qt::Key_Z && keypress->modifiers() == Qt::ControlModifier)
		{
			// Undo
			cvOverlay = TADDeng->Undo();
			ui.overlayWidget->drawOverlay(cvOverlay);
			return;
		}

		if (keypress->key() == Qt::Key_Y && keypress->modifiers() == Qt::ControlModifier)
		{
			// Redo
			cvOverlay = TADDeng->Redo();
			ui.overlayWidget->drawOverlay(cvOverlay);
			return;
		}
	QWidget::keyPressEvent(keypress);

}

void webcamUI::timerEvent(QTimerEvent*) {
	///
	///Automatically updates the camera preview frame

         if(truthToggle == false){
			//retrieve new frame
			cvImage = cvCamera->captureFrame(); //returns to cvImage
			//shrink image for webcam feed
			cvResize(cvImage,cvImageMini,0);
			
			ui.videoWidget->drawImage(cvImageMini);
		 }

			/*
	switch (pause){
		 case false:
			//retrieve new frame
			cvImage = cvCamera->captureFrame(); //returns to cvImage

			//shrink image for webcam feed
			cvResize(cvImage,cvImageMini,0);
			ui.videoWidget->drawImage(cvImageMini);

			//get classifed overlay
			//cvOverlay = TADDeng->classifyOverlayKNN(isClassify);
			//cvOverlay = TADDeng->classifyOverlayAdaboost(isClassify);

			//draw overlay
			//ui.overlayWidget->drawOverlay(cvOverlay);
			break;
	case true:
			//retrieve new frame
			cvImage = cvCamera->captureFrame(); //returns to cvImage

			//shrink image for webcam feed
			cvResize(cvImage,cvImageMini,0);
			ui.videoWidget->drawImage(cvImageMini);

			//draw overlay
			//

			break;
	}
	*/
	
}


bool webcamUI::eventFilter(QObject *object, QEvent *event) {
	/**
	*	An inherited function which captures Mouse and Wheel events from within 
	*	the overlay window and sends them on either to cvOverlayWidgetEvent, in 
	*	the case of mouse button or movement events, or to the zoomin() and 
	*	zoomout() functions in the case of the wheel.
	*/
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

	if (object==ui.overlayWidget && event->type() == QEvent::Wheel)
	{
		QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
		if (wheelEvent->delta() < 0)
			zoomout();
		else
			zoomin();

		return true;
	}

     return QObject::eventFilter(object, event);

	 

	msgBox.setText(object->objectName());
	msgBox.exec();

     return QObject::eventFilter(object, event);
}

void webcamUI::updateClassLabels()
{
	///
	/// When a class name has changed, this will update everywhere in the interface
	/// that the class name appears.
	classNames.clear();
	classNames.append(ui.EditClass1->text());
	classNames.append(ui.EditClass2->text());
	classNames.append(ui.EditClass3->text());
	classNames.append(ui.EditClass4->text());
	classNames.append(ui.EditClass5->text());
	classNames.append(ui.EditClass6->text());

	ui.lblClass1->setText(ui.EditClass1->text());
	ui.lblClass2->setText(ui.EditClass2->text());
	ui.lblClass3->setText(ui.EditClass3->text());
	ui.lblClass4->setText(ui.EditClass4->text());
	ui.lblClass5->setText(ui.EditClass5->text());
	ui.lblClass6->setText(ui.EditClass6->text());

	ui.lblClass1_2->setText(ui.EditClass1->text());
	ui.lblClass2_2->setText(ui.EditClass2->text());
	ui.lblClass3_2->setText(ui.EditClass3->text());
	ui.lblClass4_2->setText(ui.EditClass4->text());
	ui.lblClass5_2->setText(ui.EditClass5->text());
	ui.lblClass6_2->setText(ui.EditClass6->text());
}

void webcamUI::updateClassColours()
{
	///
	/// Sets all class colour patches to the correct colour in the interface.

	QPalette pal = ui.lblColClass_1->palette();
	pal.setColor(ui.lblColClass_1->backgroundRole(),QColor(TADDeng->colourList[1][2],
		TADDeng->colourList[1][1],
		TADDeng->colourList[1][0]));
	ui.lblColClass_1->setPalette(pal);
	ui.lblColClass_1_2->setPalette(pal);

	pal = ui.lblColClass_2->palette();
	pal.setColor(ui.lblColClass_2->backgroundRole(),QColor(
		TADDeng->colourList[2][2],
		TADDeng->colourList[2][1],
		TADDeng->colourList[2][0]));
	ui.lblColClass_2->setPalette(pal);
	ui.lblColClass_2_2->setPalette(pal);

	pal = ui.lblColClass_3->palette();
	pal.setColor(ui.lblColClass_3->backgroundRole(),QColor(
		TADDeng->colourList[3][2],
		TADDeng->colourList[3][1],
		TADDeng->colourList[3][0]));
	ui.lblColClass_3->setPalette(pal);
	ui.lblColClass_3_2->setPalette(pal);

	pal = ui.lblColClass_4->palette();
	pal.setColor(ui.lblColClass_4->backgroundRole(),QColor(
		TADDeng->colourList[4][2],
		TADDeng->colourList[4][1],
		TADDeng->colourList[4][0]));
	ui.lblColClass_4->setPalette(pal);
	ui.lblColClass_4_2->setPalette(pal);

	pal = ui.lblColClass_5->palette();
	pal.setColor(ui.lblColClass_5->backgroundRole(),QColor(
		TADDeng->colourList[5][2],
		TADDeng->colourList[5][1],
		TADDeng->colourList[5][0]));
	ui.lblColClass_5->setPalette(pal);
	ui.lblColClass_5_2->setPalette(pal);

	pal = ui.lblColClass_6->palette();
	pal.setColor(ui.lblColClass_6->backgroundRole(),QColor(
		TADDeng->colourList[6][2],
		TADDeng->colourList[6][1],
		TADDeng->colourList[6][0]));
	ui.lblColClass_6->setPalette(pal);
	ui.lblColClass_6_2->setPalette(pal);

}

void webcamUI::nameChange(QString txt)
{
	///
	///Calls updateClassLabels, because one of the writeable fields containing class 
	///names has been altered.
	updateClassLabels();
}

void webcamUI::cvOverlayWidgetEvent(QMouseEvent *event){
	/**
	*	This funtion organises the system's response to a human making changes to the markup in the main overlay Widget.
	*/
	isUnselected = false;
	
	if(isCaptured == 1 && isEdit==0){

			if (event->button() == Qt::LeftButton) {
				isSelecting = true;
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				TADDeng->KeepBackup();
				cvOverlay = TADDeng->updateOverlay(clickPos,selectedClass);
				
			}
				if (event->button() == Qt::RightButton) {
				isSelecting = false;
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				TADDeng->KeepBackup();
				cvOverlay = TADDeng->updateOverlay(clickPos,0); //off
			}
				if(event->button() == Qt::NoButton && isSelecting) {
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				cvOverlay = TADDeng->updateOverlay(clickPos,selectedClass); 
			}

				if(event->button() == Qt::NoButton && !isSelecting) {
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				cvOverlay = TADDeng->updateOverlay(clickPos,0); 
			}


			ui.overlayWidget->drawOverlay(cvOverlay);

		ui.label_SelectedOneVal->setText(QString::number(TADDeng->ref_nb[1]));
		ui.label_SelectedTwoVal->setText(QString::number(TADDeng->ref_nb[2]));
		ui.label_SelectedThreeVal->setText(QString::number(TADDeng->ref_nb[3]));
		ui.label_SelectedFourVal->setText(QString::number(TADDeng->ref_nb[4]));
		ui.label_SelectedFiveVal->setText(QString::number(TADDeng->ref_nb[5]));
		ui.label_SelectedSixVal->setText(QString::number(TADDeng->ref_nb[6]));
			

	}

	if(isEdit==1){
			isCaptured == 1;
			   // ui.overlayWidget->drawOverlay(cvOverlay);
				TADDeng->KeepBackup();

				if (event->button() == Qt::LeftButton) {
				isSelecting = true;
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				TADDeng->KeepBackup();
				cvOverlay = TADDeng->updateOverlay(clickPos,selectedClass);
				
			}
				if (event->button() == Qt::RightButton) {
				isSelecting = false;
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				TADDeng->KeepBackup();
				cvOverlay = TADDeng->updateOverlay(clickPos,0); //off
			}
				if(event->button() == Qt::NoButton && isSelecting) {
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				cvOverlay = TADDeng->updateOverlay(clickPos,selectedClass); 
			}

				if(event->button() == Qt::NoButton && !isSelecting) {
				clickPos = ui.overlayWidget->convertPoint(event->pos());
				//updateOverlay
				cvOverlay = TADDeng->updateOverlay(clickPos,0); 
			}


		ui.overlayWidget->drawOverlay(cvOverlay);
		ui.label_SelectedOneVal->setText(QString::number(TADDeng->ref_nb[1]));
		ui.label_SelectedTwoVal->setText(QString::number(TADDeng->ref_nb[2]));
		ui.label_SelectedThreeVal->setText(QString::number(TADDeng->ref_nb[3]));
		ui.label_SelectedFourVal->setText(QString::number(TADDeng->ref_nb[4]));
		ui.label_SelectedFiveVal->setText(QString::number(TADDeng->ref_nb[5]));
		ui.label_SelectedSixVal->setText(QString::number(TADDeng->ref_nb[6]));


}
}

void webcamUI::centerWidgetOnScreen(QWidget * widget) {
	///
	/// Positions the window
     QRect rect = QApplication::desktop()->availableGeometry(widget);

     widget->move(rect.center() - widget->rect().center());
}


void webcamUI::captureAction(){
	/**
	This function first copies the most recent webcam frame, detects what pixels 
	should be considered background, then calls TADDengine::SLIC to segment the image.

	Next it updates the overlay using TADDengine::initOverlay(), TADDengine::clearOverlay() 
	and then calls CvOverlayWidget::drawOverlay.

	It then calls TADDengine::updateMatches() to store a record of which segments are 
	considered to belong to the background and thus to be ignored.
	*/
	if (imageIdx<= TADDeng->capsTrained)
		imageIdx = TADDeng->capsTrained;
		//imageIdx=nextImageIdx++;

	    potatoCounter = 0;

		isCaptured = true;
		isUnselected = true;
		//generate superpixels
		//cvImage = TADDeng->SLIC(cvImage,0);

		//cvImage = cvLoadImage("potato.jpg",1);

		cvResize(cvImage,cvCaptured,0);

		QString fn = qApp->applicationDirPath() + "/Workspace/current.png";
		//msgBox.setText(fn);
		//msgBox.exec();

		cvSaveImage(fn.toAscii(),cvCaptured,0);

		//draw frame onto videoWidget
		ui.videoWidget_Interactive->drawImage(cvCaptured);

		//////cvShowImage("1",cvCaptured);

		//filtering
		IplConvKernel* element3 = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);
		IplConvKernel* element2 = cvCreateStructuringElementEx(5,5,2,2,CV_SHAPE_ELLIPSE);
		IplConvKernel* element1 = cvCreateStructuringElementEx(11,11,5,5,CV_SHAPE_ELLIPSE);

		IplImage* noise = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
		IplImage* threshold = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
		IplImage* cvtemp = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
		

		cvCvtColor( cvCaptured, noise, CV_RGB2GRAY );

		//////cvShowImage("2",noise);
		
		cvCopyImage(noise,threshold);

		cvThreshold(threshold, threshold, thresholdValue, 255, CV_THRESH_BINARY);

		////cvShowImage("3",threshold);

		////cvShowImage("threshold",threshold);

		//cvMorphologyEx(threshold, threshold, cvtemp, element3, CV_MOP_OPEN, 5);
		//cvMorphologyEx(threshold, threshold, cvtemp, element2, CV_MOP_CLOSE, 5);

		////cvShowImage("testing2",threshold);

		  CvMemStorage* storage = cvCreateMemStorage(0);

		   IplImage* potatoes = cvCreateImage( cvGetSize(threshold), 8, 3 );
		    IplImage* potato = cvCreateImage( cvGetSize(threshold), 8, 3 );

		    CvSeq* contour = 0;
		cvFindContours( threshold, storage, &contour, sizeof(CvContour),
           CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1 );
        cvZero( potatoes );
		  

		double area;
		double size = areasizeValue;


        while(contour!=NULL) {
			area = cvContourArea(contour, CV_WHOLE_SEQ, 1);
			if (-size <= area)
			{
				cvDrawContours( potatoes, contour, CV_RGB(0,0,0), CV_RGB(0,0,0), 1, CV_FILLED, 8 );
				
			}
			else
			{
				cvZero( potato );
				cvZero(cvtemp);
				cvSet(potato, cvScalar(0,0,0));

				cvDrawContours(potato, contour, CV_RGB(0xff,0xff,0xff),
                     CV_RGB(0,0,0), 0, CV_FILLED, CV_AA, cvPoint(0,0));

				////cvShowImage("4a",potato);

				cvMorphologyEx(potato, potato, cvtemp, element1, CV_MOP_CLOSE, 1);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);

				////cvShowImage("4b",potato);

				cvAdd(potato,potatoes,potatoes,NULL);
			}

			contour = contour->h_next; //get next contour

        }

		cvCvtColor( potatoes, noise, CV_RGB2GRAY );
		
		cvThreshold(noise, threshold, 0, 255, CV_THRESH_BINARY);

		////cvShowImage("threshold2",threshold);


		for (int h = 0; h < cvCaptured->height; h++) {
			for (int w = 0; w < cvCaptured->width; w++) {
				int Index = h*cvCaptured->width+w;

				if(threshold->imageData[Index] != -1){
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0] = 0; //R
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1] = 0; //G
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2] = 0; //B
				}
			}
		}

		//potato counter hack
		cvFindContours( threshold, storage, &contour, sizeof(CvContour),
           CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		while(contour!=NULL){
			potatoCounter++;

			contour = contour->h_next; //get next contour
        }

		//generate superpixels
		cvCapturedbg = TADDeng->SLIC(cvCaptured ,0);
		//cvShowImage("image_cvCaptured", cvCaptured);


		ui.videoWidget_Interactive->drawImage(cvCapturedbg);//

		//clear overlay
		cvOverlay = TADDeng->initOverlay();
		cvOverlay = TADDeng->clearOverlay();

		ui.overlayWidget->drawOverlay(cvOverlay);

		//update matches
		TADDeng->updateMatches(cvCapturedbg,threshold);

		//clear data
		memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0


		ui.label_PotatoCount->setText(QString("%1").arg(potatoCounter));

		cvReleaseImage(&noise);
		cvReleaseImage(&threshold);
		cvReleaseImage(&cvtemp);
		cvReleaseImage(&potatoes);
		cvReleaseImage(&potato);
		cvReleaseMemStorage(&storage);
		cvReleaseStructuringElement(&element3);
		cvReleaseStructuringElement(&element2);
		cvReleaseStructuringElement(&element1);

}


void webcamUI::captureAction_NEW(){
	
	//captureAction();

}


void webcamUI::pauseAction(){
	///
	/// Not used
	if(!pause)
	{
		//clear overlay
		//initialise cvOverlay for selection process
		cvOverlay = TADDeng->initOverlay();
	}

	if(!pause)
	{
		//clear overlay
		//deitialise cvOverlay
		cvOverlay = TADDeng->clearOverlay();
	}

	//toggle pause mode
	pause = !pause;
}
void webcamUI::trainAction(){
	///
	///Calls TADDengine->refAdaBoost() to update the classifier with data from the 
	///current image. This call now involves an image identifier to allow re-training 
	///using a previously marked up image. 
	///
	///Next the function autosaves the current application status and copies small 
	///versions of the current image and markup to cvImageBucket and cvOverlayBucket 
	///to allow the user to review smaller versions of previously marked up images.

	if(isUnselected == false){

		storedData = true;
		if (imageIdx >= TADDeng->capsTrained && isMultipleImageClassifier==false)
			moved = -1;

		if (isMultipleImageClassifier==true && imageIdx-latestIMGIDX>1 )

		{imageIdx=latestIMGIDX+1;}

		

		TADDeng->refAdaboost(imageIdx);

		QString ofn = qApp->applicationDirPath() + "/Workspace/autosave.xml";
		msgBox.setText(ofn);
		//msgBox.exec();

		// autosave
		std::ofstream ofs(ofn.toAscii());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(TADDeng);

		// Save image
		QString ifn = qApp->applicationDirPath() + "/Workspace/" +QString::number(imageIdx) +".png";
		//msgBox.setText(ifn);

		QString olfn = qApp->applicationDirPath() + "/Workspace/o_" +QString::number(imageIdx) +".png";
		//msgBox.exec();
		cvSaveImage(ifn.toAscii(),cvCaptured);
		
		QString cmd = QString::fromAscii("copy ") +qApp->applicationDirPath() +"/Workspace/current.png \"" 
			+ ifn+"\"";

		cmd.replace(QString("/"), QString("\\"));
		cmd.append(QString(" /y"));

		msgBox.setText(cmd);
		//msgBox.exec();
		
		system(cmd.toAscii());
		cvSaveImage(olfn.toAscii(), cvOverlay);
		//cvShowImage("cvOverlay", cvOverlay);
		cvResize(cvCaptured,cvMiniOverlayImage,0);
		cvResize(cvOverlay,cvMiniOverlay,0);

		ui.videoWidget_Minioverlay->drawImage(cvMiniOverlayImage);
		ui.overlayWidget_Minioverlay->drawOverlay(cvMiniOverlay);

		//add to image bucket
		CvMat *matImage = cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		
		CvMat *matOverlay= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		cvConvert( cvMiniOverlayImage, matImage );
	//	cvSaveImage("xyz.png",matImage);
		cvConvert( cvMiniOverlay, matOverlay );
	//	cvSaveImage("xyz2.png",matOverlay);

		//copy data
		memcpy((unsigned char*)cvImageBucket + imageIdx*((1280 * 720) * 3),(unsigned char*)matImage->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvOverlayBucket + imageIdx*((1280 * 720) * 3),(unsigned char*)matOverlay->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvImageBucket + TADDeng->capsTrained*((1280 * 720) * 3),(unsigned char*)matImage->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvOverlayBucket + TADDeng->capsTrained*((1280 * 720) * 3),(unsigned char*)matOverlay->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		cvReleaseMat(&matImage); //release orginal
		cvReleaseMat(&matOverlay);
		latestIMGIDX=imageIdx;

		/*get data back
		CvMat *debug = cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		memcpy((unsigned char*)debug->data.ptr,(unsigned char*)cvImageBucket->data.ptr + 0*(cvImageBucket->dim[0].size * cvImageBucket->dim[0].step) + (cvImageBucket->dim[1].size * cvImageBucket->dim[1].step) + (cvImageBucket->dim[2].size * cvImageBucket->dim[2].step),cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		IplImage * cvImageDebug = cvCreateImage( cvSize( 1280,720 ), IPL_DEPTH_8U, 3);
		cvConvert( debug, cvImageDebug );
		//cvShowImage("test",cvImageDebug);
		*/

		
	}
}


void webcamUI::classifyAction(){

	/**
	*	This function is called in response to a click on the Classify button.
	*	This calls TADDeng.classifyOverlayAdaboost() to classify the current image, 
	*	then updates the UI labels with the appropriate outputs per class, 
	*	in absolute and percentage levels.
	*/

	//isClassify = !isClassify;

	cvOverlay = TADDeng->classifyOverlayAdaboost(true);
	ui.overlayWidget->drawOverlay(cvOverlay);

	int total = TADDeng->sign_nb[1] + TADDeng->sign_nb[2] + TADDeng->sign_nb[3] + TADDeng->sign_nb[4] + TADDeng->sign_nb[5] + TADDeng->sign_nb[6];

	ui.label_ClassifiedOneVal->setText(QString::number(TADDeng->sign_nb[1]));
	ui.label_ClassifiedTwoVal->setText(QString::number(TADDeng->sign_nb[2]));
	ui.label_ClassifiedThreeVal->setText(QString::number(TADDeng->sign_nb[3]));
	ui.label_ClassifiedFourVal->setText(QString::number(TADDeng->sign_nb[4]));
	ui.label_ClassifiedFiveVal->setText(QString::number(TADDeng->sign_nb[5]));
	ui.label_ClassifiedSixVal->setText(QString::number(TADDeng->sign_nb[6]));


	double OnePer = (double(TADDeng->sign_nb[1])/total)*100;
	double TwoPer = (double(TADDeng->sign_nb[2])/total)*100;
	double ThreePer = (double(TADDeng->sign_nb[3])/total)*100;
	double FourPer = (double(TADDeng->sign_nb[4])/total)*100;
	double FivePer = (double(TADDeng->sign_nb[5])/total)*100;
	double SixPer = (double(TADDeng->sign_nb[6])/total)*100;

	ui.label_ClassifiedOnePer->setText(QString::number(OnePer,'f',1) + "%");
	ui.label_ClassifiedTwoPer->setText(QString::number(TwoPer,'f',1) + "%");
	ui.label_ClassifiedThreePer->setText(QString::number(ThreePer,'f',1) + "%");
	ui.label_ClassifiedFourPer->setText(QString::number(FourPer,'f',1) + "%");
	ui.label_ClassifiedFivePer->setText(QString::number(FivePer,'f',1) + "%");
	ui.label_ClassifiedSixPer->setText(QString::number(SixPer,'f',1) + "%");

	if (logging)
	{
		updateLog();
	}
}

void webcamUI::removeBackgroundAction(){
	///
	///Not used
	
}

void webcamUI::BGs1Action(){
	///
	///Not used
	isBackground =0;
}

void webcamUI::BGs2Action(){
	///
	///Not used
	isBackground = 1;

}

void webcamUI::Reset(){

		//clear overlay
	cvOverlay = TADDeng->initOverlay();
	cvOverlay = TADDeng->clearOverlay();

	ui.overlayWidget->drawOverlay(cvOverlay);

	TADDeng->~TADDengine();
	TADDeng = new TADDengine(cvImage->width, cvImage->height);
	pause = false; //init on false
	isCaptured = false;
	isSelecting = false; //init off
	isClassify = false;
	isUnselected = true;
	isBackground = false;
	isLearned = false;

	ui.videoWidget_Interactive->resetInputContext();
	clearFolderTempl();
	clearFolder();
}

void webcamUI::editAction()
{
	if(isMultipleImageClassifier==false){

    isEdit=1;
	imageIdx =(TADDeng->capsTrained + moved);
	//for (int jj = 0; jj <=imageIdx;jj++) {
	QString infname = qApp->applicationDirPath() + "/Workspace/" +QString::number(moved+1) +".png";
	//if (NumberOfImages>1){QString infname=qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";}
	potatoCounter = 0;
	isCaptured = true;
	IplImage *tempimg;
	QString filename1 = qApp->applicationDirPath() + "/Workspace/" +QString::number(moved+1) +".png";
	//if (NumberOfImages>1){QString filename1=qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";}
	tempimg = cvLoadImage(filename1.toStdString().c_str(),1);
	cvCopy(tempimg,cvCaptured,0);
	IplConvKernel* element3 = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);
	IplConvKernel* element2 = cvCreateStructuringElementEx(5,5,2,2,CV_SHAPE_ELLIPSE);
	IplConvKernel* element1 = cvCreateStructuringElementEx(11,11,5,5,CV_SHAPE_ELLIPSE);
	IplImage* noise = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	IplImage* threshold = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	IplImage* cvtemp = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	cvCvtColor( cvCaptured, noise, CV_RGB2GRAY );
	cvCopyImage(noise,threshold);
	cvThreshold(threshold, threshold, thresholdValue, 255, CV_THRESH_BINARY);
	CvMemStorage* storage = cvCreateMemStorage(0);
	IplImage* potatoes = cvCreateImage( cvGetSize(threshold), 8, 3 );
	IplImage* potato = cvCreateImage( cvGetSize(threshold), 8, 3 );
	CvSeq* contour = 0;
	cvFindContours( threshold, storage, &contour, sizeof(CvContour),
    CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1 );
    cvZero( potatoes );
	double area;
	double size = areasizeValue;
	        while(contour!=NULL) {
			area = cvContourArea(contour, CV_WHOLE_SEQ, 1);
			if (-size <= area)
			{
				cvDrawContours( potatoes, contour, CV_RGB(0,0,0), CV_RGB(0,0,0), 1, CV_FILLED, 8 );
				
			}
			else
			{
				cvZero( potato );
				cvZero(cvtemp);
				cvSet(potato, cvScalar(0,0,0));
				cvDrawContours(potato, contour, CV_RGB(0xff,0xff,0xff),
                CV_RGB(0,0,0), 0, CV_FILLED, CV_AA, cvPoint(0,0));
				cvMorphologyEx(potato, potato, cvtemp, element1, CV_MOP_CLOSE, 1);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvAdd(potato,potatoes,potatoes,NULL);
			}

			contour = contour->h_next; //get next contour

        }

		cvCvtColor( potatoes, noise, CV_RGB2GRAY );
	
		cvThreshold(noise, threshold, 0, 255, CV_THRESH_BINARY);

		for (int h = 0; h < cvCaptured->height; h++) {
			for (int w = 0; w < cvCaptured->width; w++) {
				int Index = h*cvCaptured->width+w;

				if(threshold->imageData[Index] != -1){
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0] = 0; //R
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1] = 0; //G
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2] = 0; //B
				}
			}
		}

		//potato counter hack
		cvFindContours( threshold, storage, &contour, sizeof(CvContour),
           CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		while(contour!=NULL){
			potatoCounter++;

			contour = contour->h_next; //get next contour
        }

		//generate superpixels
		cvCapturedbg = TADDeng->SLIC(cvCaptured ,0);
		ui.videoWidget_Interactive->drawImage(cvCapturedbg);
		cvOverlay = TADDeng->initOverlay();
		cvOverlay = TADDeng->clearOverlay();
		QString filename = qApp->applicationDirPath() + "/Workspace/o_" +QString::number(moved+1) +".png";
		//if (NumberOfImages>1){QString filename= qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_markup.png";}
	    cvOverlay = cvLoadImage(filename.toStdString().c_str(),1);
		ui.overlayWidget->drawOverlay(cvOverlay);
		std::vector<vector<int>> labelIDs(TADDeng->Matches.size(),  vector<int> ( 8 ));
		int* labelMask = TADDeng->labelMask;
		//cvOverlay->widthStep=1;
		for(int v = 0; v < labelIDs.size(); v++){
				std::fill(labelIDs[v].begin(),labelIDs[v].end(),0);
			                                    }
		int XX=cvOverlay->height;
		int YY=cvOverlay->width;
				   
		tbb::parallel_for (int (0), XX, [&](int h) {
				for (int w = 0; w < YY; w++) {

				    int labelIndex = h*YY+w;
					int labelValue = labelMask[labelIndex];
					int R = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2];
					int G = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1];
			        int B = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0];
					if(R == 0 && G !=0 && B == 0) {labelIDs[labelValue][0]++;}
					if(R !=0 && G !=0 && B == 0) {labelIDs[labelValue][1]++;}
				   	if(R == 0 && G !=0 && B !=0) {labelIDs[labelValue][2]++;}
					if(R == 0 && G == 0 && B !=0) {labelIDs[labelValue][3]++;}
				   	if(R !=0 && G == 0 && B !=0) {labelIDs[labelValue][4]++;}
				    if(R !=0 && G !=0 && B !=0) {labelIDs[labelValue][5]++;}
					 labelIDs[labelValue][6] = h;
					 labelIDs[labelValue][7] = w;
				}
			});

							 
		    QPoint pixelpos;
			for(int id = 0; id < labelIDs.size(); id++){
			  pixelpos.setX(labelIDs[id][7]);
			  pixelpos.setY(labelIDs[id][6]);
			  int superpixelClass = 0;

			  if(labelIDs[id][0] > labelIDs[id][1] &&
				 labelIDs[id][0] > labelIDs[id][2] &&
				 labelIDs[id][0] > labelIDs[id][3] &&
				 labelIDs[id][0] > labelIDs[id][4] &&
				 labelIDs[id][0] > labelIDs[id][5]){
			 superpixelClass = 1;
			 }

			  if(labelIDs[id][1] > labelIDs[id][0] &&
				 labelIDs[id][1] > labelIDs[id][2] &&
				 labelIDs[id][1] > labelIDs[id][3] &&
				 labelIDs[id][1] > labelIDs[id][4] &&
				 labelIDs[id][1] > labelIDs[id][5]){
			 superpixelClass = 2;
			 }

			  if(labelIDs[id][2] > labelIDs[id][0] &&
				 labelIDs[id][2] > labelIDs[id][1] &&
				 labelIDs[id][2] > labelIDs[id][3] &&
				 labelIDs[id][2] > labelIDs[id][4] &&
				 labelIDs[id][2] > labelIDs[id][5]){
			 superpixelClass = 3;
			 }

		      if(labelIDs[id][3] > labelIDs[id][0] &&
				 labelIDs[id][3] > labelIDs[id][1] &&
				 labelIDs[id][3] > labelIDs[id][2] &&
				 labelIDs[id][3] > labelIDs[id][4] &&
				 labelIDs[id][3] > labelIDs[id][5]){
			 superpixelClass = 4;

			 }

			  if(labelIDs[id][4] > labelIDs[id][0] &&
				 labelIDs[id][4] > labelIDs[id][1] &&
				 labelIDs[id][4] > labelIDs[id][2] &&
				 labelIDs[id][4] > labelIDs[id][3] &&
				 labelIDs[id][4] > labelIDs[id][5]){
			 superpixelClass = 5;
			 }

		      if(labelIDs[id][5] > labelIDs[id][0] &&
				 labelIDs[id][5] > labelIDs[id][1] &&
				 labelIDs[id][5] > labelIDs[id][2] &&
				 labelIDs[id][5] > labelIDs[id][3] &&
				 labelIDs[id][5] > labelIDs[id][4]){
			 superpixelClass =6;
			 }

			 if(superpixelClass > 0){
				 cvOverlay = TADDeng->updateOverlay(pixelpos,superpixelClass);
			 }
			}


	  TADDeng->refAdaboost(moved+1);
	  TADDeng->updateMatches(cvCapturedbg,threshold);
      ui.label_PotatoCount->setText(QString("%1").arg(potatoCounter));
		
		//clear data
		//memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0
	  	// Loading the markup to edit is broken for now.
	    //cvOverlay = TADDeng->overlayFromBucket(imageIdx);
		
		cvReleaseImage(&noise);
		cvReleaseImage(&threshold);
		cvReleaseImage(&cvtemp);
		cvReleaseImage(&potatoes);
		cvReleaseImage(&potato);
		cvReleaseMemStorage(&storage);
		cvReleaseImage(&tempimg);
			//}
		}



	if (isMultipleImageClassifier==true && NumberOfImages>1){
		
	
		}

	  		
		



}

void webcamUI::previousAction(){
	///
	///Change which marked-up image is being viewed in the bottom left preview panel.
	//moved--;
	if((NumberOfImages + moved) > 0)
	{
	//clear data
	memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0		
	moved--;
	imageIdx =(TADDeng->capsTrained + moved);
    CvMat *matImage= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
	CvMat *matOverlay= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
	//memcpy((unsigned char*)matImage->data.ptr,(unsigned char*)cvImageBucket + ((NumberOfImages + moved)+1)*((576 * 720) * 3),cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
	//memcpy((unsigned char*)matOverlay->data.ptr,(unsigned char*)cvOverlayBucket + ((NumberOfImages + moved)+1)*((576 * 720) * 3),cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
	IplImage * cvMatImage = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);
	IplImage * cvMatOverlay = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);
	IplImage *tempimg;
	QString filename1 = qApp->applicationDirPath() + "/Workspace_Temp/" +QString::number(moved+1) +".png";
	tempimg  = cvLoadImage(filename1.toStdString().c_str(),1);
	cvCopy(tempimg,cvCaptured,0);
	cvCapturedbg = TADDeng->SLIC(cvCaptured ,0);
	QString filename = qApp->applicationDirPath() + "/Workspace_Temp/o_" +QString::number(moved+1) +".png";
	cvOverlay = cvLoadImage(filename.toStdString().c_str(),1);
	cvImage = cvLoadImage(filename1.toStdString().c_str(),1);
	cvResize(tempimg,cvMatImage,0);
	cvResize(cvOverlay,cvMatOverlay,0);
	ui.videoWidget_Minioverlay->drawImage(cvMatImage);
	ui.overlayWidget_Minioverlay->drawOverlay(cvMatOverlay);
	QString fn1 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";
	cvSaveImage(fn1.toAscii(),cvCaptured,0);
	QString fn2 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_markup.png";
	cvSaveImage(fn2.toAscii(),cvOverlay,0);
//	moved--;
		        
    isEdit=1;
    QString infname = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";
	potatoCounter = 0;
	isCaptured = true;
//	IplImage *tempimg;
	QString filename12 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";
	tempimg = cvLoadImage(filename12.toStdString().c_str(),1);
	cvCopy(tempimg,cvCaptured,0);
	IplConvKernel* element3 = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);
	IplConvKernel* element2 = cvCreateStructuringElementEx(5,5,2,2,CV_SHAPE_ELLIPSE);
	IplConvKernel* element1 = cvCreateStructuringElementEx(11,11,5,5,CV_SHAPE_ELLIPSE);
	IplImage* noise = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	IplImage* threshold = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	IplImage* cvtemp = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	cvCvtColor( cvCaptured, noise, CV_RGB2GRAY );
	cvCopyImage(noise,threshold);
	cvThreshold(threshold, threshold, thresholdValue, 255, CV_THRESH_BINARY);
	CvMemStorage* storage = cvCreateMemStorage(0);
	IplImage* potatoes = cvCreateImage( cvGetSize(threshold), 8, 3 );
	IplImage* potato = cvCreateImage( cvGetSize(threshold), 8, 3 );
	CvSeq* contour = 0;
	cvFindContours( threshold, storage, &contour, sizeof(CvContour),
    CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1 );
    cvZero( potatoes );
	double area;
	double size = areasizeValue;
	        while(contour!=NULL) {
			area = cvContourArea(contour, CV_WHOLE_SEQ, 1);
			if (-size <= area)
			{
				cvDrawContours( potatoes, contour, CV_RGB(0,0,0), CV_RGB(0,0,0), 1, CV_FILLED, 8 );
				
			}
			else
			{
				cvZero( potato );
				cvZero(cvtemp);
				cvSet(potato, cvScalar(0,0,0));
				cvDrawContours(potato, contour, CV_RGB(0xff,0xff,0xff),
                CV_RGB(0,0,0), 0, CV_FILLED, CV_AA, cvPoint(0,0));
				cvMorphologyEx(potato, potato, cvtemp, element1, CV_MOP_CLOSE, 1);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvAdd(potato,potatoes,potatoes,NULL);
			}

			contour = contour->h_next; //get next contour

        }

		cvCvtColor( potatoes, noise, CV_RGB2GRAY );
	
		cvThreshold(noise, threshold, 0, 255, CV_THRESH_BINARY);

		for (int h = 0; h < cvCaptured->height; h++) {
			for (int w = 0; w < cvCaptured->width; w++) {
				int Index = h*cvCaptured->width+w;

				if(threshold->imageData[Index] != -1){
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0] = 0; //R
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1] = 0; //G
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2] = 0; //B
				}
			}
		}

		//potato counter hack
		cvFindContours( threshold, storage, &contour, sizeof(CvContour),
           CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		while(contour!=NULL){
			potatoCounter++;

			contour = contour->h_next; //get next contour
        }

		//generate superpixels
		cvCapturedbg = TADDeng->SLIC(cvCaptured ,0);
		ui.videoWidget_Interactive->drawImage(cvCapturedbg);
		//if (moved +1 <=1) {
		cvOverlay = TADDeng->initOverlay();
		
//		cvSet(cvOverlay , cvScalar(0,0,0));//}
		QString filename11 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_markup.png";



	    cvOverlay = cvLoadImage(filename11.toStdString().c_str(),1);
		ui.overlayWidget->drawOverlay(cvOverlay);
		std::vector<vector<int>> labelIDs(TADDeng->Matches.size(),  vector<int> ( 8 ));
		int* labelMask = TADDeng->labelMask;
		//cvOverlay->widthStep=1;
		for(int v = 0; v < labelIDs.size(); v++){
				std::fill(labelIDs[v].begin(),labelIDs[v].end(),0);
			                                    }
		int XX=cvOverlay->height;
		int YY=cvOverlay->width;
				   
		tbb::parallel_for (int (0), XX, [&](int h) {
				for (int w = 0; w < YY; w++) {

				    int labelIndex = h*YY+w;
					int labelValue = labelMask[labelIndex];
					int R = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2];
					int G = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1];
			        int B = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0];
					if(R == 0 && G !=0 && B == 0) {labelIDs[labelValue][0]++;}
					if(R !=0 && G !=0 && B == 0) {labelIDs[labelValue][1]++;}
				   	if(R == 0 && G !=0 && B !=0) {labelIDs[labelValue][2]++;}
					if(R == 0 && G == 0 && B !=0) {labelIDs[labelValue][3]++;}
				   	if(R !=0 && G == 0 && B !=0) {labelIDs[labelValue][4]++;}
				    if(R !=0 && G !=0 && B !=0) {labelIDs[labelValue][5]++;}
					 labelIDs[labelValue][6] = h;
					 labelIDs[labelValue][7] = w;
				}
			});

							 
		    QPoint pixelpos;
			for(int id = 0; id < labelIDs.size(); id++){
			  pixelpos.setX(labelIDs[id][7]);
			  pixelpos.setY(labelIDs[id][6]);
			  int superpixelClass = 0;

			  if(labelIDs[id][0] > labelIDs[id][1] &&
				 labelIDs[id][0] > labelIDs[id][2] &&
				 labelIDs[id][0] > labelIDs[id][3] &&
				 labelIDs[id][0] > labelIDs[id][4] &&
				 labelIDs[id][0] > labelIDs[id][5]){
			 superpixelClass = 1;
			 }

			  if(labelIDs[id][1] > labelIDs[id][0] &&
				 labelIDs[id][1] > labelIDs[id][2] &&
				 labelIDs[id][1] > labelIDs[id][3] &&
				 labelIDs[id][1] > labelIDs[id][4] &&
				 labelIDs[id][1] > labelIDs[id][5]){
			 superpixelClass = 2;
			 }

			  if(labelIDs[id][2] > labelIDs[id][0] &&
				 labelIDs[id][2] > labelIDs[id][1] &&
				 labelIDs[id][2] > labelIDs[id][3] &&
				 labelIDs[id][2] > labelIDs[id][4] &&
				 labelIDs[id][2] > labelIDs[id][5]){
			 superpixelClass = 3;
			 }

		      if(labelIDs[id][3] > labelIDs[id][0] &&
				 labelIDs[id][3] > labelIDs[id][1] &&
				 labelIDs[id][3] > labelIDs[id][2] &&
				 labelIDs[id][3] > labelIDs[id][4] &&
				 labelIDs[id][3] > labelIDs[id][5]){
			 superpixelClass = 4;

			 }

			  if(labelIDs[id][4] > labelIDs[id][0] &&
				 labelIDs[id][4] > labelIDs[id][1] &&
				 labelIDs[id][4] > labelIDs[id][2] &&
				 labelIDs[id][4] > labelIDs[id][3] &&
				 labelIDs[id][4] > labelIDs[id][5]){
			 superpixelClass = 5;
			 }

		      if(labelIDs[id][5] > labelIDs[id][0] &&
				 labelIDs[id][5] > labelIDs[id][1] &&
				 labelIDs[id][5] > labelIDs[id][2] &&
				 labelIDs[id][5] > labelIDs[id][3] &&
				 labelIDs[id][5] > labelIDs[id][4]){
			 superpixelClass =6;
			 }

			 if(superpixelClass > 0){
				 cvOverlay = TADDeng->updateOverlay(pixelpos,superpixelClass);
			 }
			}


	  //TADDeng->refAdaboost();
	//  if (isMultipleImageClassifier==true && NumberOfImages>1){imageIdx=moved+1;}
	//	TADDeng->refAdaboost(imageIdx);
	  TADDeng->updateMatches(cvCapturedbg,threshold);
      ui.label_PotatoCount->setText(QString("%1").arg(potatoCounter));
		
		//clear data
		//memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0
	  	// Loading the markup to edit is broken for now.
	    //cvOverlay = TADDeng->overlayFromBucket(imageIdx);
		
		cvReleaseImage(&noise);
		cvReleaseImage(&threshold);
		cvReleaseImage(&cvtemp);
		cvReleaseImage(&potatoes);
		cvReleaseImage(&potato);
		cvReleaseMemStorage(&storage);
		cvReleaseImage(&tempimg);
			//}
	}






	}
//}

void webcamUI::nextAction(){
	

	

	if ( moved +1 < NumberOfImages) 
	{
	
	//clear data
	memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0	
    //get data back
	CvMat *matImage= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
	CvMat *matOverlay= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
	//memcpy((unsigned char*)matImage->data.ptr,(unsigned char*)cvImageBucket + ((NumberOfImages + moved)+1)*((576 * 324) * 3),cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
	//memcpy((unsigned char*)matOverlay->data.ptr,(unsigned char*)cvOverlayBucket + ((NumberOfImages + moved)+1)*((576 * 324) * 3),cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
	IplImage * cvMatImage = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);
	IplImage * cvMatOverlay = cvCreateImage( cvSize( 576,324 ), IPL_DEPTH_8U, 3);
	IplImage *tempimg;
	QString filename1 = qApp->applicationDirPath() + "/Workspace_Temp/" +QString::number(moved+1) +".png";
	tempimg  = cvLoadImage(filename1.toStdString().c_str(),1);
	cvCopy(tempimg,cvCaptured,0);
	cvCapturedbg = TADDeng->SLIC(cvCaptured ,0);
	QString filename = qApp->applicationDirPath() + "/Workspace_Temp/o_" +QString::number(moved+1) +".png";
	cvOverlay = cvLoadImage(filename.toStdString().c_str(),1);
	cvImage = cvLoadImage(filename1.toStdString().c_str(),1);

	

	cvResize(tempimg,cvMatImage,0);
	cvResize(cvOverlay,cvMatOverlay,0);
	ui.videoWidget_Minioverlay->drawImage(cvMatImage);
	ui.overlayWidget_Minioverlay->drawOverlay(cvMatOverlay);
	QString fn1 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";
	cvSaveImage(fn1.toAscii(),cvCaptured,0);
	QString fn2 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_markup.png";
	cvSaveImage(fn2.toAscii(),cvOverlay,0);
	moved++;
	imageIdx =moved;	        
    isEdit=1;
    QString infname = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";
	potatoCounter = 0;
	isCaptured = true;
//	IplImage *tempimg;
	QString filename12 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_image.png";
	tempimg = cvLoadImage(filename12.toStdString().c_str(),1);
	cvCopy(tempimg,cvCaptured,0);
	IplConvKernel* element3 = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);
	IplConvKernel* element2 = cvCreateStructuringElementEx(5,5,2,2,CV_SHAPE_ELLIPSE);
	IplConvKernel* element1 = cvCreateStructuringElementEx(11,11,5,5,CV_SHAPE_ELLIPSE);
	IplImage* noise = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	IplImage* threshold = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	IplImage* cvtemp = cvCreateImage(cvGetSize(cvCapturedbg),  IPL_DEPTH_8U, 1);
	cvCvtColor( cvCaptured, noise, CV_RGB2GRAY );
	cvCopyImage(noise,threshold);
	cvThreshold(threshold, threshold, thresholdValue, 255, CV_THRESH_BINARY);
	CvMemStorage* storage = cvCreateMemStorage(0);
	IplImage* potatoes = cvCreateImage( cvGetSize(threshold), 8, 3 );
	IplImage* potato = cvCreateImage( cvGetSize(threshold), 8, 3 );
	CvSeq* contour = 0;
	cvFindContours( threshold, storage, &contour, sizeof(CvContour),
    CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1 );
    cvZero( potatoes );
	double area;
	double size = areasizeValue;
	        while(contour!=NULL) {
			area = cvContourArea(contour, CV_WHOLE_SEQ, 1);
			if (-size <= area)
			{
				cvDrawContours( potatoes, contour, CV_RGB(0,0,0), CV_RGB(0,0,0), 1, CV_FILLED, 8 );
				
			}
			else
			{
				cvZero( potato );
				cvZero(cvtemp);
				cvSet(potato, cvScalar(0,0,0));
				cvDrawContours(potato, contour, CV_RGB(0xff,0xff,0xff),
                CV_RGB(0,0,0), 0, CV_FILLED, CV_AA, cvPoint(0,0));
				cvMorphologyEx(potato, potato, cvtemp, element1, CV_MOP_CLOSE, 1);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 10);
				cvAdd(potato,potatoes,potatoes,NULL);
			}

			contour = contour->h_next; //get next contour

        }

		cvCvtColor( potatoes, noise, CV_RGB2GRAY );
	
		cvThreshold(noise, threshold, 0, 255, CV_THRESH_BINARY);

		for (int h = 0; h < cvCaptured->height; h++) {
			for (int w = 0; w < cvCaptured->width; w++) {
				int Index = h*cvCaptured->width+w;

				if(threshold->imageData[Index] != -1){
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0] = 0; //R
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1] = 0; //G
					cvCaptured->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2] = 0; //B
				}
			}
		}

		//potato counter hack
		cvFindContours( threshold, storage, &contour, sizeof(CvContour),
           CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

		while(contour!=NULL){
			potatoCounter++;

			contour = contour->h_next; //get next contour
        }

		//generate superpixels
		cvCapturedbg = TADDeng->SLIC(cvCaptured ,0);
		ui.videoWidget_Interactive->drawImage(cvCapturedbg);
		//if (moved +1 <=1) {
		cvOverlay = TADDeng->initOverlay();
		
//		cvSet(cvOverlay , cvScalar(0,0,0));//}
		QString filename11 = qApp->applicationDirPath() + "/Workspace_Temp/currently_loaded_markup.png";



	    cvOverlay = cvLoadImage(filename11.toStdString().c_str(),1);
		ui.overlayWidget->drawOverlay(cvOverlay);
		std::vector<vector<int>> labelIDs(TADDeng->Matches.size(),  vector<int> ( 8 ));
		int* labelMask = TADDeng->labelMask;
		//cvOverlay->widthStep=1;
		for(int v = 0; v < labelIDs.size(); v++){
				std::fill(labelIDs[v].begin(),labelIDs[v].end(),0);
			                                    }
		int XX=cvOverlay->height;
		int YY=cvOverlay->width;
				   
		tbb::parallel_for (int (0), XX, [&](int h) {
				for (int w = 0; w < YY; w++) {

				    int labelIndex = h*YY+w;
					int labelValue = labelMask[labelIndex];
					int R = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 2];
					int G = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 1];
			        int B = cvOverlay->imageData[h*cvCaptured->widthStep + w*cvCaptured->nChannels + 0];
					if(R == 0 && G !=0 && B == 0) {labelIDs[labelValue][0]++;}
					if(R !=0 && G !=0 && B == 0) {labelIDs[labelValue][1]++;}
				   	if(R == 0 && G !=0 && B !=0) {labelIDs[labelValue][2]++;}
					if(R == 0 && G == 0 && B !=0) {labelIDs[labelValue][3]++;}
				   	if(R !=0 && G == 0 && B !=0) {labelIDs[labelValue][4]++;}
				    if(R !=0 && G !=0 && B !=0) {labelIDs[labelValue][5]++;}
					 labelIDs[labelValue][6] = h;
					 labelIDs[labelValue][7] = w;
				}
			});

							 
		    QPoint pixelpos;
			for(int id = 0; id < labelIDs.size(); id++){
			  pixelpos.setX(labelIDs[id][7]);
			  pixelpos.setY(labelIDs[id][6]);
			  int superpixelClass = 0;

			  if(labelIDs[id][0] > labelIDs[id][1] &&
				 labelIDs[id][0] > labelIDs[id][2] &&
				 labelIDs[id][0] > labelIDs[id][3] &&
				 labelIDs[id][0] > labelIDs[id][4] &&
				 labelIDs[id][0] > labelIDs[id][5]){
			 superpixelClass = 1;
			 }

			  if(labelIDs[id][1] > labelIDs[id][0] &&
				 labelIDs[id][1] > labelIDs[id][2] &&
				 labelIDs[id][1] > labelIDs[id][3] &&
				 labelIDs[id][1] > labelIDs[id][4] &&
				 labelIDs[id][1] > labelIDs[id][5]){
			 superpixelClass = 2;
			 }

			  if(labelIDs[id][2] > labelIDs[id][0] &&
				 labelIDs[id][2] > labelIDs[id][1] &&
				 labelIDs[id][2] > labelIDs[id][3] &&
				 labelIDs[id][2] > labelIDs[id][4] &&
				 labelIDs[id][2] > labelIDs[id][5]){
			 superpixelClass = 3;
			 }

		      if(labelIDs[id][3] > labelIDs[id][0] &&
				 labelIDs[id][3] > labelIDs[id][1] &&
				 labelIDs[id][3] > labelIDs[id][2] &&
				 labelIDs[id][3] > labelIDs[id][4] &&
				 labelIDs[id][3] > labelIDs[id][5]){
			 superpixelClass = 4;

			 }

			  if(labelIDs[id][4] > labelIDs[id][0] &&
				 labelIDs[id][4] > labelIDs[id][1] &&
				 labelIDs[id][4] > labelIDs[id][2] &&
				 labelIDs[id][4] > labelIDs[id][3] &&
				 labelIDs[id][4] > labelIDs[id][5]){
			 superpixelClass = 5;
			 }

		      if(labelIDs[id][5] > labelIDs[id][0] &&
				 labelIDs[id][5] > labelIDs[id][1] &&
				 labelIDs[id][5] > labelIDs[id][2] &&
				 labelIDs[id][5] > labelIDs[id][3] &&
				 labelIDs[id][5] > labelIDs[id][4]){
			 superpixelClass =6;
			 }

			 if(superpixelClass > 0){
				 cvOverlay = TADDeng->updateOverlay(pixelpos,superpixelClass);
			 }
			}


	  //TADDeng->refAdaboost();
	//  if (isMultipleImageClassifier==true && NumberOfImages>1){imageIdx=moved+1;}
	//	TADDeng->refAdaboost(imageIdx);
	  TADDeng->updateMatches(cvCapturedbg,threshold);
      ui.label_PotatoCount->setText(QString("%1").arg(potatoCounter));
		
		//clear data
		//memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0
	  	// Loading the markup to edit is broken for now.
	    //cvOverlay = TADDeng->overlayFromBucket(imageIdx);
		
	   // memset(TADDeng->ref_nb,0,7 * sizeof(int));
		cvReleaseImage(&noise);
		cvReleaseImage(&threshold);
		cvReleaseImage(&cvtemp);
		cvReleaseImage(&potatoes);
		cvReleaseImage(&potato);
		cvReleaseMemStorage(&storage);
		cvReleaseImage(&tempimg);
			//}
	}




}



void webcamUI::nextAction_NEW(){
	}


void webcamUI::loadImage(){
	///
	///Loads an image as though it had been captured from the webcam. Requires 
	///the resolution to match.

	//open file dialog box
	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",tr("Images (*.png)"));
	if (filename.isNull())
		return;
	//copy loaded image in cvImage
	cvImage = cvLoadImage(filename.toStdString().c_str(),1);

	captureAction();

}



void webcamUI::saveAction(){
	///
	/// Not used
	QString filename = "C:\\Users\\sbcsr\\Desktop\\potato_mini" + QString("%1").arg(picCount) + ".jpg";

	cvSaveImage(filename.toStdString().c_str(),cvImageMini);

	picCount++;
}

void webcamUI::saveBigAction(){
	/**
		Saves the image from the larger view. By selecting the filter in the save dialog window
		this image can be saved as the camera image, the overlay or a mixture of both.
	*/

	QString filterImage = "Image only (*.png)";
	QString filterImageOverlay = "Image with Overlay (*.png)";
	QString filterOverlay = "Overlay only (*.png)";

	static int lastSel = 1;
	QString selFilter;

	switch(lastSel)
	{
	case 1:
		selFilter = filterImage;
		break;
	case 2:
		selFilter = filterImageOverlay;
		break;
	case 3:
		selFilter = filterOverlay;
		break;
	default:
		selFilter = filterImage;
		break;
	}

	QString filename = QFileDialog::getSaveFileName(this,"Save image and/or overlay",QString(),
		filterImage +";;"+ filterImageOverlay +";;"+ filterOverlay,&selFilter);

	if (filename.isNull())
	{
		return;
	}

	if (selFilter == filterImage)
	{
		lastSel = 1;
		cvSaveImage(filename.toAscii(),cvCaptured);
	}

	if (selFilter == filterImageOverlay)
	{
		lastSel = 2;
		IplImage *cvDst = cvCloneImage(cvCaptured);
		cvAddWeighted(cvCaptured,0.5,cvOverlay,0.5,0,cvDst);
		cvSaveImage(filename.toAscii(),cvDst);
		cvReleaseImage(&cvDst);
	}

	if (selFilter == filterOverlay)
	{
		lastSel = 3;
		cvSaveImage(filename.toAscii(),cvOverlay);
	}




	//QString filename =  "C:\\Users\\sbcsr\\Desktop\\potato_big" + QString("%1").arg(picCount1) + "_count" + QString("_%1").arg(potatoCounter) + ".jpg";

	//cvSaveImage(filename.toStdString().c_str(),cvCapturedbg);

	//picCount1++;
}

void webcamUI::saveBGBigAction(){
	///
	///Not used

	//QString filename = "C:\\Users\\sbcsr\\Desktop\\potato_big_BG" + QString("%1").arg(picCount2) + ".jpg";

	//cvSaveImage(filename.toStdString().c_str(),cvImage);

	//picCount2++;
}

void webcamUI::trainAllAction(){


	for (int ii = 0; ii < NumberOfImages; ii++) {
		nextAction();
		//if(isUnselected == false){

		storedData = true;
		if (imageIdx >= TADDeng->capsTrained && isMultipleImageClassifier==false)
			moved = -1;

		if (isMultipleImageClassifier==true && imageIdx-latestIMGIDX>1 )

		{imageIdx=latestIMGIDX+1;}

		

		TADDeng->refAdaboost(imageIdx);

		QString ofn = qApp->applicationDirPath() + "/Workspace/autosave.xml";
		msgBox.setText(ofn);
		//msgBox.exec();

		// autosave
		std::ofstream ofs(ofn.toAscii());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(TADDeng);

		// Save image
		QString ifn = qApp->applicationDirPath() + "/Workspace/" +QString::number(imageIdx) +".png";
		//msgBox.setText(ifn);

		QString olfn = qApp->applicationDirPath() + "/Workspace/o_" +QString::number(imageIdx) +".png";
		//msgBox.exec();
		cvSaveImage(ifn.toAscii(),cvCaptured);
		
		QString cmd = QString::fromAscii("copy ") +qApp->applicationDirPath() +"/Workspace/current.png \"" 
			+ ifn+"\"";

		cmd.replace(QString("/"), QString("\\"));
		cmd.append(QString(" /y"));

		msgBox.setText(cmd);
		//msgBox.exec();
		
		system(cmd.toAscii());
		cvSaveImage(olfn.toAscii(), cvOverlay);
		//cvShowImage("cvOverlay", cvOverlay);
		cvResize(cvCaptured,cvMiniOverlayImage,0);
		cvResize(cvOverlay,cvMiniOverlay,0);

		ui.videoWidget_Minioverlay->drawImage(cvMiniOverlayImage);
		ui.overlayWidget_Minioverlay->drawOverlay(cvMiniOverlay);

		//add to image bucket
		CvMat *matImage = cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		
		CvMat *matOverlay= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		cvConvert( cvMiniOverlayImage, matImage );
	//	cvSaveImage("xyz.png",matImage);
		cvConvert( cvMiniOverlay, matOverlay );
	//	cvSaveImage("xyz2.png",matOverlay);

		//copy data
		memcpy((unsigned char*)cvImageBucket + imageIdx*((1280 * 720) * 3),(unsigned char*)matImage->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvOverlayBucket + imageIdx*((1280 * 720) * 3),(unsigned char*)matOverlay->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvImageBucket + TADDeng->capsTrained*((1280 * 720) * 3),(unsigned char*)matImage->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvOverlayBucket + TADDeng->capsTrained*((1280 * 720) * 3),(unsigned char*)matOverlay->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		cvReleaseMat(&matImage); //release orginal
		cvReleaseMat(&matOverlay);
		latestIMGIDX=imageIdx;

		/*get data back
		CvMat *debug = cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		memcpy((unsigned char*)debug->data.ptr,(unsigned char*)cvImageBucket->data.ptr + 0*(cvImageBucket->dim[0].size * cvImageBucket->dim[0].step) + (cvImageBucket->dim[1].size * cvImageBucket->dim[1].step) + (cvImageBucket->dim[2].size * cvImageBucket->dim[2].step),cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		IplImage * cvImageDebug = cvCreateImage( cvSize( 1280,720 ), IPL_DEPTH_8U, 3);
		cvConvert( debug, cvImageDebug );
		//cvShowImage("test",cvImageDebug);
		*/

		
	//}
	}

			

}





//Radio buttons
void webcamUI::class1Action(){
	///
	/// Utility function sets selectedClass to the number of the control in the name.
	selectedClass =1;
}
void webcamUI::class2Action(){
	///
	/// Utility function sets selectedClass to the number of the control in the name.
	selectedClass=2;
}
void webcamUI::class3Action(){
	///
	/// Utility function sets selectedClass to the number of the control in the name.
	selectedClass=3;
}
void webcamUI::class4Action(){
	///
	/// Utility function sets selectedClass to the number of the control in the name.
	selectedClass=4;
}
void webcamUI::class5Action(){
	///
	/// Utility function sets selectedClass to the number of the control in the name.
	selectedClass=5;
}
void webcamUI::class6Action(){
	///
	/// Utility function sets selectedClass to the number of the control in the name.
	selectedClass=6;
}
//Radio buttons



void webcamUI::saveData(){
	/**
		Uses 7z.exe to copy the current state of the system to a .7z archive, including 
		previously marked-up images. This file is then renamed to .tadd
	*/
	QString filename = QFileDialog::getSaveFileName(this, tr("Save workspace"), "",tr("Files (*.tadd)"));

	if (filename.isEmpty())
		return;


	// If file exists, confirm to overwrite. Exit if no.

	QMessageBox confirm;
	confirm.setText("File exists");
	confirm.setInformativeText("Overwrite file?");
	confirm.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);

	QFile fout(filename);
	if (fout.exists())
	{
		int choice = confirm.exec();

		if (choice!=QMessageBox::Save)
			return;

		// If file exists, delete it.
		QString cmd = filename;
		cmd.replace("/","\\");
		cmd.prepend(" del /Q \"");
		cmd.append("\"");
		system(cmd.toAscii());
	}

	// Run 7z.exe -a filename qApp->path() \workfolder\*.*
	// need to know where 7z is. Set in a text file?

	QString cmd = qApp->applicationDirPath() + "/7z/7z.exe a -y \"" +filename+ "\" " +
		qApp->applicationDirPath() + "/Workspace/*.*";
	cmd.replace("/","\\");

	system(cmd.toAscii());


	//write
	/*{
		std::ofstream ofs("filename.xml");
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(TADDeng);
	}*/
/*
	{
		std::ofstream ofs2("filename.xml");
		boost::archive::xml_oarchive oa2(ofs2);
		oa2 << boost::serialization::make_nvp("BucketData",TADDeng->BucketData);
	}
	*/
	
}

void webcamUI::loadData(){
	/**
		Copies a .tadd file's contents back to the autosave folder, overwriting any 
		autosave that might have been there, then reloads the folder contents. If the 
		class variable autoloading is true then the first stage is skipped and the last 
		known state of the application is reloaded from the autosave folder.
	*/
	//TADDeng->~TADDengine();
	if (!autoloading)
	{
		// We are loading from a file so replace the autosave folder with the data from the file
		QString ifn = QFileDialog::getOpenFileName(this,tr("Load workspace"), "",tr("TADD files (*.tadd)"));
		if (ifn.isEmpty())
			return;
		QString cmd = QString::fromAscii("del \"") + qApp->applicationDirPath() + "/Workspace/*.*\"";
		cmd.replace("/","\\");
		cmd.append(" /y");
		system(cmd.toAscii());
		cmd = qApp->applicationDirPath() + "/7z/7z.exe e -y -o" + qApp->applicationDirPath() + "/Workspace/ \""
			+ifn +"\"";
		cmd.replace("/","\\");
		system(cmd.toAscii());
		QString cmd2 = QString::fromAscii("del \"") + qApp->applicationDirPath() + "/Workspace_Temp/*.*\"";
		cmd2.replace("/","\\");
		cmd2.append(" /y");
		system(cmd2.toAscii());
		cmd2 = qApp->applicationDirPath() + "/7z/7z.exe e -y -o" + qApp->applicationDirPath() + "/Workspace_Temp/ \""
			+ifn +"\"";
		cmd2.replace("/","\\");
		system(cmd2.toAscii());
	}
	QString xmlfname = qApp->applicationDirPath() + "/Workspace/autosave.xml";
	xmlfname.replace("/","\\");
	ifstream ifs(xmlfname.toAscii());
	assert(ifs.good());
	boost::archive::xml_iarchive ia(ifs);
	ia >> BOOST_SERIALIZATION_NVP(TADDeng);
    NumberOfImages=TADDeng->capsTrained;

	if(NumberOfImages<=1){
	loadFolder(qApp->applicationDirPath() + "/Workspace/");
	TADDeng->initLoad(true);


	TADDeng->initClassifers();
	autoloading = false;}

	
	if (NumberOfImages>1) {
		loadFolder(qApp->applicationDirPath() + "/Workspace_Temp/");
	TADDeng->initLoad(true);
	isMultipleImageClassifier=true;
	clearFolder();
	//TADDeng = new TADDengine(1280, 720);//1280x720 is the current resolution in TADD.
	TADDeng = new TADDengine(cvImage->width, cvImage->height);
	//TADDeng->~TADDengine();
	

	//clear data
		//memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0

	
	}// this flag controls if a classifier consists of more than one image


	
}


//ground truth hacky script

void webcamUI::groundTruth(){
	///
	///Not used

	//toggle diable timer ( will need to put if statement in timer)
	truthToggle = true;

	//Folder of Orginal images
	boost::filesystem3::path Original = "../Database/WhiteDatabase/Original";
	//boost::filesystem::path Original = "../Database/RedDatabase/Original";

	//Folder of Maps images
	boost::filesystem3::path Maps = "../Database/WhiteDatabase/Maps";
	//boost::filesystem::path Maps = "../Database/RedDatabase/Maps";

	//Use boost filesystem to build a directory list of both folders
	 try{

		  // does Orginal and Maps actually exist?
		  if (boost::filesystem::exists(Original) && boost::filesystem::exists(Maps)){

			  // Are they acutally directorys
			  if (boost::filesystem::is_directory(Original) && boost::filesystem::is_directory(Maps)){

				  // store paths si we cab sort them later
				   typedef vector<boost::filesystem::path> v;
				   v vOriginal;             
				   v vMaps;  

				  //make a copy
				  copy(boost::filesystem::directory_iterator(Original), boost::filesystem::directory_iterator(), back_inserter(vOriginal));
				  copy(boost::filesystem::directory_iterator(Maps), boost::filesystem::directory_iterator(), back_inserter(vMaps));

				  //sort as directory iteration may not order them correctly
				  //this is important becuase we will compare them later
				  sort(vOriginal.begin(), vOriginal.end());
				  sort(vMaps.begin(), vMaps.end());

				  //Do vOrginal and cMaps contain a list of file paths which are the same size? 1 Orginal for 1 map
				  if(vOriginal.size() == vMaps.size())
				  { 
					//if Identical create a list of file names to use later for loading images
					  //directory interation loop
					  for (v::const_iterator it(vOriginal.begin()), it_end(vOriginal.end()); it != it_end; ++it)
					  {
						boost::filesystem::path fn = it->filename();   // extract the filename from the path
						fileNames.push_back(fn); // push it into the vecor of file names
					  }
				  }
			  }
		  }
	 }
	 catch (const boost::filesystem::filesystem_error& ex)
	 {
	 }

	 //char* test = fileNames[0];
	 for(int i = 0; i < fileNames.size(); i++){
		wstring wstr = fileNames[i].wstring();
		std::string str(wstr.length()+16, ' ');
		std::copy(wstr.begin(), wstr.end(), str.begin());
		fileNamesC.push_back(str);
	 }

	 //crate string
	 string rootwDir = "..\\Database\\WhiteDatabase\\";
	 string rootrDir = "..\\Database\\RedDatabase\\";
	 string originalDir = "Original\\";
	 string mapsDir = "Maps\\";
 

    //load an image to get size data
	string filenameO = rootwDir + originalDir + fileNamesC[0];
	boost::trim(filenameO);
	cvOrginal = cvLoadImage(filenameO.c_str());

	//create holdr images
	IplImage* cvOrginalSLIC = cvCreateImage( cvSize( cvOrginal->width, cvOrginal->height ), IPL_DEPTH_8U, 3);

	//destroy TADDengine for webcam
	TADDeng->~TADDengine();
	//create new TADDengine for image size
	TADDeng = new TADDengine(cvOrginal->width, cvOrginal->height); //may need to change to pass superpixel amount

	//alocate memory
		//filtering
		IplConvKernel* element3 = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);
		IplConvKernel* element2 = cvCreateStructuringElementEx(6,6,2,2,CV_SHAPE_ELLIPSE);

		//use Maps maps image to create thrshold image and remove coin
		IplImage* noise = cvCreateImage(cvGetSize(cvMap),  IPL_DEPTH_8U, 1);
		IplImage* threshold = cvCreateImage(cvGetSize(cvMap),  IPL_DEPTH_8U, 1);
		IplImage* cvtemp = cvCreateImage(cvGetSize(cvMap),  IPL_DEPTH_8U, 1);
		IplImage* potatoes = cvCreateImage( cvGetSize(cvMap), 8, 3 );
		IplImage* potato = cvCreateImage( cvGetSize(cvMap), 8, 3 );

	   //loop for 1/4 of images
	for(int i =0; i < fileNamesC.size()/4; i++){
		//Load orginal into cvImage
		filenameO = rootwDir + originalDir + fileNamesC[i];
		boost::trim(filenameO);
		cvOrginal = cvLoadImage(filenameO.c_str());

		//Load map into cvMap
		string filenameM = rootwDir + mapsDir + fileNamesC[i];
		boost::trim(filenameM);
		boost::replace_first(filenameM, ".JPG", ".png");
		cvMap = cvLoadImage(filenameM.c_str());

		/*
		cvNamedWindow( "Example1", CV_WINDOW_AUTOSIZE );
		//cvShowImage("Example1", cvMap);
	
		int R = cvMap->imageData[1059*cvMap->widthStep + 2112*cvMap->nChannels + 2];
		int G = cvMap->imageData[1059*cvMap->widthStep + 2112*cvMap->nChannels + 1];
		int B = cvMap->imageData[1059*cvMap->widthStep + 2112*cvMap->nChannels + 0];

		int test = 0;
		*/
	
		//Call Capture Action //


			//copy map image
			cvCvtColor( cvMap, noise, CV_RGB2GRAY );
			cvCopyImage(noise,threshold);
			cvThreshold(threshold, threshold, 15, 255, CV_THRESH_BINARY);

			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;
			cvFindContours( threshold, storage, &contour, sizeof(CvContour),CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_L1 );
			cvZero( potatoes );
		  
			double area;
			double size = 7000; //edit this if coin is still visible

			while(contour!=NULL) {
				area = cvContourArea(contour, CV_WHOLE_SEQ, 1);
				if (-size <= area)
				{
					cvDrawContours( potatoes, contour, CV_RGB(0,0,0), CV_RGB(0,0,0), 1, CV_FILLED, 8 );
				}
				else
				{
					cvZero( potato );
					cvSet(potato, cvScalar(0,0,0));
					cvDrawContours(potato, contour, CV_RGB(0xff,0xff,0xff),CV_RGB(0,0,0), 0, CV_FILLED, 8, cvPoint(0,0));
					cvMorphologyEx(potato, potato, cvtemp, element3, CV_MOP_CLOSE, 100);
					cvAdd(potato,potatoes,potatoes,NULL);
				}
				contour = contour->h_next; //get next contour
			}

			cvCvtColor( potatoes, noise, CV_RGB2GRAY );
			cvCopyImage(noise,threshold);

			cvThreshold(threshold, threshold, 0, 255, CV_THRESH_BINARY);

			////cvShowImage("thresholded", threshold);

			for (int h = 0; h < cvOrginal->height; h++) {
				for (int w = 0; w < cvOrginal->width; w++) {
					int Index = h*cvOrginal->width+w;

					if(threshold->imageData[Index] != -1){
						cvOrginal->imageData[h*cvOrginal->widthStep + w*cvOrginal->nChannels + 0] = 0; //R
						cvOrginal->imageData[h*cvOrginal->widthStep + w*cvOrginal->nChannels + 1] = 0; //G
						cvOrginal->imageData[h*cvOrginal->widthStep + w*cvOrginal->nChannels + 2] = 0; //B
					}
				}
			}

			//generate superpixels
			 cvOrginalSLIC = TADDeng->SLIC(cvOrginal ,0);
			 //duplicate label data
			 int* labelMask = TADDeng->labelMask;
			 ////cvShowImage("superpixels", cvOrginalSLIC);

			//clear overlay
			cvOverlay = TADDeng->initOverlay();
			cvOverlay = TADDeng->clearOverlay();

			//update matches
			TADDeng->updateMatches(cvOrginal,threshold);

			//clear data
			memset(TADDeng->ref_nb,0,7 * sizeof(int)); //default to 0


		 //Nested loop for pixel location
		 //
			//get label ids (vector<vector>) (IDs, 5 Classes) = count  (row 5 and 6 = X and Y)
			std::vector<vector<int>> labelIDs(TADDeng->Matches.size(),  vector<int> ( 7 ));

			//ZERO entire vector
	
			for(int v = 0; v < labelIDs.size(); v++){
				std::fill(labelIDs[v].begin(),labelIDs[v].end(),0);
			}

		   //check X.Y postion for colour
       		for (int h = 0; h < cvOrginal->height; h++) {
				for (int w = 0; w < cvOrginal->width; w++) {
					int labelIndex = h*cvMap->width+w;
					int labelValue = labelMask[labelIndex];

					int R = cvMap->imageData[h*cvMap->widthStep + w*cvMap->nChannels + 2];
					int G = cvMap->imageData[h*cvMap->widthStep + w*cvMap->nChannels + 1];
					int B = cvMap->imageData[h*cvMap->widthStep + w*cvMap->nChannels + 0];
			

					//if map = (0,0,0) = class 1  // good potato
					 if(R !=0 & G !=0 & B !=0){
							 labelIDs[labelValue][0]++;
					 }
				
				   //if map = (255,0,0) = class 2 // black dot
					 if(R !=0 & G == 0 & B == 0){
							 labelIDs[labelValue][1]++;
					 }
				   //if map = (0,255,0) = class 3 // silver scurf
					if(R == 0 & G !=0 & B == 0){
							 labelIDs[labelValue][2]++;
					 }
				   //if map = (0,0,255) = class 4 // scab
					if(R == 0 & G == 0 & B !=0){
							 labelIDs[labelValue][3]++;
					 }
				   //if map = (255,255,0) = class 5 // greening 
					 if(R !=0 & G !=0 & B == 0){
							 labelIDs[labelValue][4]++;
					 }

					 labelIDs[labelValue][5] = h;
					 labelIDs[labelValue][6] = w;
				}
			}
	 
			QPoint pixelpos;
		  //for each id run below for highest class return
			for(int id = 0; id < labelIDs.size(); id++){
			 //create Qpoint from X and Y  ( row 6 and 7) = pixelPos
			 pixelpos.setX(labelIDs[id][6]);
			  pixelpos.setY(labelIDs[id][5]);

			 int superpixelClass = 0;

			 if(labelIDs[id][0] > labelIDs[id][1] &&
				 labelIDs[id][0] > labelIDs[id][2] &&
				 labelIDs[id][0] > labelIDs[id][3] &&
				 labelIDs[id][0] > labelIDs[id][4]){
			 superpixelClass = 1;
			 }

			 if(labelIDs[id][1] > labelIDs[id][0] &&
				 labelIDs[id][1] > labelIDs[id][2] &&
				 labelIDs[id][1] > labelIDs[id][3] &&
				 labelIDs[id][1] > labelIDs[id][4]){
			 superpixelClass = 2;
			 }

			 // y = height x = width
			 if(superpixelClass > 0){
				 cvOverlay = TADDeng->updateOverlay(pixelpos,superpixelClass);
			 }
			}


		//once markup is complete, train classifer
		  TADDeng->refAdaboost();
    }

	//dereference memeory
	cvReleaseImage(&noise);
	cvReleaseImage(&threshold);
	cvReleaseImage(&cvtemp);
	cvReleaseImage(&potato);
	cvReleaseImage(&potatoes);

/*
	//loop for next 1/4 of images
	  
	  //create vector size of number of images were using
	  //this will be for the output and analysis later
	  

	 //Load orginal into cvImage

	 //Load map into cvMap

	 //Call Capture Action

	 //clasiify
	  
	 //Nested loop for pixel location
        //
	   //check X.Y postion for colour
       
	   //create Qpoint from X and Y

	  //get label ids (vector<vector>) (IDs, 5 Classes) increase total for gruth

	  //what is the superpixels main class

	  // go over images
	  //get label id

	  //new vector of labels

	  // 0 the classifer got it correct
	  // 1-5 = what it should of been

	  //calcuate percentages


*/

	//toggle enable timer 
	truthToggle = false;
}

void webcamUI::updateThreshold(int value){
	///
	///Updates the sensitivity of the background detection algorithm.
		thresholdValue = value;
		ui.label_ThresholdValue->setText(QString::number(thresholdValue));

}

void webcamUI::updateArea(int value){
	///
	///Updates the minimum size of an area of non-background. When detecting background, 
	///areas smaller than value are considered noise.
		areasizeValue = value;
		ui.label_AreaValue->setText(QString::number(areasizeValue));
}

void webcamUI::scrollX(int value)
{
	///
	/// Scroll the main widget by the X axis only
	if (isCaptured!=1)
		return;
	ui.videoWidget_Interactive->setX(value);
	ui.overlayWidget->setX(value);
	
	//ui.statusLabel->setText(QString::fromAscii(" X: ") + QString::number(ui.hScroll->value()) + QString::fromAscii(" / ") + QString::number(ui.hScroll->maximum()) +
	//		QString::fromAscii(" Y: ") + QString::number(ui.vScroll->value()) + QString::fromAscii(" / ") + QString::number(ui.vScroll->maximum()));

}
void webcamUI::scrollY(int value)
{
	///
	/// Scroll the main widget by the Y axis only
	if (isCaptured!=1)
		return;
	ui.videoWidget_Interactive->setY(value);
	ui.overlayWidget->setY(value);
	
	//ui.statusLabel->setText(QString::fromAscii(" X: ") + QString::number(ui.hScroll->value()) + QString::fromAscii(" / ") + QString::number(ui.hScroll->maximum()) +
	//		QString::fromAscii(" Y: ") + QString::number(ui.vScroll->value()) + QString::fromAscii(" / ") + QString::number(ui.vScroll->maximum()));
}

void webcamUI::scrollXY(int x, int y)
{
	///
	/// Scroll the main widget by both axes
	if (isCaptured!=1)
		return;
	ui.videoWidget_Interactive->setXY(x,y);
	ui.overlayWidget->setXY(x,y);

}

void webcamUI::scaleslot(int value)
{
	///
	/// Handles the zooming of the main display widget.

	//msgBox.setText(QString::number(value));
	//msgBox.exec();

//	if (value<0)
//	{
	/*double hfrac, vfrac;

	if (ui.hScroll->maximum() > 0 && ui.vScroll->maximum() > 0)
	{
		hfrac = (double)(ui.hScroll->value()) / (double)(ui.hScroll->maximum());
		vfrac = (double)(ui.vScroll->value()) / (double)(ui.vScroll->maximum());
	}
	else
		hfrac = vfrac = 0;*/
		
		int hoff = ui.overlayWidget->getHOff(1.0/pow(2.0,value));
		int voff = ui.overlayWidget->getVOff(1.0/pow(2.0,value));

		hoff=voff=-1;
		
		ui.videoWidget_Interactive->setScale(pow(2.0,value),hoff,voff);
		ui.overlayWidget->setScale(pow(2.0,value),hoff,voff);

		float perc = 100.0 * pow(2.0,value);
		QString percString = QString::number((int) perc) + "%";
		ui.zoomLabel->setText(percString);
//	}
	//if (value>0)
	//{
	//	ui.videoWidget_Interactive->setScale(pow(2.0,value));
	//	ui.overlayWidget->setScale(pow(2.0,value));
	//}
	//if (value==0)
	//{
	//	ui.videoWidget_Interactive->setScale(1);
	//	ui.overlayWidget->setScale(1);
	//}
}

void webcamUI::zoomout()
{
	/**
	*	called by mousewheel events this is a simple utility to zoom a step.
	*/
	ui.zoomSlider->setValue(ui.zoomSlider->value()-1);
	scaleslot(ui.zoomSlider->value());
}
void webcamUI::zoomin()
{
	/**
	*	called mousewheel events this is a simple utility to zoom a step.
	*/
	ui.zoomSlider->setValue(ui.zoomSlider->value()+1);
	scaleslot(ui.zoomSlider->value());
}

void webcamUI::clearFolder()
{
	///
	/// Called to clear the workspace folder
	QString cmd = qApp->applicationDirPath();
	//cmd.replace(" ","\ ");
	cmd = cmd + "/Workspace/*.*\"";
	cmd.replace("/","\\");
	cmd.prepend("del /Q \"");
	system(cmd.toAscii());

}



void webcamUI::clearFolderTempl()
{		///
	/// Called to clear the workspace folder
	QString cmd2 = qApp->applicationDirPath();
	//cmd.replace(" ","\ ");
	cmd2 = cmd2 + "/Workspace_Temp/*.*\"";
	cmd2.replace("/","\\");
	cmd2.prepend("del /Q \"");
	system(cmd2.toAscii());
}
void webcamUI::loadFolder(QString folder)
{
	///
	/// When loading a previous session, this will detect and load formerly marked-up
	/// images from the folder, by default this is the Workspace folder into which a 
	/// save file has been unzipped (using 7-zip)
	QDir dr(folder);

	QStringList filters;
	filters << "*.png";

	dr.setNameFilters(filters);

	int maxname = -1;

	for (int i=0;i<dr.count();i++)
	{
		QString fname = dr.entryList()[i];

		int j = 1;
		bool ok;
		fname.left(j).toInt(&ok);
		while (ok)
		{
			// Only file names that START with a numeral are of interest

			if (fname.left(j).toInt() > maxname)
			{
				maxname = fname.left(j).toInt();
			}
			j++;
			fname.left(j).toInt(&ok);
		}
	}

	if (maxname !=0)
		return; // nothing to load

	for (int i = 0; i<maxname+1;i++)
	{
		QString fname = QString::number(i) + ".png";

		IplImage *cvCapturedTmp = cvLoadImage((folder + fname).toAscii());
		////cvShowImage("image_cvCapturedTmp", cvCapturedTmp);//////////////////////new
		IplImage *cvOverlayTmp = cvLoadImage((folder + "o_" + fname).toAscii());
		////cvShowImage("image_cvOverlayTmp", cvOverlayTmp);//////////////////////new
		cvResize(cvCapturedTmp,cvMiniOverlayImage,0);
		cvResize(cvOverlayTmp,cvMiniOverlay,0);

		ui.videoWidget_Minioverlay->drawImage(cvMiniOverlayImage);
		ui.overlayWidget_Minioverlay->drawOverlay(cvMiniOverlay);
		cvConvert( cvCapturedTmp, cvImage);////here
		////////ui.videoWidget_Interactive->drawImage(cvCaptured);////////////////new

		//add to image bucket
		CvMat *matImage = cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		CvMat *matOverlay= cvCreateMat(cvMiniOverlayImage->height,cvMiniOverlayImage->width,CV_8UC3 );
		cvConvert( cvMiniOverlayImage, matImage );
		


		cvConvert( cvMiniOverlay, matOverlay );
		////cvShowImage("image_cvMiniOverlay", cvMiniOverlay);///////////////////////////


		//copy data
		memcpy((unsigned char*)cvImageBucket + i*((1280 * 720) * 3),(unsigned char*)matImage->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		memcpy((unsigned char*)cvOverlayBucket + i*((1280 * 720) * 3),(unsigned char*)matOverlay->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		//memcpy((unsigned char*)cvImageBucket + TADDeng->capsTrained*((1280 * 720) * 3),(unsigned char*)matImage->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		//memcpy((unsigned char*)cvOverlayBucket + TADDeng->capsTrained*((1280 * 720) * 3),(unsigned char*)matOverlay->data.ptr,cvMiniOverlayImage->height*cvMiniOverlayImage->width*3);
		cvReleaseMat(&matImage); //release orginal
		cvReleaseMat(&matOverlay);
		cvReleaseImage(&cvCapturedTmp);
		cvReleaseImage(&cvOverlayTmp);
		
		cvImage = cvLoadImage((folder + fname).toAscii(),1);

	    captureAction_NEW();
        //captureAction();/////here

	}
}


webcamUI::~webcamUI() {
	///
	///Destructor
	//release camera
	cvCamera->~CvCamera();
}

bool webcamUI::openLog(QString fname)
{
	/**
	Creates or continues a log of classifications, detecting previous images which may have been logged
	to update the starting image number. Prints two newlines and two header lines including
	class names into a CSV file which Excel or similar can read.

	Logs are unique by file prefix and folder, so if you want to work in one folder, just give
	the logs different names. By default the prefix is the current date.
	*/
	FILE *fid = fopen(fname.toAscii(),"a");
	if (fid==NULL)
		return false;

	if (ui.chk_LogImgs->isChecked())
	{
		// are there any pre-existing logged images getting in the way?
		QDir dr(LogFolder);
		QStringList files = dr.entryList(QStringList(LogPrefix+"*.png"));
		if (!files.isEmpty())
		{
			int ix = 0;
			for (int i = 0;i<files.length();i++)
			{
				bool flag;
				files.at(i).mid(LogPrefix.length(),1).toInt(&flag);
				if (flag)
				{
					int num = files.at(i).mid(LogPrefix.length(),
						files.at(i).length() -(4 +LogPrefix.length())).toInt();
					if (num>ix)
						ix = num+1;
				}
			}
			if (ix>LogIdx)
				LogIdx = ix;
		}
	}

	// Print a header and set the filename
	QDateTime DateAndTime = QDateTime::currentDateTime();//print date and time in .csv file
	fprintf(fid,"\n\n %s,%s\n",DateAndTime.toString().toAscii().data(),ui.txtLogDesc->text().toAscii().data());
	fprintf(fid,"Index");
	for (int i = 0;i<classNames.length();i++)
	{
		fprintf(fid,",%s",classNames.at(i).toAscii().data());
	}

	fclose(fid);
	return true;
	//LogName = fname;
}

void webcamUI::updateLog()
{
	///
	/// Adds an entry to the log and stores images if appropriate.
	FILE *fid = fopen(LogName.toAscii(),"a");
	fprintf(fid,"\n%d,%s,%s,%s,%s,%s,%s",LogIdx,ui.label_ClassifiedOnePer->text().toAscii().data(),
		ui.label_ClassifiedTwoPer->text().toAscii().data(),ui.label_ClassifiedThreePer->text().toAscii().data(),
		ui.label_ClassifiedFourPer->text().toAscii().data(),ui.label_ClassifiedFivePer->text().toAscii().data(),
		ui.label_ClassifiedSixPer->text().toAscii().data());
	fclose(fid);

	if (ui.chk_LogImgs->isChecked())
	{
		cvSaveImage((LogFolder+"/"+LogPrefix + QString::number(LogIdx) + ".png").toAscii().data(),cvCaptured);
		
		IplImage *cvDst = cvCloneImage(cvCaptured);
		cvAddWeighted(cvCaptured,0.5,cvOverlay,0.5,0,cvDst);
		cvSaveImage((LogFolder+"/"+LogPrefix + QString::number(LogIdx) + "_labelled.png").toAscii().data(),cvDst);
		cvReleaseImage(&cvDst);
	}
	LogIdx++;
}


void webcamUI::openFolder()
{
	// Opens the current log folder
	QDesktopServices::openUrl(QUrl::fromLocalFile(ui.txtLogFolder->text()));
}

void webcamUI::logButton()
{
	if (logging)
	{
		logging = false;
		ui.btnLog->setText("Start Logging");
		return;
	}


	logging = true;

	LogFolder = ui.txtLogFolder->text();

	LogName = LogFolder +"/" + ui.txtLogPrefix->text() + "_Log.csv";
	LogPrefix = ui.txtLogPrefix->text() +"_";
	QFileInfo finfo(LogFolder);

	if (finfo.exists()==false)
	{
		QDir().mkpath(LogFolder);
	}
	else if (finfo.isDir()==false)
	{
		logging = false;
		msgBox.setText("Folder has the same name as a file");
		msgBox.exec();
		return;
	}

	openLog(LogName);

	ui.btnLog->setText("Stop Logging");
}

void webcamUI::logFolderButton()
{
	QString temp;
	temp = QFileDialog::getExistingDirectory(this);

	if (temp.isNull()==false)
	{
		ui.txtLogFolder->setText(temp);
	}
}