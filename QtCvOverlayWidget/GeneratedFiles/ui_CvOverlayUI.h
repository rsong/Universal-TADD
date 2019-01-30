/********************************************************************************
** Form generated from reading UI file 'CvOverlayUI.ui'
**
** Created: Wed 16. Apr 15:33:59 2014
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CVOVERLAYUI_H
#define UI_CVOVERLAYUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CvOverlayWidget
{
public:
    QLabel *overlayLabel;

    void setupUi(QWidget *CvOverlayWidget)
    {
        if (CvOverlayWidget->objectName().isEmpty())
            CvOverlayWidget->setObjectName(QString::fromUtf8("CvOverlayWidget"));
        CvOverlayWidget->resize(400, 300);
        overlayLabel = new QLabel(CvOverlayWidget);
        overlayLabel->setObjectName(QString::fromUtf8("overlayLabel"));
        overlayLabel->setGeometry(QRect(0, 0, 46, 13));

        retranslateUi(CvOverlayWidget);

        QMetaObject::connectSlotsByName(CvOverlayWidget);
    } // setupUi

    void retranslateUi(QWidget *CvOverlayWidget)
    {
        CvOverlayWidget->setWindowTitle(QApplication::translate("CvOverlayWidget", "Form", 0, QApplication::UnicodeUTF8));
        overlayLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CvOverlayWidget: public Ui_CvOverlayWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CVOVERLAYUI_H
