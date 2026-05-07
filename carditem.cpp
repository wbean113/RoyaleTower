#include "carditem.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QCursor>

CardItem::CardItem(CardType t, QGraphicsItem *parent)
    : QGraphicsRectItem(-40, -28, 80, 56, parent)
    , type(t)
    , cost(getCostFor(t))
    , cooldown(false)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::PointingHandCursor));

    // 名称标签
    nameLabel = new QGraphicsTextItem(this);
    nameLabel->setPlainText(getNameFor(t));
    nameLabel->setDefaultTextColor(Qt::white);
    nameLabel->setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
    nameLabel->setPos(-38, -22);

    // 费用标签
    costLabel = new QGraphicsTextItem(this);
    costLabel->setPlainText(QString::number(cost));
    costLabel->setDefaultTextColor(Qt::yellow);
    costLabel->setFont(QFont("Arial", 10, QFont::Bold));
    costLabel->setPos(22, -22);

    // 冷却标签（默认隐藏）
    cooldownLabel = new QGraphicsTextItem(this);
    cooldownLabel->setPlainText("");
    cooldownLabel->setDefaultTextColor(Qt::gray);
    cooldownLabel->setFont(QFont("Arial", 10, QFont::Bold));
    cooldownLabel->setPos(-38, -8);

    updateAppearance();
}

CardType CardItem::getCardType() const { return type; }
int CardItem::getCost() const { return cost; }

QString CardItem::getName() const { return getNameFor(type); }

QString CardItem::getDescription() const
{
    switch (type) {
    case CardType::ArrowTower:  return "快速攻击，低伤害";
    case CardType::CannonTower: return "高伤害，慢攻速";
    case CardType::IceTower:    return "减速敌人";
    case CardType::MagicTower:  return "范围攻击";
    case CardType::Fireball:    return "对范围内敌人造成伤害";
    case CardType::Freeze:      return "冰冻所有敌人2秒";
    case CardType::Lightning:   return "对3个敌人造成高伤害";
    }
    return "";
}

bool CardItem::isSpell() const
{
    return type == CardType::Fireball ||
           type == CardType::Freeze ||
           type == CardType::Lightning;
}

void CardItem::setOnCooldown(bool cd)
{
    cooldown = cd;
    updateAppearance();
}

bool CardItem::isOnCooldown() const { return cooldown; }

void CardItem::updateCooldownDisplay(int remainingMs)
{
    if (remainingMs > 0) {
        cooldownLabel->setPlainText(QString::number(qreal(remainingMs) / 1000.0, 'f', 1) + "s");
    } else {
        cooldownLabel->setPlainText("");
        setOnCooldown(false);
    }
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (!cooldown)
        emit cardClicked(type);
}

void CardItem::updateAppearance()
{
    QColor bgColor;
    if (cooldown) {
        bgColor = QColor(80, 80, 80);
    } else if (isSpell()) {
        bgColor = QColor(120, 40, 140);   // 紫色 - 法术
    } else {
        switch (type) {
        case CardType::ArrowTower:  bgColor = QColor(34, 139, 34);  break;  // 绿
        case CardType::CannonTower: bgColor = QColor(139, 69, 19);  break;  // 棕
        case CardType::IceTower:    bgColor = QColor(70, 130, 180);  break;  // 蓝
        case CardType::MagicTower:  bgColor = QColor(148, 0, 211);  break;  // 紫
        default: bgColor = QColor(100, 100, 100); break;
        }
    }
    setBrush(bgColor);
    setPen(QPen(Qt::white, 1.5));
    setOpacity(cooldown ? 0.5 : 1.0);
}

int CardItem::getCostFor(CardType type)
{
    switch (type) {
    case CardType::ArrowTower:  return 30;
    case CardType::CannonTower: return 50;
    case CardType::IceTower:    return 40;
    case CardType::MagicTower:  return 60;
    case CardType::Fireball:    return 40;
    case CardType::Freeze:      return 35;
    case CardType::Lightning:   return 50;
    }
    return 0;
}

QString CardItem::getNameFor(CardType type)
{
    switch (type) {
    case CardType::ArrowTower:  return "弓箭塔";
    case CardType::CannonTower: return "加农炮";
    case CardType::IceTower:    return "冰塔";
    case CardType::MagicTower:  return "魔法塔";
    case CardType::Fireball:    return "火球术";
    case CardType::Freeze:      return "冰冻";
    case CardType::Lightning:   return "闪电";
    }
    return "";
}