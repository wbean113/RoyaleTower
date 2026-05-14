#pragma once

#include <QMainWindow>
#include <QCloseEvent>

enum class Difficulty;

class GameScene;
class GameView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void startGame(int level);
    void setDifficulty(Difficulty diff);

signals:
    void requestReturnToStart();       // 通知main.cpp显示StartUI
    void requestShowLevelSelect();     // 游戏内返回菜单 → 显示LevelSelectUI

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onRequestBackToMenu();

private:
    void cleanupGame();
    void showGame(int level);

    Difficulty m_difficulty;
    GameView *m_gameView = nullptr;
    GameScene *m_gameScene = nullptr;
};