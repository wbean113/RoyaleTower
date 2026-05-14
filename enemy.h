#pragma once

#include <QGraphicsEllipseItem>
#include <QPen>
#include <QPixmap>

enum class EnemyType {
    Scout,   // 快速/低血量
    Soldier, // 标准
    Tank,    // 慢速/高血量
    Boss,    // 极慢/极高血量
    king1,
    king2
};

class Enemy : public QGraphicsEllipseItem {
public:
    explicit Enemy(EnemyType type, const QPointF &startPos, int laneX,
                   const QList<QPointF> &path);

    void moveTowards(const QPointF &target);
    void takeDamage(int damage);
    bool isDead() const { return hp <= 0; }
    bool isSlowed() const { return slowRemaining > 0; }
    bool isFrozen() const { return frozenRemaining > 0; }
    bool isKing() const { return enemyType == EnemyType::king1 || enemyType == EnemyType::king2; }
    bool isSpellImmune() const { return enemyType == EnemyType::king1; }
    void applySlow(int durationMs);
    void applyFreeze(int durationMs);
    void updateStatusEffects(int deltaMs);
    void regenerate(int deltaMs);

    EnemyType getEnemyType() const { return enemyType; }
    int getLaneX() const { return laneX; }
    int getReward() const;

    // 路点系统
    QPointF getCurrentWaypoint() const;
    void advanceToNextWaypoint();
    bool hasReachedEnd() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    QRectF boundingRect() const override;

private:
    EnemyType enemyType;
    int hp;
    int maxHp;
    qreal baseSpeed;
    qreal speed;
    int laneX;

    QList<QPointF> waypoints;
    int waypointIndex = 0;

    int slowRemaining = 0;
    int frozenRemaining = 0;
    int m_damageFlashRemaining = 0;
    int m_regenAccumulator = 0;
    QColor m_hpBarColorOverride;

    QPixmap m_pixmap;

    void updateAppearance();
};