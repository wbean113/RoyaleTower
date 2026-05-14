#include "startui.h"
#include "ui_startui.h"
#include <bgm.h>

#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QCloseEvent>

// 构造函数：初始化UI指针并调用setupUi
StartUI::StartUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StartUI)
{
    // 必须调用！把UI控件加载到当前窗口
    ui->setupUi(this);

    // 固定窗口尺寸匹配.ui设计尺寸（1587x991），禁止拉伸
    setFixedSize(1587, 991);

    // 背景QLabel禁止缩放内容，保持原图1:1居中显示
    ui->label->setScaledContents(false);
    ui->label->setPixmap(QPixmap(":/images/images/start_bg1.png"));

    // 窗口自动屏幕居中
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect screenGeo = screen->geometry();
        move((screenGeo.width() - width()) / 2,
             (screenGeo.height() - height()) / 2);
    }

    // 播放菜单音乐（随机music03或music04）
    BGMPlayer::instance().playRandomBGM({
        ":/music/music/music03.mp3",
        ":/music/music/music04.mp3"
    });

    // 设置窗口标题
    setWindowTitle("Royal Tower");
}

// 析构函数：释放UI指针
StartUI::~StartUI()
{
    delete ui;
}

// 按钮点击槽函数
void StartUI::on_pushButton_clicked()
{
    emit startGameRequested();
    this->hide(); // 隐藏开始页面（不关闭，规则1仅右上角X退出）
}

// 退出游戏按钮
void StartUI::on_pushButton_2_clicked()
{
    QApplication::quit();
}

void StartUI::closeEvent(QCloseEvent *event)
{
    // 规则1：关闭【开始页面StartUI】→ 直接退出程序
    QApplication::quit();
    event->accept();
}
