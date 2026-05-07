#include "towerr.h"
#include "enemy.h"
#include <QBrush>
#include <QPen>
#include <QLineF>
#include <QtMath>
#include <QPainter>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QTimer>

Tower::Tower(TowerType type, const QPointF &pos)
    : QGraphicsRectItem(-22, -22, 44, 44)
    , towerType(type)
{
    setPos(pos);

    switch (type) {
    case TowerType::Arrow:
        range = 120;
        damage = 2;
        attackIntervalMs = 600;
        break;
    case TowerType::Cannon:
        range = 100;
        damage = 10;
        attackIntervalMs = 1500;
        break;
    case TowerType::Ice:
        range = 110;
        damage = 1;
        attackIntervalMs = 800;
        break;
    case TowerType::Magic:
        range = 90;
        damage = 4;
        attackIntervalMs = 1200;
        break;
    }

    attackTimer = new QTimer(this);
    connect(attackTimer, &QTimer::timeout, this, &Tower::onAttackReady);
    attackTimer->start(attackIntervalMs);
}

Tower::~Tower()
{
    // ==== 三重安全防护 — 杜绝定时器回调访问已销毁对象 ====
    // 第1步：取消所有与塔关联的定时器
    if (attackTimer) {
        attackTimer->stop();
        delete attackTimer;
        attackTimer = nullptr;
    }
    killAllTrailTimers();  // 停止+删除所有轨迹专属定时器

    // 第2步：强制遍历所有trail，安全删除（同步，不依赖定时器）
    cleanupTrails();

    // 第3步：清空列表，确保无野指针残留
    m_activeTrails.clear();
    m_trailTimers.clear();
    // 至此，塔的所有资源已安全释放，无任何pending回调可访问this
}

void Tower::cleanupTrails()
{
    // 同步安全清理所有攻击轨迹 — 完整的双重检查 + 置空
    // 设计原则：
    //   1. 轨迹定时器已在调用此方法前被killAllTrailTimers()取消
    //   2. 直接从scene移除 → delete → 置nullptr
    //   3. 不依赖任何异步回调，100%同步安全
    for (int i = 0; i < m_activeTrails.size(); ++i) {
        QGraphicsLineItem *trail = m_activeTrails[i];
        if (trail != nullptr) {
            QGraphicsScene *s = trail->scene();
            if (s != nullptr) {
                s->removeItem(trail);
            }
            delete trail;
            trail = nullptr;
        }
    }
    m_activeTrails.clear();
}

void Tower::killAllTrailTimers()
{
    // 取消并删除所有轨迹专属定时器
    // 必须在cleanupTrails()之前调用，防止定时器回调触发
    for (int i = 0; i < m_trailTimers.size(); ++i) {
        QTimer *timer = m_trailTimers[i];
        if (timer != nullptr) {
            timer->stop();
            delete timer;
            timer = nullptr;
        }
    }
    m_trailTimers.clear();
}

