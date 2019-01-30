/********************************************************************************
** Form generated from reading UI file 'CvImageUI.ui'
**
** Created: Wed 16. Apr 15:33:51 2014
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CVIMAGEUI_H
#define UI_CVIMAGEUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CvImageWidget
{
public:
    QLabel *imageLabel;

    void setupUi(QWidget *CvImageWidget)
    {
        if (CvImageWidget->objectName().isEmpty())
            CvImageWidget->setObjectName(QString::fromUtf8("CvImageWidget"));
        CvImageWidget->setEnabled(true);
        CvImageWidget->resize(400, 400);
        imageLabel = new QLabel(CvImageWidget);
        imageLabel->setObjectName(QString::fromUtf8("imageLabel"));
        imageLabel->setGeometry(QRect(0, 0, 46, 13));

        retranslateUi(CvImageWidget);

        QMetaObject::connectSlotsByName(CvImageWidget);
    } // setupUi

    void retranslateUi(QWidget *CvImageWidget)
    {
        CvImageWidget->setWindowTitle(QApplication::translate("CvImageWidget", "CvImageWidget", 0, QApplication::UnicodeUTF8));
        imageLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class CvImageWidget: public Ui_CvImageWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CVIMAGEUI_H
