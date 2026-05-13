#include "enemy.h"
#include <QBrush>
#include <QtMath>
#include <QFont>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

Enemy::Enemy(EnemyType type, const QPointF &startPos, int lane,
             const QList<QPointF> &path)
    : QGraphicsEllipseItem(0, 0, 0, 0)
    , enemyType(type)
    , laneX(lane)
    , waypoints(path)
    , waypointIndex(0)
{
    // 如果路点列表为空，添加一个默认终点
    if (waypoints.isEmpty()) {
        waypoints.append(QPointF(laneX, 490));
    }

    // 加载对应类型的PNG图片
    switch (type) {
    case EnemyType::Scout:
        m_pixmap.load(":/enemy/enemies/scout.png");
        break;
    case EnemyType::Soldier:
        m_pixmap.load(":/enemy/enemies/soldier.png");
        break;
    case EnemyType::Tank:
        m_pixmap.load(":/enemy/enemies/tank.png");
        break;
    case EnemyType::Boss:
        m_pixmap.load(":/enemy/enemies/boss.png");
        break;
    }

    int radius;
    switch (type) {
    case EnemyType::Scout:
        hp = 2;
        baseSpeed = 3.5;
        radius = 20;
        break;
    case EnemyType::Soldier:
        hp = 5;
        baseSpeed = 2.0;
        radius = 24;
        break;
    case EnemyType::Tank:
        hp = 15;
        baseSpeed = 1.2;
        radius = 32;
        break;
    case EnemyType::Boss:
        hp = 50;
        baseSpeed = 0.6;
        radius = 40;
        break;
    default:
        hp = 5;
        baseSpeed = 2.0;
        radius = 24;
        break;
    }
    maxHp = hp;
    speed = baseSpeed;
    setRect(-radius, -radius, radius * 2, radius * 2);
    setPos(startPos);
    updateAppearance();
}

QRectF Enemy::boundingRect() const
{
    // 在原始椭圆基础上，上方多留 10px 供血条绘制
    QRectF base = QGraphicsEllipseItem::boundingRect();
    constexpr qreal barExtra = 10.0;
    return base.adjusted(0, -barExtra, 0, 0);
}

void Enemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    Q_UNUSED(widget);

    // 原始椭圆矩形：用于绘制图片、蒙版、边框（不受血条 extra 影响）
    QRectF ellipseRect = QGraphicsEllipseItem::boundingRect();
    QRectF drawRect = ellipseRect.adjusted(2, 2, -2, -2);

    // 扩展后矩形：用于定位血条 Y 坐标
    QRectF rect = boundingRect();

    // 1. 绘制敌人图片（缩放到矩形内）
    if (!m_pixmap.isNull()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        // 使用保存/恢复确保叠加不会影响其他绘制
        painter->save();
        painter->drawPixmap(static_cast<int>(drawRect.x()),
                            static_cast<int>(drawRect.y()),
                            static_cast<int>(drawRect.width()),
                            static_cast<int>(drawRect.height()),
                            m_pixmap);

        // 2. 冰冻状态 → 叠加蓝色半透明蒙版
        if (frozenRemaining > 0) {
            painter->fillRect(drawRect, QColor(100, 150, 255, 140));
        }
        // 3. 减速状态 → 叠加浅蓝色半透明蒙版
        else if (slowRemaining > 0) {
            painter->fillRect(drawRect, QColor(80, 130, 255, 100));
        }

        // 4. 低血量 → 叠加暗色蒙版
        if (!isDead()) {
            qreal ratio = qreal(hp) / maxHp;
            if (ratio < 0.5) {
                int alpha = static_cast<int>((1.0 - ratio) * 2.0 * 120);
                painter->fillRect(drawRect, QColor(0, 0, 0, alpha));
            }
        }

        painter->restore();
    } else {
        // 降级：图片加载失败时绘制纯色圆形
        QColor color;
        if (frozenRemaining > 0) {
            color = QColor(150, 200, 255);
        } else if (slowRemaining > 0) {
            color = QColor(100, 180, 255);
        } else {
            switch (enemyType) {
            case EnemyType::Scout:   color = QColor(255, 180, 50);  break;
            case EnemyType::Soldier: color = QColor(220, 60, 60);   break;
            case EnemyType::Tank:    color = QColor(120, 40, 40);   break;
            case EnemyType::Boss:    color = QColor(200, 20, 20);   break;
            }
        }
        if (!isDead()) {
            qreal ratio = qreal(hp) / maxHp;
            color = color.darker(100 + int((1.0 - ratio) * 150));
        }
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(drawRect);
    }

    // 5. Boss 黄色边框
    if (enemyType == EnemyType::Boss) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::yellow, 2));
        painter->drawEllipse(rect.adjusted(1, 1, -1, -1));
    }

    // 6. 血量条（显示在敌人头顶）
    if (!isDead() && hp < maxHp) {
        qreal barWidth = ellipseRect.width();
        qreal barHeight = 5;
        qreal barX = ellipseRect.x();
        qreal barY = ellipseRect.y() - barHeight - 3;
        qreal ratio = qreal(hp) / maxHp;

        // 背景
        painter->fillRect(QRectF(barX, barY, barWidth, barHeight), QColor(60, 60, 60));
        // 血量
        QColor barColor = ratio > 0.5 ? Qt::green : (ratio > 0.25 ? Qt::yellow : Qt::red);
        painter->fillRect(QRectF(barX, barY, barWidth * ratio, barHeight), barColor);
    }
}

