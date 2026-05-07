#include <QApplication>
#include <QScreen>
#include <QPointer>
#include <cstdlib>

#include "startui.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Qt6 高DPI适配：设置环境变量禁用自动缩放，保持96DPI基准
    // 必须在QApplication构造之前设置，确保所有界面不受系统缩放影响
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    // Qt6 中 AA_DisableHighDpiScaling / AA_Use96Dpi 已弃用，
    // 环境变量方式兼容且生效可靠

    QApplication a(argc, argv);

    // 启动时仅显示开始页面
    StartUI startPage;
    startPage.show();

    // 点击开始按钮 → 显示/创建主窗口，隐藏StartUI
    QPointer<MainWindow> mainWin;
    QObject::connect(&startPage, &StartUI::startGameRequested, [&]() {
        if (mainWin) {
            mainWin->show();
            mainWin->raise();
            startPage.hide();
            return;
        }
        mainWin = new MainWindow();
        mainWin->setAttribute(Qt::WA_DeleteOnClose);
        mainWin->show();
        startPage.hide();

        // MainWindow请求返回StartUI时，隐藏窗口并显示StartUI
        QObject::connect(mainWin.data(), &MainWindow::requestReturnToStart, [&]() {
            startPage.show();
        });

        // MainWindow销毁时重新显示StartUI
        QObject::connect(mainWin.data(), &QObject::destroyed, [&]() {
            startPage.show();
        });
    });

    return a.exec();
}