void Tower::attack(QList<Enemy *> &enemies)
{
    if (!canAttack)
        return;

    Enemy *target = findTarget(enemies);
    if (!target)
        return;

    canAttack = false;

    // 获取塔中心位置（场景坐标）
    QPointF towerCenter = pos();
    QPointF targetPos = target->pos();

    // === 攻击轨迹动画 ===
    QColor trailColor;
    int trailDurationMs;
    qreal lineWidth;

    switch (towerType) {
    case TowerType::Arrow:
        trailColor = QColor(34, 200, 34);   // 绿色弹道
        trailDurationMs = 200;
        lineWidth = 2.5;
        break;
    case TowerType::Cannon:
        trailColor = QColor(220, 150, 30);  // 橙色弹道
        trailDurationMs = 200;
        lineWidth = 3.5;
        break;
    case TowerType::Ice:
        trailColor = QColor(100, 180, 255); // 冰蓝色闪光
        trailDurationMs = 100;
        lineWidth = 3.0;
        break;
    case TowerType::Magic:
        trailColor = QColor(180, 80, 255);  // 紫色闪光
        trailDurationMs = 100;
        lineWidth = 3.0;
        break;
    }

    QGraphicsLineItem *trail = new QGraphicsLineItem(
        QLineF(towerCenter, targetPos));
    trail->setPen(QPen(trailColor, lineWidth, Qt::SolidLine, Qt::RoundCap));
    trail->setZValue(50);
    if (!scene()) return;  // 安全检查：塔已被移除则放弃攻击效果
    scene()->addItem(trail);

    m_activeTrails.append(trail);

    // === 使用Tower拥有的QTimer替代QTimer::singleShot ===
    // 核心安全保证：
    //   - 定时器被Tower持有（m_trailTimers），~Tower()先killAllTrailTimers()再cleanupTrails()
    //   - killAllTrailTimers()在~Tower和~GameScene中都是第一步，确保回调不再触发
    //   - 回调同时从m_activeTrails和m_trailTimers移除自己，cleanupTrails只处理残留
    //   - 100%消除双重释放和野指针
    QTimer *trailTimer = new QTimer(this);
    m_trailTimers.append(trailTimer);
    trailTimer->setSingleShot(true);

    // 连接：使用this上下文，确保直接连接（同线程），this在timer存活期间有效
    connect(trailTimer, &QTimer::timeout, this, [this, trail, trailTimer]() {
        // 从活跃列表中移除（防止cleanupTrails二次delete）
        m_activeTrails.removeAll(trail);
        m_trailTimers.removeAll(trailTimer);

        // 安全删除轨迹
        if (trail != nullptr) {
            QGraphicsScene *s = trail->scene();
            if (s != nullptr) {
                s->removeItem(trail);
            }
            delete trail;
        }

        // 定时器自清理
        trailTimer->deleteLater();
    });

    trailTimer->start(trailDurationMs);

    // === 伤害逻辑（不变） ===
    // 魔法塔AOE攻击
    if (towerType == TowerType::Magic) {
        const int aoeRadius = 50;
        for (Enemy *e : enemies) {
            qreal dist = QLineF(pos(), e->pos()).length();
            if (dist <= aoeRadius) {
                e->takeDamage(damage);
            }
        }
    } else {
        // 单体攻击
        target->takeDamage(damage);
    }

    // 冰塔附加减速效果
    if (towerType == TowerType::Ice && target) {
        target->applySlow(1500);  // 减速1.5秒
    }
}

Enemy *Tower::findTarget(QList<Enemy *> &enemies) const
{
    Enemy *bestTarget = nullptr;
    qreal bestDist = range;

    for (Enemy *e : enemies) {
        if (e->isDead())
            continue;
        qreal dist = QLineF(pos(), e->pos()).length();
        if (dist < bestDist) {
            bestDist = dist;
            bestTarget = e;
        }
    }
    return bestTarget;
}

QRectF Tower::boundingRect() const
{
    return QRectF(-22, -22, 44, 44);
}

void Tower::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QColor color;
    QColor borderColor = Qt::black;
    int borderWidth = 2;

    switch (towerType) {
    case TowerType::Arrow:
        color = QColor(34, 139, 34);   // 绿
        break;
    case TowerType::Cannon:
        color = QColor(139, 69, 19);   // 棕
        break;
    case TowerType::Ice:
        color = QColor(70, 130, 180);  // 蓝
        borderColor = QColor(200, 230, 255);
        borderWidth = 1;
        break;
    case TowerType::Magic:
        color = QColor(148, 0, 211);   // 紫
        borderColor = QColor(230, 180, 255);
        borderWidth = 1;
        break;
    }

    painter->setBrush(color);
    painter->setPen(QPen(borderColor, borderWidth));
    painter->drawRect(boundingRect());

    // 绘制塔的符号标记
    painter->setPen(QPen(Qt::white, 1.5));
    QFont font("Arial", 9, QFont::Bold);
    painter->setFont(font);
    QString symbol;
    switch (towerType) {
    case TowerType::Arrow:  symbol = "->"; break;
    case TowerType::Cannon: symbol = "O";  break;
    case TowerType::Ice:    symbol = "*";  break;
    case TowerType::Magic:  symbol = "~";  break;
    }
    painter->drawText(boundingRect(), Qt::AlignCenter, symbol);

    // 绘制射程指示圈（半透明）
    if (!canAttack) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255, 255, 255, 30), 1, Qt::DashLine));
        painter->drawEllipse(QPointF(0, 0), range, range);
    }
}

void Tower::onAttackReady()
{
    canAttack = true;
}