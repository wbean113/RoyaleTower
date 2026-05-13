#ifndef STARTUI_H
#define STARTUI_H

#include <QWidget>
#include <QCloseEvent>

// 前向声明UI类
namespace Ui {
class StartUI;
}

class StartUI : public QWidget
{
    Q_OBJECT // Qt元对象系统，必须保留

public:
    explicit StartUI(QWidget *parent = nullptr);
    ~StartUI();

signals:
    void startGameRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

    // 自动生成的槽函数声明
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    // UI指针，Qt自动生成的关键成员
    Ui::StartUI *ui;
};

#endif // STARTUI_H
