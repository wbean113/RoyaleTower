#include "gamescene.h"
#include "carditem.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QtMath>
#include <QFont>
#include <cstdlib>
#include <cstring>
#include <ctime>

// 法术特效
class SpellEffectItem : public QGraphicsEllipseItem {
public:
    SpellEffectItem(const QPointF &center, qreal radius, const QColor &color, int lifetimeMs)
        : QGraphicsEllipseItem(center.x() - radius, center.y() - radius, radius * 2, radius * 2)
    {
        setBrush(QColor(color.red(), color.green(), color.blue(), 80));
        setPen(QPen(color, 2));
        SpellEffectItem *self = this;
        QTimer::singleShot(lifetimeMs, [self]() {
            if (self->scene())
                self->scene()->removeItem(self);
        });
        setZValue(10);
    }
};

// ==================== 网格坐标辅助 ====================
// 将网格坐标(col, row)转换为场景像素中心 (col*80+40, row*80+40)
QPointF GameScene::gridToScene(int col, int row)
{
    return QPointF(col * GRID_SIZE + GRID_SIZE / 2.0,
                   row * GRID_SIZE + GRID_SIZE / 2.0);
}
// ==================== 关卡路线数据（基于80×80网格，20列×9行） ====================

// 关卡1：三条S型直角路线
static const QList<QPointF> LEVEL1_PATH_LEFT = {
    GameScene::gridToScene(2, -1),
    GameScene::gridToScene(2, 0),
    GameScene::gridToScene(2, 2),
    GameScene::gridToScene(4, 2),
    GameScene::gridToScene(4, 4),
    GameScene::gridToScene(4, 5),
    GameScene::gridToScene(1, 5),
    GameScene::gridToScene(1, 7),
    GameScene::gridToScene(2, 7),
    GameScene::gridToScene(2, 9),
};
static const QList<QPointF> LEVEL1_PATH_MID = {
    GameScene::gridToScene(10, -1),
    GameScene::gridToScene(10, 0),
    GameScene::gridToScene(10, 2),
    GameScene::gridToScene(12, 2),
    GameScene::gridToScene(12, 4),
    GameScene::gridToScene(12, 5),
    GameScene::gridToScene(8, 5),
    GameScene::gridToScene(8, 7),
    GameScene::gridToScene(10, 7),
    GameScene::gridToScene(10, 9),
};
static const QList<QPointF> LEVEL1_PATH_RIGHT = {
    GameScene::gridToScene(17, -1),
    GameScene::gridToScene(17, 0),
    GameScene::gridToScene(17, 2),
    GameScene::gridToScene(15, 2),
    GameScene::gridToScene(15, 4),
    GameScene::gridToScene(15, 5),
    GameScene::gridToScene(19, 5),
    GameScene::gridToScene(19, 7),
    GameScene::gridToScene(17, 7),
    GameScene::gridToScene(17, 9),
};

// 关卡2：回字型循环路线（单路，绕地图一圈）
static const QList<QPointF> LEVEL2_PATH = {
    GameScene::gridToScene(10, -1),
    GameScene::gridToScene(10, 0),
    GameScene::gridToScene(10, 1),
    GameScene::gridToScene(18, 1),
    GameScene::gridToScene(18, 8),
    GameScene::gridToScene(1, 8),
    GameScene::gridToScene(1, 1),
    GameScene::gridToScene(10, 1),
    GameScene::gridToScene(10, 9),
};

// 关卡3：双路汇合路线
static const QList<QPointF> LEVEL3_PATH_LEFT = {
    GameScene::gridToScene(3, -1),
    GameScene::gridToScene(3, 0),
    GameScene::gridToScene(3, 2),
    GameScene::gridToScene(7, 2),
    GameScene::gridToScene(7, 5),
    GameScene::gridToScene(10, 5),
    GameScene::gridToScene(10, 9),
};
static const QList<QPointF> LEVEL3_PATH_RIGHT = {
    GameScene::gridToScene(17, -1),
    GameScene::gridToScene(17, 0),
    GameScene::gridToScene(17, 2),
    GameScene::gridToScene(13, 2),
    GameScene::gridToScene(13, 5),
    GameScene::gridToScene(10, 5),
    GameScene::gridToScene(10, 9),
};

