/********************************************************************************
** Form generated from reading UI file 'splashUI.ui'
**
** Created: Thu 29. May 14:28:01 2014
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SPLASHUI_H
#define UI_SPLASHUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_splashUI
{
public:
    QTabWidget *tabWidget;
    QWidget *tab_new;
    QPushButton *webcamButton_new;
    QLabel *label_name_new;
    QLabel *label_create_new;
    QLineEdit *lineEdit_name_new;
    QLineEdit *lineEdit_create_new;
    QPushButton *button_browse_new;
    QWidget *tab_open;
    QPushButton *webcamButton_open;
    QLabel *label_name_open;
    QLabel *label_create_open;
    QLineEdit *lineEdit_name_open;
    QLineEdit *lineEdit_locate_open;
    QPushButton *button_browse_open;

    void setupUi(QWidget *splashUI)
    {
        if (splashUI->objectName().isEmpty())
            splashUI->setObjectName(QString::fromUtf8("splashUI"));
        splashUI->resize(680, 215);
        tabWidget = new QTabWidget(splashUI);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 661, 200));
        tab_new = new QWidget();
        tab_new->setObjectName(QString::fromUtf8("tab_new"));
        webcamButton_new = new QPushButton(tab_new);
        webcamButton_new->setObjectName(QString::fromUtf8("webcamButton_new"));
        webcamButton_new->setGeometry(QRect(510, 120, 141, 50));
        webcamButton_new->setMinimumSize(QSize(120, 50));
        label_name_new = new QLabel(tab_new);
        label_name_new->setObjectName(QString::fromUtf8("label_name_new"));
        label_name_new->setGeometry(QRect(30, 50, 46, 13));
        label_create_new = new QLabel(tab_new);
        label_create_new->setObjectName(QString::fromUtf8("label_create_new"));
        label_create_new->setGeometry(QRect(30, 80, 91, 16));
        lineEdit_name_new = new QLineEdit(tab_new);
        lineEdit_name_new->setObjectName(QString::fromUtf8("lineEdit_name_new"));
        lineEdit_name_new->setGeometry(QRect(130, 50, 321, 20));
        lineEdit_create_new = new QLineEdit(tab_new);
        lineEdit_create_new->setObjectName(QString::fromUtf8("lineEdit_create_new"));
        lineEdit_create_new->setGeometry(QRect(130, 80, 321, 20));
        button_browse_new = new QPushButton(tab_new);
        button_browse_new->setObjectName(QString::fromUtf8("button_browse_new"));
        button_browse_new->setGeometry(QRect(460, 80, 75, 23));
        tabWidget->addTab(tab_new, QString());
        tab_open = new QWidget();
        tab_open->setObjectName(QString::fromUtf8("tab_open"));
        webcamButton_open = new QPushButton(tab_open);
        webcamButton_open->setObjectName(QString::fromUtf8("webcamButton_open"));
        webcamButton_open->setGeometry(QRect(510, 120, 141, 50));
        webcamButton_open->setMinimumSize(QSize(120, 50));
        label_name_open = new QLabel(tab_open);
        label_name_open->setObjectName(QString::fromUtf8("label_name_open"));
        label_name_open->setGeometry(QRect(30, 50, 46, 13));
        label_create_open = new QLabel(tab_open);
        label_create_open->setObjectName(QString::fromUtf8("label_create_open"));
        label_create_open->setGeometry(QRect(30, 80, 91, 16));
        lineEdit_name_open = new QLineEdit(tab_open);
        lineEdit_name_open->setObjectName(QString::fromUtf8("lineEdit_name_open"));
        lineEdit_name_open->setGeometry(QRect(110, 50, 321, 20));
        lineEdit_locate_open = new QLineEdit(tab_open);
        lineEdit_locate_open->setObjectName(QString::fromUtf8("lineEdit_locate_open"));
        lineEdit_locate_open->setGeometry(QRect(110, 80, 321, 20));
        button_browse_open = new QPushButton(tab_open);
        button_browse_open->setObjectName(QString::fromUtf8("button_browse_open"));
        button_browse_open->setGeometry(QRect(440, 80, 75, 23));
        tabWidget->addTab(tab_open, QString());

        retranslateUi(splashUI);
        QObject::connect(webcamButton_new, SIGNAL(clicked()), splashUI, SLOT(webcamMode()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(splashUI);
    } // setupUi

    void retranslateUi(QWidget *splashUI)
    {
        splashUI->setWindowTitle(QApplication::translate("splashUI", "Session Window", 0, QApplication::UnicodeUTF8));
        webcamButton_new->setText(QApplication::translate("splashUI", "Create Session", 0, QApplication::UnicodeUTF8));
        label_name_new->setText(QApplication::translate("splashUI", "Name:", 0, QApplication::UnicodeUTF8));
        label_create_new->setText(QApplication::translate("splashUI", "Create in Folder:", 0, QApplication::UnicodeUTF8));
        button_browse_new->setText(QApplication::translate("splashUI", "Browse...", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_new), QApplication::translate("splashUI", "New Session", 0, QApplication::UnicodeUTF8));
        webcamButton_open->setText(QApplication::translate("splashUI", "Open Session", 0, QApplication::UnicodeUTF8));
        label_name_open->setText(QApplication::translate("splashUI", "Name:", 0, QApplication::UnicodeUTF8));
        label_create_open->setText(QApplication::translate("splashUI", "Open Session:", 0, QApplication::UnicodeUTF8));
        button_browse_open->setText(QApplication::translate("splashUI", "Browse...", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_open), QApplication::translate("splashUI", "Open Session", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class splashUI: public Ui_splashUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPLASHUI_H
