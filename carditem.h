#include <QCursor>
#pragma once

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>

enum class CardType {
    ArrowTower,   // 弓箭塔
    CannonTower,  // 加农炮塔
    IceTower,     // 冰塔（减速）
    MagicTower,   // 魔法塔（AOE）
    Fireball,     // 火球术
    Freeze,       // 冰冻术
    Lightning     // 闪电术
};

class CardItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    CardItem(CardType type, QGraphicsItem *parent = nullptr);

    CardType getCardType() const;
    int getCost() const;
    QString getName() const;
    QString getDescription() const;
    bool isSpell() const;
    void setOnCooldown(bool cd);
    bool isOnCooldown() const;
    void updateCooldownDisplay(int remainingMs);

    static int getCostFor(CardType type);
    static QString getNameFor(CardType type);

signals:
    void cardClicked(CardType type);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    CardType type;
    int cost;
    bool cooldown;
    QGraphicsTextItem *nameLabel;
    QGraphicsTextItem *costLabel;
    QGraphicsTextItem *cooldownLabel;

    void updateAppearance();
};