// ==================== 构造函数 ====================

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    setSceneRect(0, 0, SCENE_W, SCENE_H);

    // 初始化地图布局数据
    initMapLayouts();

    // 初始化卡组
    deck = {
        CardType::ArrowTower,
        CardType::CannonTower,
        CardType::IceTower,
        CardType::MagicTower,
        CardType::Fireball,
        CardType::Freeze,
        CardType::Lightning
    };
    for (CardType ct : deck) {
        cardCooldowns[ct] = 0;
    }

    // 创建HUD和卡牌面板 — 必须在setupLevel之前
    setupBackground();
    setupCardPanel();
    setupHUD();
    setupLevel(1);

    // 定时器
    resourceTimer = new QTimer(this);
    connect(resourceTimer, &QTimer::timeout, this, &GameScene::addResource);
    resourceTimer->start(2000);

    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &GameScene::spawnEnemy);
    spawnTimer->start(2500);

    gameLoopTimer = new QTimer(this);
    connect(gameLoopTimer, &QTimer::timeout, this, &GameScene::updateGame);
    gameLoopTimer->start(50);

    cooldownTimer = new QTimer(this);
    connect(cooldownTimer, &QTimer::timeout, this, &GameScene::updateCardCooldowns);
    cooldownTimer->start(200);

    spellEffectTimer = new QTimer(this);
    connect(spellEffectTimer, &QTimer::timeout, this, &GameScene::clearSpellEffects);
    spellEffectTimer->start(1000);

    // 起始波次
    wave = 1;
    enemiesSpawned = 0;
    spawnNextWave();
}

GameScene::~GameScene()
{
    // ==== 关卡退出时的强制安全清理顺序 ====
    if (resourceTimer)   { resourceTimer->stop();   delete resourceTimer;   resourceTimer = nullptr; }
    if (spawnTimer)      { spawnTimer->stop();      delete spawnTimer;      spawnTimer = nullptr; }
    if (gameLoopTimer)   { gameLoopTimer->stop();   delete gameLoopTimer;   gameLoopTimer = nullptr; }
    if (cooldownTimer)   { cooldownTimer->stop();   delete cooldownTimer;   cooldownTimer = nullptr; }
    if (spellEffectTimer){ spellEffectTimer->stop();delete spellEffectTimer; spellEffectTimer = nullptr; }

    // 遍历所有塔，强制清理攻击轨迹
    for (Tower *t : towers) {
        if (t != nullptr) {
            t->killAllTrailTimers();
            t->cleanupTrails();
        }
    }

    qDeleteAll(enemies);     enemies.clear();
    qDeleteAll(towers);      towers.clear();
    qDeleteAll(spellEffects);spellEffects.clear();
    qDeleteAll(cardPanel);   cardPanel.clear();
}

// ======================== 关卡系统 ========================

void GameScene::setLevelForMenu(int level)
{
    setupLevel(level);
}

void GameScene::setupLevel(int level)
{
    currentLevel = level;

    // 清除旧游戏对象
    qDeleteAll(enemies);
    enemies.clear();
    qDeleteAll(towers);
    towers.clear();
    qDeleteAll(spellEffects);
    spellEffects.clear();

    pathsForLevel.clear();
    spawnXList.clear();

    switch (level) {
    case 1: {
        spawnXList = {
            static_cast<int>(gridToScene(2, 0).x()),
            static_cast<int>(gridToScene(10, 0).x()),
            static_cast<int>(gridToScene(17, 0).x())
        };
        waveEnemiesCount = 10;
        startResource = 100;

        pathsForLevel.append(LEVEL1_PATH_LEFT);
        pathsForLevel.append(LEVEL1_PATH_MID);
        pathsForLevel.append(LEVEL1_PATH_RIGHT);
        break;
    }
    case 2: {
        spawnXList = { static_cast<int>(gridToScene(10, 0).x()) };
        waveEnemiesCount = 12;
        startResource = 120;

        pathsForLevel.append(LEVEL2_PATH);
        break;
    }
    case 3: {
        spawnXList = {
            static_cast<int>(gridToScene(3, 0).x()),
            static_cast<int>(gridToScene(17, 0).x())
        };
        waveEnemiesCount = 8;
        startResource = 90;

        pathsForLevel.append(LEVEL3_PATH_LEFT);
        pathsForLevel.append(LEVEL3_PATH_RIGHT);
        break;
    }
    default:
        setupLevel(1);
        return;
    }

    // 重置游戏状态
    resource = startResource;
    enemiesReached = 0;
    wave = 0;
    enemiesSpawned = 0;
    enemiesAlive = 0;
    totalKills = 0;
    isPaused = false;
    if (pausedText) pausedText->setVisible(false);

    buildGridCells();

    // 更新HUD
    if (resourceText)
        resourceText->setPlainText(QString("圣水: %1").arg(resource));
    if (enemiesReachedText)
        enemiesReachedText->setPlainText(QString("漏掉: %1/%2").arg(enemiesReached).arg(MAX_ENEMIES_REACHED));
    if (levelText)
        levelText->setPlainText(QString("关卡 %1").arg(currentLevel));

    wave = 1;
    enemiesSpawned = 0;
    spawnNextWave();

    update();
}

