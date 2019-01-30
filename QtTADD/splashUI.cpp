#include "splashUI.h"

splashUI::splashUI(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	ui.setupUi(this);
	this->setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint );
	centerWidgetOnScreen(this);

	started = false;

	//webcamMode();

}

void splashUI::webcamMode() {

	started = true;

	//webcam window
	webcamWin = new webcamUI();

	//close splash window
	this->close();
	webcamWin->show();
}



void splashUI::centerWidgetOnScreen(QWidget * widget) {

     QRect rect = QApplication::desktop()->availableGeometry(widget);

     widget->move(rect.center() - widget->rect().center());
}

splashUI::~splashUI()
{
	if(started)
		webcamWin->~webcamUI();

}


