#include "carditem.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QCursor>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

CardItem::CardItem(CardType t, QGraphicsItem *parent)
    : QGraphicsRectItem(-60, -80, 120, 160, parent)
    , type(t)
    , cost(getCostFor(t))
    , cooldown(false)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::PointingHandCursor));

    // 加载对应类型的卡牌PNG图片 (120x160)
    switch (type) {
    case CardType::ArrowTower:
        m_pixmap.load(":/cards/cards/achor_card.png");
        break;
    case CardType::CannonTower:
        m_pixmap.load(":/cards/cards/canon_card.png");
        break;
    case CardType::IceTower:
        m_pixmap.load(":/cards/cards/ice_card.png");
        break;
    case CardType::MagicTower:
        m_pixmap.load(":/cards/cards/magic_card.png");
        break;
    case CardType::Fireball:
        m_pixmap.load(":/cards/cards/fireball_card.png");
        break;
    case CardType::Freeze:
        m_pixmap.load(":/cards/cards/freeze_card.png");
        break;
    case CardType::Lightning:
        m_pixmap.load(":/cards/cards/lightening_card.png");
        break;
    }

    // 名称标签 - 调整位置到 120x160 卡片内
    nameLabel = new QGraphicsTextItem(this);
    nameLabel->setPlainText(getNameFor(t));
    nameLabel->setDefaultTextColor(Qt::white);
    nameLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    nameLabel->setPos(-55, -75);

    // 费用标签
    costLabel = new QGraphicsTextItem(this);
    costLabel->setPlainText(QString::number(cost));
    costLabel->setDefaultTextColor(Qt::yellow);
    costLabel->setFont(QFont("Arial", 12, QFont::Bold));
    costLabel->setPos(30, -75);

    // 冷却标签（默认隐藏）
    cooldownLabel = new QGraphicsTextItem(this);
    cooldownLabel->setPlainText("");
    cooldownLabel->setDefaultTextColor(Qt::gray);
    cooldownLabel->setFont(QFont("Arial", 12, QFont::Bold));
    cooldownLabel->setPos(-55, -55);

    updateAppearance();
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF rect = boundingRect(); // (-60, -80, 120, 160)

    // 1. 绘制卡牌PNG图片
    if (!m_pixmap.isNull()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(static_cast<int>(rect.x()),
                            static_cast<int>(rect.y()),
                            static_cast<int>(rect.width()),
                            static_cast<int>(rect.height()),
                            m_pixmap);

        // 2. 冷却中 → 叠加灰色半透明蒙版
        if (cooldown) {
            painter->fillRect(rect, QColor(128, 128, 128, 160));
        }
    } else {
        // 降级：图片加载失败时绘制纯色矩形
        QColor bgColor;
        if (cooldown) {
            bgColor = QColor(80, 80, 80);
        } else if (isSpell()) {
            bgColor = QColor(120, 40, 140);   // 紫色 - 法术
        } else {
            switch (type) {
            case CardType::ArrowTower:  bgColor = QColor(34, 139, 34);  break;
            case CardType::CannonTower: bgColor = QColor(139, 69, 19);  break;
            case CardType::IceTower:    bgColor = QColor(70, 130, 180);  break;
            case CardType::MagicTower:  bgColor = QColor(148, 0, 211);  break;
            default: bgColor = QColor(100, 100, 100); break;
            }
        }
        painter->setBrush(bgColor);
        painter->setPen(QPen(Qt::white, 2));
        painter->drawRect(rect);
    }

    // 3. 冷却中时整体降低不透明度（文字标签也会受影响，因为它们作为子item独立绘制）
    if (cooldown) {
        setOpacity(0.5);
    } else {
        setOpacity(1.0);
    }
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
    update();  // 触发 paint() 重绘
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