void GameScene::resetGame()
{
    setupLevel(currentLevel);
}

const QList<QPointF> &GameScene::getPathForLaneX(int x) const
{
    for (int i = 0; i < spawnXList.size() && i < pathsForLevel.size(); ++i) {
        if (spawnXList[i] == x)
            return pathsForLevel[i];
    }
    static const QList<QPointF> emptyPath;
    if (pathsForLevel.isEmpty())
        return emptyPath;
    return pathsForLevel.first();
}

// ======================== 游戏循环 ========================

void GameScene::addResource()
{
    if (isPaused) return;
    resource += 10;
    if (resourceText)
        resourceText->setPlainText(QString("圣水: %1").arg(resource));
}

void GameScene::spawnEnemy()
{
    if (isPaused) return;

    if (enemiesSpawned >= waveEnemiesCount) {
        if (enemies.isEmpty()) {
            wave++;
            enemiesSpawned = 0;
            spawnNextWave();
        }
        return;
    }

    int laneX = randomLaneX();
    const QList<QPointF> &path = getPathForLaneX(laneX);
    QPointF start = path.first();

    EnemyType etype = randomEnemyType(laneX);
    Enemy *enemy = new Enemy(etype, start, laneX, path);
    enemies.append(enemy);
    addItem(enemy);
    enemiesSpawned++;
    enemiesAlive++;
}

void GameScene::updateGame()
{
    if (isPaused) return;
    // 1. 更新敌人状态效果 + 移动 + 到达检测
    for (int i = 0; i < enemies.size(); ++i) {
        Enemy *e = enemies[i];

        e->updateStatusEffects(50);

        QPointF target = e->getCurrentWaypoint();
        e->moveTowards(target);

        if (QLineF(e->pos(), target).length() < 6.0) {
            e->advanceToNextWaypoint();
        }

        if (e->hasReachedEnd()) {
            enemiesReached++;
            enemiesAlive--;
            removeItem(e);
            enemies.removeAt(i);
            delete e;
            --i;

            if (enemiesReachedText)
                enemiesReachedText->setPlainText(QString("漏掉: %1/%2").arg(enemiesReached).arg(MAX_ENEMIES_REACHED));

            if (enemiesReached >= MAX_ENEMIES_REACHED) {
                gameover();
                return;
            }
        }
    }

    // 2. 塔攻击
    for (Tower *t : towers) {
        t->attack(enemies);
    }

    // 3. 移除死亡敌人 + 发放击杀奖励
    for (int i = 0; i < enemies.size(); ++i) {
        if (enemies[i]->isDead()) {
            int reward = enemies[i]->getReward();
            addKillReward(reward);
            totalKills++;
            enemiesAlive--;
            removeItem(enemies[i]);
            delete enemies[i];
            enemies.removeAt(i);
            --i;
        }
    }

    // 4. 更新波次和UI
    if (waveText) {
        waveText->setPlainText(QString("波次: %1 | 击杀: %2 | 存活敌人: %3")
                                   .arg(wave)
                                   .arg(totalKills)
                                   .arg(enemiesAlive));
    }
}

