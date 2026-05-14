/********************************************************************************
** Form generated from reading UI file 'levelselectui.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LEVELSELECTUI_H
#define UI_LEVELSELECTUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LevelSelectUI
{
public:
    QLabel *label;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *btnDifficulty;

    void setupUi(QWidget *LevelSelectUI)
    {
        if (LevelSelectUI->objectName().isEmpty())
            LevelSelectUI->setObjectName("LevelSelectUI");
        LevelSelectUI->resize(1402, 1122);
        label = new QLabel(LevelSelectUI);
        label->setObjectName("label");
        label->setGeometry(QRect(0, 0, 1402, 1122));
        label->setPixmap(QPixmap(QString::fromUtf8(":/images/images/level_select_bg.png")));
        pushButton = new QPushButton(LevelSelectUI);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(240, 963, 191, 61));
        pushButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,20); /* \351\274\240\346\240\207\346\202\254\345\201\234\350\275\273\345\276\256\351\253\230\344\272\256\357\274\214\345\217\257\351\200\211 */\n"
"}"));
        pushButton_2 = new QPushButton(LevelSelectUI);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(470, 673, 171, 61));
        pushButton_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,20); /* \351\274\240\346\240\207\346\202\254\345\201\234\350\275\273\345\276\256\351\253\230\344\272\256\357\274\214\345\217\257\351\200\211 */\n"
"}"));
        pushButton_3 = new QPushButton(LevelSelectUI);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(860, 453, 151, 51));
        pushButton_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,20); /* \351\274\240\346\240\207\346\202\254\345\201\234\350\275\273\345\276\256\351\253\230\344\272\256\357\274\214\345\217\257\351\200\211 */\n"
"}"));
        pushButton_4 = new QPushButton(LevelSelectUI);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(150, 473, 71, 61));
        pushButton_4->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"	gridline-color: rgb(255, 151, 33);\n"
"    background-color: transparent;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,20); /* \351\274\240\346\240\207\346\202\254\345\201\234\350\275\273\345\276\256\351\253\230\344\272\256\357\274\214\345\217\257\351\200\211 */\n"
"}"));
        btnDifficulty = new QPushButton(LevelSelectUI);
        btnDifficulty->setObjectName("btnDifficulty");
        btnDifficulty->setGeometry(QRect(1090, 693, 120, 71));

        retranslateUi(LevelSelectUI);

        QMetaObject::connectSlotsByName(LevelSelectUI);
    } // setupUi

    void retranslateUi(QWidget *LevelSelectUI)
    {
        LevelSelectUI->setWindowTitle(QCoreApplication::translate("LevelSelectUI", "Form", nullptr));
        label->setText(QString());
        pushButton->setText(QString());
        pushButton_2->setText(QString());
        pushButton_3->setText(QString());
        pushButton_4->setText(QCoreApplication::translate("LevelSelectUI", "\350\277\224\345\233\236", nullptr));
        btnDifficulty->setText(QCoreApplication::translate("LevelSelectUI", "\351\232\276\345\272\246", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LevelSelectUI: public Ui_LevelSelectUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LEVELSELECTUI_H
