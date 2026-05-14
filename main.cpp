#include <QApplication>
#include <QScreen>
#include <QPointer>
#include <cstdlib>

#include "startui.h"
#include "mainwindow.h"
#include "levelselectui.h"

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

    // LevelSelectUI：独立窗口，由main.cpp管理生命周期
    QPointer<LevelSelectUI> levelSelectUI;

    // MainWindow：游戏窗口，用完即销毁
    QPointer<MainWindow> mainWin;

    // ───────────────────────────────────────
    // StartUI → LevelSelectUI
    // ───────────────────────────────────────
    QObject::connect(&startPage, &StartUI::startGameRequested, [&]() {
        if (!levelSelectUI) {
            levelSelectUI = new LevelSelectUI();
            levelSelectUI->setWindowTitle("Royal Tower");
        }

        // --- LevelSelectUI → 选择关卡 ---
        QObject::connect(levelSelectUI.data(), &LevelSelectUI::levelSelected,
                         [&](int level) {
            levelSelectUI->hide();

            // 销毁旧的游戏窗口
            if (mainWin) {
                mainWin->disconnect();
                mainWin->close();
                mainWin = nullptr;
            }

            mainWin = new MainWindow();
            mainWin->setAttribute(Qt::WA_DeleteOnClose);

            // 从关卡选择界面读取用户选择的难度
            mainWin->setDifficulty(levelSelectUI->selectedDifficulty());

            mainWin->show();
            mainWin->startGame(level);

            // 游戏内"返回菜单" → 显示关卡选择
            QObject::connect(mainWin.data(), &MainWindow::requestShowLevelSelect, [&]() {
                if (levelSelectUI)
                    levelSelectUI->show();
            });

            // MainWindow X关闭 → 回到StartUI
            QObject::connect(mainWin.data(), &MainWindow::requestReturnToStart, [&]() {
                startPage.show();
            });

            // MainWindow被销毁时也回到StartUI（兜底）
            QObject::connect(mainWin.data(), &QObject::destroyed, [&]() {
                // 如果levelSelectUI可见说明是返回菜单路径，否则回StartUI
                if (levelSelectUI && levelSelectUI->isVisible()) {
                    // 返回菜单 → 不额外处理
                } else {
                    startPage.show();
                }
            });
        });

        // --- LevelSelectUI → 返回开始页 ---
        QObject::connect(levelSelectUI.data(), &LevelSelectUI::backToStartUI, [&]() {
            levelSelectUI->hide();
            if (mainWin) {
                mainWin->disconnect();
                mainWin->close();
                mainWin = nullptr;
            }
            startPage.show();
        });

        // --- LevelSelectUI X关闭 → 回到StartUI ---
        QObject::connect(levelSelectUI.data(), &LevelSelectUI::closeRequested, [&]() {
            levelSelectUI->hide();
            if (mainWin) {
                mainWin->disconnect();
                mainWin->close();
                mainWin = nullptr;
            }
            startPage.show();
        });

        startPage.hide();
        levelSelectUI->show();
    });

    return a.exec();
}