void GameScene::gameover()
{
    resourceTimer->stop();
    spawnTimer->stop();
    gameLoopTimer->stop();
    cooldownTimer->stop();
    spellEffectTimer->stop();

    QMessageBox msgBox;
    msgBox.setWindowTitle("游戏结束");
    msgBox.setText(QString("你输了！\n\n关卡: %1\n波次: %2\n击杀: %3\n漏掉: %4\n\n重试？")
                       .arg(currentLevel)
                       .arg(wave)
                       .arg(totalKills)
                       .arg(enemiesReached));
    QPushButton *retryBtn = msgBox.addButton("重试本关", QMessageBox::ActionRole);
    QPushButton *backBtn = msgBox.addButton("返回关卡选择", QMessageBox::ActionRole);
    msgBox.setDefaultButton(retryBtn);

    msgBox.exec();

    if (msgBox.clickedButton() == retryBtn) {
        resourceTimer->start();
        spawnTimer->start();
        gameLoopTimer->start();
        cooldownTimer->start();
        spellEffectTimer->start();
        resetGame();
    } else if (msgBox.clickedButton() == backBtn) {
        emit requestBackToMenu();
    }
}

// ======================== 鼠标事件 / 卡牌放置 / 施法 ========================

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->scenePos();

    if (pos.y() < GAME_AREA_H && hasCardSelected) {
        if (CardItem::getCostFor(selectedCardType) > resource) {
            hasCardSelected = false;
            if (selectedCardText)
                selectedCardText->setPlainText("资源不足！");
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        bool isSpell = (selectedCardType == CardType::Fireball ||
                        selectedCardType == CardType::Freeze ||
                        selectedCardType == CardType::Lightning);

        if (isSpell) {
            castSpell(selectedCardType, pos);
        } else {
            TowerType ttype;
            switch (selectedCardType) {
            case CardType::ArrowTower:  ttype = TowerType::Arrow;  break;
            case CardType::CannonTower: ttype = TowerType::Cannon; break;
            case CardType::IceTower:    ttype = TowerType::Ice;    break;
            case CardType::MagicTower:  ttype = TowerType::Magic;  break;
            default: return;
            }
            QPointF snappedPos = snapToNearestCell(pos);
            if (!isValidGridCell(snappedPos)) {
                hasCardSelected = false;
                if (selectedCardText)
                    selectedCardText->setPlainText("请放在有效格子上！");
                QGraphicsScene::mousePressEvent(event);
                return;
            }
            addTower(ttype, snappedPos);
        }

        int cost = CardItem::getCostFor(selectedCardType);
        resource -= cost;
        cardCooldowns[selectedCardType] = 5000;

        if (resourceText)
            resourceText->setPlainText(QString("圣水: %1").arg(resource));

        hasCardSelected = false;
        if (selectedCardText)
            selectedCardText->setPlainText("");
    } else {
        hasCardSelected = false;
        if (selectedCardText)
            selectedCardText->setPlainText("");
    }

    QGraphicsScene::mousePressEvent(event);
}

void GameScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QPointF pos = event->scenePos();
    if (pos.y() >= GAME_AREA_H && !cardPanel.isEmpty()) {
        const int cardWidth = 120;
        const int cardSpacing = 24;
        const int totalCards = cardPanel.size();
        const qreal lastCardRightEdge = baseCardStartX + (totalCards - 1) * (cardWidth + cardSpacing) + 60;
        const qreal maxScroll = qMax(0.0, lastCardRightEdge - SCENE_W);

        qreal delta = event->delta();
        cardScrollOffset += delta;
        cardScrollOffset = qBound(0.0, cardScrollOffset, maxScroll);

        for (int i = 0; i < totalCards; ++i) {
            qreal x = baseCardStartX + i * (cardWidth + cardSpacing) - cardScrollOffset;
            cardPanel[i]->setPos(x, 810);
        }
    }

    QGraphicsScene::wheelEvent(event);
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Space:
        isPaused = !isPaused;
        if (pausedText) pausedText->setVisible(isPaused);
        if (!isPaused) update();
        break;

    case Qt::Key_1:
        setupLevel(1);
        break;
    case Qt::Key_2:
        setupLevel(2);
        break;
    case Qt::Key_3:
        setupLevel(3);
        break;

    case Qt::Key_Escape:
        emit requestBackToMenu();
        break;

    default:
        break;
    }

    QGraphicsScene::keyPressEvent(event);
}

void GameScene::onCardClicked(CardType type)
{
    if (cardCooldowns.contains(type) && cardCooldowns[type] > 0)
        return;

    if (CardItem::getCostFor(type) > resource)
        return;

    selectedCardType = type;
    hasCardSelected = true;

    if (selectedCardText) {
        selectedCardText->setPlainText(
            QString("已选择: %1 (费用: %2)")
                .arg(CardItem::getNameFor(type))
                .arg(CardItem::getCostFor(type)));
    }
}

