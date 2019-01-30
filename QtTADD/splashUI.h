#ifndef SPLASHUI_H
#define SPLASHUI_H

#include <QtGui/QWidget>
#include <QtGui/QDesktopWidget>

#include "ui_splashUI.h"

#include "webcamUI.h"

class splashUI : public QWidget
{
	Q_OBJECT

private:
	bool started;

public:
	splashUI(QWidget *parent = 0, Qt::WFlags flags = 0);
	~splashUI();

protected:


private:
	void centerWidgetOnScreen(QWidget * widget);

private:
	Ui::splashUI ui;
	webcamUI* webcamWin; 

protected slots:
	void webcamMode();

};

#endif // SPLASHUI_H
