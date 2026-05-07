#include "mainwindow.h"
#include "gamescene.h"
#include "gameview.h"
#include "levelselectui.h"

#include <QStackedWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 窗口尺寸：1600x900（游戏区域1600x720 + UI区域1600x180）
    // GameView通过fitInView自动适配场景
    resize(1600, 900);
    setFixedSize(1600, 900);

    // 屏幕居中显示
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        QRect screenGeo = screen->availableGeometry();
        move((screenGeo.width() - width()) / 2,
             (screenGeo.height() - height()) / 2);
    }

    // QStackedWidget 管理两个页面
    m_stack = new QStackedWidget(this);
    //m_stack->setAlignment(Qt::AlignCenter);//不辣身
    setCentralWidget(m_stack);

    // 页面0：关卡选择UI（用户设计的.ui表单）
    // 创建一个空容器，用来包裹 LevelSelectUI，让它不被拉伸
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignCenter);    // 居中
    layout->setContentsMargins(0,0,0,0);     // 无边距

    // 把关卡页面放进容器
    m_levelSelectUI = new LevelSelectUI(this);
    layout->addWidget(m_levelSelectUI);

    // 把容器放进堆叠控件
    m_stack->addWidget(container);

    // 连接信号
    connect(m_levelSelectUI, &LevelSelectUI::levelSelected,
            this, &MainWindow::onLevelSelected);
    connect(m_levelSelectUI, &LevelSelectUI::backToStartUI,
            this, &MainWindow::onBackToStartUI);
    connect(m_levelSelectUI, &LevelSelectUI::closeRequested,
            this, &MainWindow::onLevelSelectCloseRequested);

    // 首次启动：显示关卡选择界面
    m_stack->setCurrentIndex(0);
}

void MainWindow::showLevelSelect()
{
    // 清理旧的游戏场景
    if (m_gameScene) {
        m_gameScene->disconnect();
        m_gameScene = nullptr;
    }
    // 移除旧的GameView
    if (m_gameView) {
        m_stack->removeWidget(m_gameView);
        m_gameView->deleteLater();
        m_gameView = nullptr;
    }

    // 切换到关卡选择页面
    m_stack->setCurrentIndex(0);
}

void MainWindow::showGame(int level)
{
    // 清理旧的游戏场景
    if (m_gameScene) {
        m_gameScene->disconnect();
        m_gameScene = nullptr;
    }
    if (m_gameView) {
        m_stack->removeWidget(m_gameView);
        m_gameView->deleteLater();
        m_gameView = nullptr;
    }

    // 创建游戏场景
    m_gameScene = new GameScene(this);
    connect(m_gameScene, &GameScene::requestBackToMenu,
            this, &MainWindow::onRequestBackToMenu);

    // 加载指定关卡
    m_gameScene->setLevelForMenu(level);

    // 创建游戏视图
    m_gameView = new GameView(m_gameScene, this);
    m_stack->addWidget(m_gameView);  // index 1（或更高）

    // 切换到游戏页面
    m_stack->setCurrentWidget(m_gameView);
    m_gameView->setFocus();
}

// --- 槽函数 ---

void MainWindow::onLevelSelected(int level)
{
    showGame(level);
}

void MainWindow::onRequestBackToMenu()
{
    // 游戏场景退出 → 回到关卡选择页（规则3）
    showLevelSelect();
}

void MainWindow::onBackToStartUI()
{
    // 按钮"返回开始页" → 关闭MainWindow → StartUI重新显示
    emit requestReturnToStart();
    close();
}

void MainWindow::onLevelSelectCloseRequested()
{
    // 关卡选择页点击右上角X → 回到StartUI（规则2）
    emit requestReturnToStart();
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 规则3：如果在游戏中关闭窗口 → 先回到关卡选择页
    // 规则2/1由LevelSelectUI的closeEvent处理
    if (m_gameScene) {
        // 正在游戏中，不直接关闭，回到关卡选择
        showLevelSelect();
        event->ignore();
        return;
    }
    // 在关卡选择页关闭 → 正常关闭（规则2由onLevelSelectCloseRequested处理）
    QMainWindow::closeEvent(event);
}