void GameScene::addTower(TowerType type, const QPointF &pos)
{
    Tower *tower = new Tower(type, pos);
    towers.append(tower);
    addItem(tower);
}

void GameScene::castSpell(CardType spellType, const QPointF &pos)
{
    switch (spellType) {
    case CardType::Fireball:
        applyFireball(pos);
        break;
    case CardType::Freeze:
        applyFreeze();
        break;
    case CardType::Lightning:
        applyLightning();
        break;
    default:
        break;
    }
}

// ======================== 网格路径检测（基于地图瓦片数据） ========================

bool GameScene::isPathTile(int col, int row) const
{
    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS)
        return false;

    const int (*mapLayout)[20] = nullptr;
    switch (currentLevel) {
    case 1: mapLayout = m_map1; break;
    case 2: mapLayout = m_map2; break;
    case 3: mapLayout = m_map3; break;
    default: return false;
    }

    // 所有非0瓦片都代表路径（1-6）
    return mapLayout[row][col] != 0;
}

bool GameScene::canPlaceAnything(const QPointF &pos) const
{
    // 不能在UI区域上放置
    if (pos.y() >= GAME_AREA_H)
        return false;

    // 计算pos所在的网格坐标
    int col = static_cast<int>(pos.x()) / GRID_SIZE;
    int row = static_cast<int>(pos.y()) / GRID_SIZE;

    // 检查该格及其8邻域是否有路径（不能放在路径上或紧邻路径）
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (isPathTile(col + dc, row + dr))
                return false;
        }
    }

    // 不能与已有塔重叠（60px最小间距，约0.75个格子）
    for (Tower *t : towers) {
        QPointF diff = pos - t->pos();
        if (diff.manhattanLength() < 60)
            return false;
    }

    return true;
}

// ======================== 卡牌冷却 ========================

void GameScene::updateCardCooldowns()
{
    if (isPaused) return;
    for (auto it = cardCooldowns.begin(); it != cardCooldowns.end(); ++it) {
        if (it.value() > 0) {
            it.value() -= 200;
            if (it.value() < 0)
                it.value() = 0;
        }
    }

    for (CardItem *card : cardPanel) {
        CardType ct = card->getCardType();
        if (cardCooldowns.contains(ct)) {
            int remaining = cardCooldowns[ct];
            if (remaining > 0) {
                card->setOnCooldown(true);
                card->updateCooldownDisplay(remaining);
            } else {
                card->setOnCooldown(false);
                card->updateCooldownDisplay(0);
            }
        }
    }
}

// ======================== 法术效果 ========================

void GameScene::applyFireball(const QPointF &pos)
{
    const int radius = 100;
    const int fireballDamage = 15;

    SpellEffectItem *effect = new SpellEffectItem(pos, radius, QColor(255, 100, 0), 300);
    spellEffects.append(effect);
    addItem(effect);

    for (Enemy *e : enemies) {
        QPointF diff = e->pos() - pos;
        if (diff.manhattanLength() < radius) {
            e->takeDamage(fireballDamage);
        }
    }
}

void GameScene::applyFreeze()
{
    const int freezeDuration = 2000;

    // 场景中心 (800, 360)
    SpellEffectItem *effect = new SpellEffectItem(
        QPointF(SCENE_W / 2.0, GAME_AREA_H / 2.0), 600, QColor(100, 150, 255), freezeDuration);
    effect->setOpacity(0.15);
    spellEffects.append(effect);
    addItem(effect);

    for (Enemy *e : enemies) {
        e->applyFreeze(freezeDuration);
    }
}

void GameScene::applyLightning()
{
    const int lightningDamage = 25;
    int hitCount = 0;

    QList<QPair<qreal, Enemy *>> sorted;
    for (Enemy *e : enemies) {
        if (e->isDead())
            continue;
        qreal dist = END_Y - e->pos().y();
        sorted.append(qMakePair(dist, e));
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });

    for (const auto &pair : sorted) {
        if (hitCount >= 3)
            break;
        Enemy *e = pair.second;
        e->takeDamage(lightningDamage);

        SpellEffectItem *effect = new SpellEffectItem(e->pos(), 40, QColor(255, 255, 0), 200);
        spellEffects.append(effect);
        addItem(effect);

        hitCount++;
    }
}

