/********************************************************************************
** Form generated from reading UI file 'startui.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STARTUI_H
#define UI_STARTUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StartUI
{
public:
    QLabel *label;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QWidget *StartUI)
    {
        if (StartUI->objectName().isEmpty())
            StartUI->setObjectName("StartUI");
        StartUI->resize(1587, 991);
        label = new QLabel(StartUI);
        label->setObjectName("label");
        label->setGeometry(QRect(3, -5, 1587, 991));
        label->setPixmap(QPixmap(QString::fromUtf8(":/images/images/start_bg1.png")));
        pushButton = new QPushButton(StartUI);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(150, 730, 631, 151));
        pushButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,20); /* \351\274\240\346\240\207\346\202\254\345\201\234\350\275\273\345\276\256\351\253\230\344\272\256\357\274\214\345\217\257\351\200\211 */\n"
"}"));
        pushButton_2 = new QPushButton(StartUI);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(810, 723, 631, 161));
        pushButton_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,20); /* \351\274\240\346\240\207\346\202\254\345\201\234\350\275\273\345\276\256\351\253\230\344\272\256\357\274\214\345\217\257\351\200\211 */\n"
"}"));

        retranslateUi(StartUI);

        QMetaObject::connectSlotsByName(StartUI);
    } // setupUi

    void retranslateUi(QWidget *StartUI)
    {
        StartUI->setWindowTitle(QCoreApplication::translate("StartUI", "Form", nullptr));
        label->setText(QString());
        pushButton->setText(QString());
        pushButton_2->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class StartUI: public Ui_StartUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STARTUI_H
