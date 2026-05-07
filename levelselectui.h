#ifndef LEVELSELECTUI_H
#define LEVELSELECTUI_H

#include <QWidget>
#include <QCloseEvent>

namespace Ui {
class LevelSelectUI;
}

class LevelSelectUI : public QWidget
{
    Q_OBJECT

public:
    explicit LevelSelectUI(QWidget *parent = nullptr);
    ~LevelSelectUI();

signals:
    void levelSelected(int level);
    void backToStartUI();
    void closeRequested();  // 点击右上角X → 通知MainWindow

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::LevelSelectUI *ui;
};

#endif // LEVELSELECTUI_H
