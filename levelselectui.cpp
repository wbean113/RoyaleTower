#include "levelselectui.h"
#include "ui_levelselectui.h"
#include "gamescene.h"

LevelSelectUI::LevelSelectUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LevelSelectUI)
{
    ui->setupUi(this);
    setFixedSize(1402, 1222);
    setWindowTitle("Royal Tower");

    // ====================== 难度按钮初始化（UI 里的 btnDifficulty）======================
    ui->btnDifficulty->setText("简单");
    ui->btnDifficulty->setStyleSheet(R"(
        QPushButton {
            color: #2E7D32;
            font-size: 24px;
            font-weight: bold;
            background: transparent;
            border: 2px solid rgba(255,255,255,120);
            border-radius: 10px;
            padding: 4px 8px;
        }
        QPushButton:hover {
            background: rgba(255,255,255,30);
        }
        QPushButton:pressed {
            background: rgba(255,255,255,50);
        }
    )");

    // ====================== 关卡逻辑不变 ======================
    connect(ui->pushButton, &QPushButton::clicked, this, [this]() {
        emit levelSelected(1);
    });
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]() {
        emit levelSelected(2);
    });
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this]() {
        emit levelSelected(3);
    });
    connect(ui->pushButton_4, &QPushButton::clicked, this, [this]() {
        emit backToStartUI();
    });
}

Difficulty LevelSelectUI::selectedDifficulty() const
{
    return m_isEasy ? Difficulty::Easy : Difficulty::Hard;
}

LevelSelectUI::~LevelSelectUI()
{
    delete ui;
}

void LevelSelectUI::closeEvent(QCloseEvent *event)
{
    emit closeRequested();
    event->ignore();
}
void LevelSelectUI::on_btnDifficulty_clicked()
{
    if (m_isEasy) {
        // 切困难
        m_isEasy = false;
        ui->btnDifficulty->setText("困难");
        ui->btnDifficulty->setStyleSheet(R"(
            QPushButton {
                color: #C62828;
                font-size: 24px;
                font-weight: bold;
                background: transparent;
                border: 2px solid rgba(255,255,255,120);
                border-radius: 10px;
                padding: 4px 8px;
            }
            QPushButton:hover {
                background: rgba(255,255,255,30);
            }
            QPushButton:pressed {
                background: rgba(255,255,255,50);
            }
        )");
    } else {
        // 切简单（
        m_isEasy = true;
        ui->btnDifficulty->setText("简单");
        ui->btnDifficulty->setStyleSheet(R"(
            QPushButton {
                color: #2E7D32;
                font-size: 24px;
                font-weight: bold;
                background: transparent;
                border: 2px solid rgba(255,255,255,120);
                border-radius: 10px;
                padding: 4px 8px;
            }
            QPushButton:hover {
                background: rgba(255,255,255,30);
            }
            QPushButton:pressed {
                background: rgba(255,255,255,50);
            }
        )");
    }
}

