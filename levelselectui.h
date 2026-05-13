#ifndef LEVELSELECTUI_H
#define LEVELSELECTUI_H

#include <QWidget>
#include <QCloseEvent>

enum class Difficulty;

namespace Ui {
class LevelSelectUI;
}

class LevelSelectUI : public QWidget
{
    Q_OBJECT

public:
    explicit LevelSelectUI(QWidget *parent = nullptr);
    ~LevelSelectUI();

    Difficulty selectedDifficulty() const;

signals:
    void levelSelected(int level);
    void backToStartUI();
    void closeRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 只保留UI自动生成的槽函数，不写任何其他槽
    void on_btnDifficulty_clicked();

private:
    Ui::LevelSelectUI *ui;
    bool m_isEasy = true; // 仅保留这个变量
};

#endif // LEVELSELECTUI_H