void GameScene::clearSpellEffects()
{
    if (isPaused) return;
    QList<SpellEffectItem *> toDelete;
    for (int i = spellEffects.size() - 1; i >= 0; --i) {
        if (!spellEffects[i]->scene()) {
            toDelete.append(spellEffects[i]);
            spellEffects.removeAt(i);
        }
    }
    qDeleteAll(toDelete);
}

// ======================== 敌人波次系统 ========================

EnemyType GameScene::randomEnemyType(int laneX) const
{
    Q_UNUSED(laneX);
    int r = std::rand() % 100;

    if (currentLevel == 1) {
        if (wave <= 2) {
            return r < 70 ? EnemyType::Scout : EnemyType::Soldier;
        } else if (wave <= 5) {
            if (r < 30) return EnemyType::Scout;
            if (r < 80) return EnemyType::Soldier;
            return EnemyType::Tank;
        } else {
            if (r < 20) return EnemyType::Scout;
            if (r < 50) return EnemyType::Soldier;
            if (r < 90) return EnemyType::Tank;
            return EnemyType::Boss;
        }
    } else if (currentLevel == 2) {
        if (wave <= 2) {
            if (r < 50) return EnemyType::Scout;
            if (r < 90) return EnemyType::Soldier;
            return EnemyType::Tank;
        } else if (wave <= 5) {
            if (r < 20) return EnemyType::Scout;
            if (r < 60) return EnemyType::Soldier;
            if (r < 90) return EnemyType::Tank;
            return EnemyType::Boss;
        } else {
            if (r < 10) return EnemyType::Scout;
            if (r < 35) return EnemyType::Soldier;
            if (r < 75) return EnemyType::Tank;
            return EnemyType::Boss;
        }
    } else {
        if (wave <= 2) {
            if (r < 40) return EnemyType::Scout;
            if (r < 85) return EnemyType::Soldier;
            return EnemyType::Tank;
        } else if (wave <= 5) {
            if (r < 15) return EnemyType::Scout;
            if (r < 50) return EnemyType::Soldier;
            if (r < 85) return EnemyType::Tank;
            return EnemyType::Boss;
        } else {
            if (r < 10) return EnemyType::Scout;
            if (r < 30) return EnemyType::Soldier;
            if (r < 70) return EnemyType::Tank;
            return EnemyType::Boss;
        }
    }
}

int GameScene::randomLaneX() const
{
    if (spawnXList.isEmpty())
        return static_cast<int>(gridToScene(10, 0).x());
    return spawnXList[std::rand() % spawnXList.size()];
}

void GameScene::spawnNextWave()
{
    if (waveText)
        waveText->setPlainText(QString("波次: %1 | 击杀: %2 | 存活敌人: %3")
                                   .arg(wave)
                                   .arg(totalKills)
                                   .arg(enemiesAlive));
}

void GameScene::addKillReward(int amount)
{
    resource += amount;
    if (resourceText)
        resourceText->setPlainText(QString("圣水: %1").arg(resource));
}

// ======================== 背景绘制 ========================

void GameScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // 草地兜底背景（游戏区域 1600×720）
    painter->fillRect(0, 0, SCENE_W, GAME_AREA_H, QColor(34, 139, 34));
    // UI区域深色底
    painter->fillRect(0, GAME_AREA_H, SCENE_W, UI_AREA_H, QColor(30, 30, 30));

    // 绘制色块地图（20×9 = 180个瓦片，每个80×80）
    drawColorBlockMap(painter);

    // 场景边框
    painter->setPen(QPen(QColor(255, 255, 255, 80), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, 0, SCENE_W, SCENE_H);

    // 游戏区域与UI区域分隔线
    painter->setPen(QPen(QColor(255, 255, 255, 60), 1.5));
    painter->drawLine(0, GAME_AREA_H, SCENE_W, GAME_AREA_H);

    // 塔位可视化（80×80网格，淡绿色虚线方格）
    QPen cellPen(QColor(100, 180, 100, 70), 1.5, Qt::DashLine);
    painter->setPen(cellPen);
    painter->setBrush(QColor(100, 180, 100, 15));
    for (const QPointF &cell : gridCells) {
        painter->drawRoundedRect(QRectF(cell.x() - 35, cell.y() - 35, 70, 70), 4, 4);
    }
}

void GameScene::setupBackground()
{
    setBackgroundBrush(Qt::NoBrush);
}

// ======================== 放置网格系统（20×9 = 180格，基于80px间距） ========================

