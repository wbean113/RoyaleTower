#pragma once

#include <QMainWindow>
#include <QCloseEvent>

class GameScene;
class LevelSelectUI;
class GameView;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

signals:
    void requestReturnToStart();  // 通知main.cpp显示StartUI

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLevelSelected(int level);
    void onRequestBackToMenu();
    void onBackToStartUI();
    void onLevelSelectCloseRequested();

private:
    void showLevelSelect();
    void showGame(int level);

    QStackedWidget *m_stack = nullptr;
    LevelSelectUI *m_levelSelectUI = nullptr;   // 页面0：关卡选择
    GameView *m_gameView = nullptr;              // 页面1：游戏关卡视图
    GameScene *m_gameScene = nullptr;            // 游戏场景（放gameView中）
};