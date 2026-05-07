#include "levelselectui.h"
#include "ui_levelselectui.h"

LevelSelectUI::LevelSelectUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LevelSelectUI)
{

    ui->setupUi(this);
    setFixedSize(1402, 1122);

    // 按钮1 → 关卡1
    connect(ui->pushButton, &QPushButton::clicked, this, [this]() {
        emit levelSelected(1);
    });

    // 按钮2 → 关卡2
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]() {
        emit levelSelected(2);
    });

    // 按钮3 → 关卡3
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this]() {
        emit levelSelected(3);
    });

    // 按钮4 → 返回开始页
    connect(ui->pushButton_4, &QPushButton::clicked, this, [this]() {
        emit backToStartUI();
    });
}

LevelSelectUI::~LevelSelectUI()
{
    delete ui;
}

void LevelSelectUI::closeEvent(QCloseEvent *event)
{
    // 点击右上角X → 通知MainWindow处理（规则2：回到StartUI）
    emit closeRequested();
    event->ignore();  // 不关闭，由MainWindow控制
}
