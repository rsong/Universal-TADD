#include "CvOverlayWidget.h"

#include <QtCore/QtPlugin>
#include "CvOverlayWidgetPlugin.h"


CvOverlayWidgetPlugin::CvOverlayWidgetPlugin(QObject *parent)
	: QObject(parent)
{
	initialized = false;
}

void CvOverlayWidgetPlugin::initialize(QDesignerFormEditorInterface */*core*/)
{
	if (initialized)
		return;

	initialized = true;
}

bool CvOverlayWidgetPlugin::isInitialized() const
{
	return initialized;
}

QWidget *CvOverlayWidgetPlugin::createWidget(QWidget *parent)
{
	return new CvOverlayWidget(parent);
}

QString CvOverlayWidgetPlugin::name() const
{
	return "CvOverlayWidget";
}

QString CvOverlayWidgetPlugin::group() const
{
	return "My Plugins";
}

QIcon CvOverlayWidgetPlugin::icon() const
{
	return QIcon();
}

QString CvOverlayWidgetPlugin::toolTip() const
{
	return QString();
}

QString CvOverlayWidgetPlugin::whatsThis() const
{
	return QString();
}

bool CvOverlayWidgetPlugin::isContainer() const
{
	return false;
}

QString CvOverlayWidgetPlugin::domXml() const
{
	return "<widget class=\"CvOverlayWidget\" name=\"cvOverlayWidget\">\n"
		" <property name=\"geometry\">\n"
		"  <rect>\n"
		"   <x>0</x>\n"
		"   <y>0</y>\n"
		"   <width>100</width>\n"
		"   <height>100</height>\n"
		"  </rect>\n"
		" </property>\n"
		"</widget>\n";
}

QString CvOverlayWidgetPlugin::includeFile() const
{
	return "CvOverlayWidget.h";
}

Q_EXPORT_PLUGIN2(qtcvoverlaywidget, CvOverlayWidgetPlugin)
