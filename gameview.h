#pragma once

#include <QGraphicsView>

class GameView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GameView(QGraphicsScene *scene, QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event) override;
};