void Enemy::moveTowards(const QPointF &target)
{
    // 被冰冻时不能移动
    if (frozenRemaining > 0)
        return;

    QPointF dir = target - pos();
    qreal dist = dir.manhattanLength();
    if (dist < speed) {
        setPos(target);
    } else {
        dir /= dist;
        setPos(pos() + dir * speed);
    }
}

void Enemy::takeDamage(int damage)
{
    hp -= damage;
    if (hp <= 0) {
        hp = 0;
    }
    updateAppearance();
}

void Enemy::applySlow(int durationMs)
{
    slowRemaining = durationMs;
    speed = baseSpeed * 0.4;
}

void Enemy::applyFreeze(int durationMs)
{
    frozenRemaining = durationMs;
    speed = 0;
    updateAppearance();
}

void Enemy::updateStatusEffects(int deltaMs)
{
    // 更新冰冻
    if (frozenRemaining > 0) {
        frozenRemaining -= deltaMs;
        if (frozenRemaining <= 0) {
            frozenRemaining = 0;
        }
    }

    // 更新减速（冰冻期间减速持续但不影响）
    if (slowRemaining > 0) {
        slowRemaining -= deltaMs;
        if (slowRemaining <= 0) {
            slowRemaining = 0;
        }
    }

    // 重新计算速度
    if (frozenRemaining > 0) {
        speed = 0;
    } else if (slowRemaining > 0) {
        speed = baseSpeed * 0.4;
    } else {
        speed = baseSpeed;
    }

    updateAppearance();
}

int Enemy::getReward() const
{
    switch (enemyType) {
    case EnemyType::Scout:   return 5;
    case EnemyType::Soldier: return 10;
    case EnemyType::Tank:    return 25;
    case EnemyType::Boss:    return 100;
    }
    return 0;
}

void Enemy::updateAppearance()
{
    update();  // 触发重绘，paint() 中处理所有视觉效果
}

// ======================== 路点系统 ========================

QPointF Enemy::getCurrentWaypoint() const
{
    if (waypointIndex < waypoints.size()) {
        return waypoints[waypointIndex];
    }
    // 到达尽头，返回最后一个路点
    return waypoints.isEmpty() ? QPointF(laneX, 490) : waypoints.last();
}

void Enemy::advanceToNextWaypoint()
{
    if (waypointIndex + 1 < waypoints.size()) {
        waypointIndex++;
    } else {
        waypointIndex = waypoints.size(); // 标记已到达终点
    }
}

bool Enemy::hasReachedEnd() const
{
    return waypointIndex >= waypoints.size();
}