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

Tower::Tower(TowerType type, const QPointF &pos, int cost)
    : QGraphicsRectItem(-40, -40, 80, 80)
    , towerType(type)
    , m_cost(cost)
{
    setPos(pos);

    switch (type) {
    case TowerType::Arrow:
        range = 300;
        damage = 3;
        attackIntervalMs = 1200;
        break;
    case TowerType::Cannon:
        range = 100;
        damage = 12;
        attackIntervalMs = 3000;
        break;
    case TowerType::Ice:
        range = 100;
        damage = 2;
        attackIntervalMs = 1600;
        break;
    case TowerType::Magic:
        range = 90;
        damage =6;
        attackIntervalMs = 2000;
        break;
    }

    // 启用右键 → 出售系统
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

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

    // === 伤害逻辑（带魔法AOE距离衰减） ===
    // 魔法塔AOE攻击
    if (towerType == TowerType::Magic) {
        const int aoeRadius = 250;
        // 衰减系数：边缘最低保留 30% 伤害
        const qreal minDamageRate = 0.3;

        for (Enemy *e : enemies) {
            if (!e) continue;

            qreal dist = QLineF(pos(), e->pos()).length();
            if (dist <= aoeRadius) {
                qreal rate = 1.0 - (dist / aoeRadius) * (1.0 - minDamageRate);
                int finalDmg = qRound(damage * rate);
                e->takeDamage(finalDmg);
            }
        }
    } else {
        // 单体攻击
        if (target) {
            target->takeDamage(damage);
        }
    }

    // 冰塔附加减速效果
    if (towerType == TowerType::Ice && target) {
        target->applySlow(1500);  // 减速1.5秒
    }
}
//索敌机制在这里
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
    return QRectF(-40, -40, 80, 80);
}

void Tower::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 绘制80×80塔图标
    QPixmap &pix = towerPixmap(towerType);
    if (!pix.isNull()) {
        painter->drawPixmap(-40, -40, 80, 80, pix);
    }
}

void Tower::onAttackReady()
{
    canAttack = true;
}

void Tower::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit sellRequested(this);
        event->accept();
        return;
    }
    QGraphicsRectItem::mousePressEvent(event);
}

// 静态图标缓存：每种塔类型只加载一次80×80 PNG
QPixmap &Tower::towerPixmap(TowerType type)
{
    static QMap<TowerType, QPixmap> cache;
    if (!cache.contains(type)) {
        QString path;
        switch (type) {
        case TowerType::Arrow:  path = ":/tower/assets/atscene/achor_atscene.png"; break;
        case TowerType::Cannon: path = ":/tower/assets/atscene/cannon_atscene.png"; break;
        case TowerType::Ice:    path = ":/tower/assets/atscene/ice_atscene.png";    break;
        case TowerType::Magic:  path = ":/tower/assets/atscene/magic_atscene.png";  break;
        }
        QPixmap pix(path);
        cache[type] = pix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return cache[type];
}
