#include "enemy.h"
#include <QBrush>
#include <QtMath>
#include <QFont>

Enemy::Enemy(EnemyType type, const QPointF &startPos, int lane,
             const QList<QPointF> &path)
    : QGraphicsEllipseItem(0, 0, 0, 0)
    , enemyType(type)
    , laneX(lane)
    , waypoints(path)
    , waypointIndex(0)
{
    // 设置初始位置为起点
    setPos(startPos);
    // 如果路点列表为空，添加一个默认终点
    if (waypoints.isEmpty()) {
        waypoints.append(QPointF(laneX, 490));
    }
    int radius;
    switch (type) {
    case EnemyType::Scout:
        hp = 2;
        baseSpeed = 3.5;
        radius = 8;
        break;
    case EnemyType::Soldier:
        hp = 5;
        baseSpeed = 2.0;
        radius = 12;
        break;
    case EnemyType::Tank:
        hp = 15;
        baseSpeed = 1.2;
        radius = 18;
        break;
    case EnemyType::Boss:
        hp = 50;
        baseSpeed = 0.6;
        radius = 25;
        break;
    default:
        hp = 5;
        baseSpeed = 2.0;
        radius = 12;
        break;
    }
    maxHp = hp;
    speed = baseSpeed;
    setRect(-radius, -radius, radius * 2, radius * 2);
    setPos(startPos);
    updateAppearance();
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
    QColor color;
    if (frozenRemaining > 0) {
        color = QColor(150, 200, 255);  // 冰冻 - 浅蓝
    } else if (slowRemaining > 0) {
        color = QColor(100, 180, 255);  // 减速 - 蓝色覆盖
    } else {
        switch (enemyType) {
        case EnemyType::Scout:   color = QColor(255, 180, 50);  break;  // 橙色
        case EnemyType::Soldier: color = QColor(220, 60, 60);   break;  // 红色
        case EnemyType::Tank:    color = QColor(120, 40, 40);   break;  // 暗红
        case EnemyType::Boss:    color = QColor(200, 20, 20);   break;  // 深红
        }
    }

    // 血量越低颜色越暗
    if (!isDead()) {
        qreal ratio = qreal(hp) / maxHp;
        color = color.darker(100 + int((1.0 - ratio) * 150));
    }

    setBrush(color);

    // Boss有特殊边框
    if (enemyType == EnemyType::Boss) {
        setPen(QPen(Qt::yellow, 2));
    } else {
        setPen(QPen(Qt::black, 1));
    }
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
