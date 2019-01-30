#include "CvImageWidget.h"

#include <QtCore/QtPlugin>
#include "CvImageWidgetPlugin.h"


CvImageWidgetPlugin::CvImageWidgetPlugin(QObject *parent)
	: QObject(parent)
{
	initialized = false;
}

void CvImageWidgetPlugin::initialize(QDesignerFormEditorInterface */*core*/)
{
	if (initialized)
		return;

	initialized = true;
}

bool CvImageWidgetPlugin::isInitialized() const
{
	return initialized;
}

QWidget *CvImageWidgetPlugin::createWidget(QWidget *parent)
{
	return new CvImageWidget(parent);
}

QString CvImageWidgetPlugin::name() const
{
	return "CvImageWidget";
}

QString CvImageWidgetPlugin::group() const
{
	return "My Plugins";
}

QIcon CvImageWidgetPlugin::icon() const
{
	return QIcon();
}

QString CvImageWidgetPlugin::toolTip() const
{
	return QString();
}

QString CvImageWidgetPlugin::whatsThis() const
{
	return QString();
}

bool CvImageWidgetPlugin::isContainer() const
{
	return false;
}

QString CvImageWidgetPlugin::domXml() const
{
	return "<widget class=\"CvImageWidget\" name=\"cvImageWidget\">\n"
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

QString CvImageWidgetPlugin::includeFile() const
{
	return "CvImageWidget.h";
}

Q_EXPORT_PLUGIN2(qtcvimagewidget, CvImageWidgetPlugin)
