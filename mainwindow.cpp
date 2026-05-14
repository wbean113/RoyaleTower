#include "mainwindow.h"
#include "gamescene.h"
#include "gameview.h"

#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>

#include <bgm.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_difficulty(Difficulty::Hard)
{
    // 窗口尺寸：1600x900（游戏区域1600x720 + UI区域1600x180）
    resize(1600, 900);
    setFixedSize(1600, 900);

    // 屏幕居中显示
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect screenGeo = screen->availableGeometry();
        move((screenGeo.width() - width()) / 2,
             (screenGeo.height() - height()) / 2);
    }

    setWindowTitle("Royal Tower");
}

void MainWindow::setDifficulty(Difficulty diff)
{
    m_difficulty = diff;
}

void MainWindow::startGame(int level)
{
    showGame(level);
}

void MainWindow::showGame(int level)
{
    cleanupGame();

    // 创建游戏场景
    m_gameScene = new GameScene(this);
    connect(m_gameScene, &GameScene::requestBackToMenu,
            this, &MainWindow::onRequestBackToMenu);

    // 应用用户选择的难度
    m_gameScene->setDifficulty(m_difficulty);

    // 加载指定关卡
    m_gameScene->setLevelForMenu(level);

    // 创建游戏视图
    m_gameView = new GameView(m_gameScene, this);
    setCentralWidget(m_gameView);
    m_gameView->setFocus();

    // 切换到战斗音乐
    BGMPlayer::instance().playRandomBGM({
        ":/music/music/music01.mp3",
        ":/music/music/music02.mp3"
    });
}

void MainWindow::cleanupGame()
{
    if (m_gameScene) {
        m_gameScene->disconnect();
        m_gameScene = nullptr;
    }
    if (m_gameView) {
        m_gameView->deleteLater();
        m_gameView = nullptr;
    }
}

void MainWindow::onRequestBackToMenu()
{
    // 游戏内"返回菜单" → 切回菜单BGM，通知main.cpp显示LevelSelectUI
    BGMPlayer::instance().playRandomBGM({
        ":/music/music/music03.mp3",
        ":/music/music/music04.mp3"
    });

    cleanupGame();
    emit requestShowLevelSelect();
    close();  // 关闭游戏窗口
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 规则3：如果在游戏中关闭窗口 → 回到StartUI（不是LevelSelectUI）
    if (m_gameScene) {
        cleanupGame();
        BGMPlayer::instance().playRandomBGM({
            ":/music/music/music03.mp3",
            ":/music/music/music04.mp3"
        });
        emit requestReturnToStart();
        event->accept();
        return;
    }
    QMainWindow::closeEvent(event);
}