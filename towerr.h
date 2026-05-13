#pragma once

#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QList>
#include <QPainter>
#include <QPixmap>
#include <QMap>
#include <QGraphicsSceneMouseEvent>

enum class TowerType {
    Arrow,   // 快速/低伤害/单体
    Cannon,  // 慢速/高伤害/单体
    Ice,     // 中等速度/低伤害/减速
    Magic    // 中等速度/中伤害/范围AOE
};

class Enemy;

class Tower : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    Tower(TowerType type, const QPointF &pos, int cost);
    ~Tower();

    TowerType getTowerType() const { return towerType; }
    int getRange() const { return range; }
    int getCost() const { return m_cost; }
    void attack(QList<Enemy *> &enemies);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    // 强制清除所有攻击轨迹（关卡切换前调用）
    void cleanupTrails();

    // 取消并删除所有轨迹定时器
    void killAllTrailTimers();

    // 静态图标缓存：每种塔类型只加载一次80×80 PNG
    static QPixmap &towerPixmap(TowerType type);

signals:
    void sellRequested(Tower *tower);

private slots:
    void onAttackReady();

private:
    TowerType towerType;
    int range;
    int damage;
    int attackIntervalMs;
    int m_cost;
    QTimer *attackTimer;
    bool canAttack = true;

    // 追踪所有活跃的攻击轨迹及其专属定时器，用于安全清理
    QList<QGraphicsLineItem *> m_activeTrails;
    QList<QTimer *> m_trailTimers;

    Enemy *findTarget(QList<Enemy *> &enemies) const;
};