void GameScene::buildGridCells()
{
    gridCells.clear();

    for (int col = 0; col < MAP_COLS; ++col) {
        for (int row = 0; row < MAP_ROWS; ++row) {
            QPointF cell = gridToScene(col, row);
            if (canPlaceAnything(cell)) {
                gridCells.append(cell);
            }
        }
    }
}

QPointF GameScene::snapToNearestCell(const QPointF &pos) const
{
    if (gridCells.isEmpty())
        return pos;

    QPointF nearest = gridCells.first();
    qreal bestDist = QLineF(pos, nearest).length();

    for (const QPointF &cell : gridCells) {
        qreal dist = QLineF(pos, cell).length();
        if (dist < bestDist) {
            bestDist = dist;
            nearest = cell;
        }
    }

    return nearest;
}

bool GameScene::isValidGridCell(const QPointF &pos) const
{
    return gridCells.contains(pos);
}

// ======================== 卡牌面板（UI区域 y=720~900，180px高） ========================

void GameScene::setupCardPanel()
{
    QGraphicsRectItem *panelBg = new QGraphicsRectItem(0, GAME_AREA_H, SCENE_W, UI_AREA_H);
    panelBg->setBrush(QColor(30, 30, 30, 220));
    panelBg->setPen(QPen(QColor(80, 80, 100), 1));
    panelBg->setZValue(5);
    addItem(panelBg);

    const int cardWidth = 120;
    const int cardSpacing = 24;
    const int totalCards = 7;
    const qreal totalWidth = totalCards * (cardWidth + cardSpacing) - cardSpacing;
    const int startX = static_cast<int>((SCENE_W - totalWidth) / 2.0);
    const int cardY = GAME_AREA_H + UI_AREA_H / 2 + 5;

    baseCardStartX = startX;

    CardType types[] = {
        CardType::ArrowTower,
        CardType::CannonTower,
        CardType::IceTower,
        CardType::MagicTower,
        CardType::Fireball,
        CardType::Freeze,
        CardType::Lightning
    };

    for (int i = 0; i < 7; ++i) {
        CardItem *card = new CardItem(types[i]);
        card->setPos(startX + i * (cardWidth + cardSpacing), cardY);
        card->setZValue(6);
        connect(card, &CardItem::cardClicked, this, &GameScene::onCardClicked);
        cardPanel.append(card);
        addItem(card);
    }
}

// ======================== HUD（游戏区域底部 y=690~720） ========================

void GameScene::setupHUD()
{
    QFont hudFont("Microsoft YaHei", 12, QFont::Bold);
    QColor hudColor = Qt::white;

    // HUD 半透明背景条
    QGraphicsRectItem *hudBg = new QGraphicsRectItem(0, GAME_AREA_H - 30, SCENE_W, 30);
    hudBg->setBrush(QColor(20, 20, 30, 200));
    hudBg->setPen(QPen(QColor(80, 80, 100), 1));
    hudBg->setZValue(7);
    addItem(hudBg);

    // 圣水资源
    resourceText = new QGraphicsTextItem();
    resourceText->setPlainText(QString("圣水: %1").arg(resource));
    resourceText->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
    resourceText->setDefaultTextColor(QColor(255, 215, 0));
    resourceText->setPos(20, GAME_AREA_H - 28);
    resourceText->setZValue(7);
    addItem(resourceText);

    // 漏掉计数
    enemiesReachedText = new QGraphicsTextItem();
    enemiesReachedText->setPlainText(QString("漏掉: %1/%2").arg(enemiesReached).arg(MAX_ENEMIES_REACHED));
    enemiesReachedText->setFont(hudFont);
    enemiesReachedText->setDefaultTextColor(hudColor);
    enemiesReachedText->setPos(400, GAME_AREA_H - 28);
    enemiesReachedText->setZValue(7);
    addItem(enemiesReachedText);

    // 波次/击杀/存活
    waveText = new QGraphicsTextItem();
    waveText->setPlainText(QString("波次: %1 | 击杀: %2 | 存活敌人: %3").arg(wave).arg(totalKills).arg(enemiesAlive));
    waveText->setFont(hudFont);
    waveText->setDefaultTextColor(hudColor);
    waveText->setPos(850, GAME_AREA_H - 28);
    waveText->setZValue(7);
    addItem(waveText);

    // 已选择卡牌提示（游戏区域左上角）
    selectedCardText = new QGraphicsTextItem();
    selectedCardText->setPlainText("");
    selectedCardText->setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    selectedCardText->setDefaultTextColor(QColor(255, 255, 100));
    selectedCardText->setPos(20, 8);
    selectedCardText->setZValue(7);
    addItem(selectedCardText);

    // 操作提示（HUD右侧）
    QGraphicsTextItem *hintText = new QGraphicsTextItem();
    hintText->setPlainText("1/2/3切关卡 | Esc返回 | 空格暂停 | 卡牌 → 场地");
    hintText->setFont(QFont("Microsoft YaHei", 9));
    hintText->setDefaultTextColor(QColor(180, 180, 180));
    hintText->setPos(1200, GAME_AREA_H - 28);
    hintText->setZValue(7);
    addItem(hintText);

    // 关卡文字（游戏区域右上角）
    levelText = new QGraphicsTextItem();
    levelText->setPlainText(QString("关卡 %1").arg(currentLevel));
    levelText->setFont(QFont("Arial", 16, QFont::Bold));
    levelText->setDefaultTextColor(QColor(255, 255, 255, 180));
    levelText->setPos(SCENE_W - 160, 8);
    levelText->setZValue(7);
    addItem(levelText);

    // 暂停提示文字（场景中央，默认隐藏）
    pausedText = new QGraphicsTextItem();
    pausedText->setPlainText("PAUSED");
    pausedText->setFont(QFont("Arial", 48, QFont::Bold));
    pausedText->setDefaultTextColor(QColor(255, 255, 255, 128));
    pausedText->setPos(SCENE_W / 2.0 - 150, GAME_AREA_H / 2.0 - 50);
    pausedText->setZValue(20);
    pausedText->setVisible(false);
    addItem(pausedText);
}

