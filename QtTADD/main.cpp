#include "splashUI.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	//splashUI w;
	webcamUI w;
	w.show();
	return a.exec();
}