// ======================== 色块地图系统（80×80网格，20列×9行） ========================

void GameScene::initMapLayouts()
{
    // ── 关卡1：三条S型直角路线 ──
    static const int L1[9][20] = {
        {0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0},
        {0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0},
        {0,0,3,2,5,0,0,0,0,0,3,2,5,0,0,6,2,4,0,0},
        {0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0},
        {0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0},
        {0,6,2,2,4,0,0,0,6,2,2,2,4,0,0,3,2,2,2,5},
        {0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
        {0,3,2,0,0,0,0,0,3,2,2,0,0,0,0,0,0,0,2,4},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };
    memcpy(m_map1, L1, sizeof(m_map1));

    // ── 关卡2：回字型单路循环 ──
    static const int L2[9][20] = {
        {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
        {0,5,2,2,2,2,2,2,2,2,3,2,2,2,2,2,2,2,5,0},
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
        {0,3,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,4,0},
    };
    memcpy(m_map2, L2, sizeof(m_map2));

    // ── 关卡3：双路汇合 ──
    static const int L3[9][20] = {
        {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,0,0,3,2,2,2,5,0,0,0,0,0,6,2,2,2,4,0,0},
        {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,0,0,0,0,0,0,3,2,2,5,2,2,4,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
    };
    memcpy(m_map3, L3, sizeof(m_map3));
}

void GameScene::drawColorBlockMap(QPainter *painter)
{
    // 选择当前关卡的地图数据
    const int (*mapLayout)[20] = nullptr;
    switch (currentLevel) {
    case 1: mapLayout = m_map1; break;
    case 2: mapLayout = m_map2; break;
    case 3: mapLayout = m_map3; break;
    default: mapLayout = m_map1; break;
    }

    // 色块颜色定义
    static const QColor grassColor(34, 139, 34);      // 草地绿
    static const QColor pathColor(210, 180, 140);      // 土路褐
    static const QColor gridLineColor(0, 0, 0, 30);    // 网格线半透明

    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            int tile = mapLayout[row][col];
            int x = col * GRID_SIZE;
            int y = row * GRID_SIZE;

            if (tile == 0) {
                // 草地
                painter->fillRect(x, y, GRID_SIZE, GRID_SIZE, grassColor);
            } else {
                // 路径（所有非0瓦片类型）
                painter->fillRect(x, y, GRID_SIZE, GRID_SIZE, pathColor);
            }

            // 绘制网格线（所有格子统一细线）
            painter->setPen(QPen(gridLineColor, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(x, y, GRID_SIZE, GRID_SIZE);
        }
    }
}