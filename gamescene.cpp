#include "gamescene.h"
#include "carditem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
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

// 关卡1：单条水平直线 (row 4, 从左到右)
static const QList<QPointF> LEVEL1_PATH = {
    GameScene::gridToScene(0, 4),
    GameScene::gridToScene(20, 4),
};

// 关卡2：回字型循环路线
static const QList<QPointF> LEVEL2_PATH_LEFT = {
    GameScene::gridToScene(9, 0),
    GameScene::gridToScene(9, 1),
    GameScene::gridToScene(9, 2),
    GameScene::gridToScene(9, 3),
    GameScene::gridToScene(9, 4),
    GameScene::gridToScene(9, 5),
    GameScene::gridToScene(9, 6),
    GameScene::gridToScene(9, 7),
    GameScene::gridToScene(8, 7),
    GameScene::gridToScene(7, 7),
    GameScene::gridToScene(6, 7),
    GameScene::gridToScene(5, 7),
    GameScene::gridToScene(4, 7),
    GameScene::gridToScene(3, 7),
    GameScene::gridToScene(2, 7),
    GameScene::gridToScene(1, 7),
    GameScene::gridToScene(1, 6),
    GameScene::gridToScene(1, 5),
    GameScene::gridToScene(1, 4),
    GameScene::gridToScene(1, 3),
    GameScene::gridToScene(1, 2),
    GameScene::gridToScene(1, 1),
    GameScene::gridToScene(2, 1),
    GameScene::gridToScene(3, 1),
    GameScene::gridToScene(4, 1),
    GameScene::gridToScene(5, 1),
    GameScene::gridToScene(6, 1),
    GameScene::gridToScene(7, 1),
    GameScene::gridToScene(8, 1),
    GameScene::gridToScene(9, 1),
    GameScene::gridToScene(10, 1),
    GameScene::gridToScene(11, 1),
    GameScene::gridToScene(12, 1),
    GameScene::gridToScene(13, 1),
    GameScene::gridToScene(14, 1),
    GameScene::gridToScene(15, 1),
    GameScene::gridToScene(16, 1),
    GameScene::gridToScene(17, 1),
    GameScene::gridToScene(18, 1),
    GameScene::gridToScene(18, 2),
    GameScene::gridToScene(18, 3),
    GameScene::gridToScene(18, 4),
    GameScene::gridToScene(18, 5),
    GameScene::gridToScene(18, 6),
    GameScene::gridToScene(18, 7),
    GameScene::gridToScene(17, 7),
    GameScene::gridToScene(16, 7),
    GameScene::gridToScene(15, 7),
    GameScene::gridToScene(14, 7),
    GameScene::gridToScene(13, 7),
    GameScene::gridToScene(12, 7),
    GameScene::gridToScene(11, 7),
    GameScene::gridToScene(10, 7),
    GameScene::gridToScene(10, 8)
};
static const QList<QPointF> LEVEL2_PATH_RIGHT = {
    GameScene::gridToScene(10, 0),
    GameScene::gridToScene(10, 1),
    GameScene::gridToScene(10, 2),
    GameScene::gridToScene(10, 3),
    GameScene::gridToScene(10, 4),
    GameScene::gridToScene(10, 5),
    GameScene::gridToScene(10, 6),
    GameScene::gridToScene(10, 7),
    GameScene::gridToScene(11, 7),
    GameScene::gridToScene(12, 7),
    GameScene::gridToScene(13, 7),
    GameScene::gridToScene(14, 7),
    GameScene::gridToScene(15, 7),
    GameScene::gridToScene(16, 7),
    GameScene::gridToScene(17, 7),
    GameScene::gridToScene(18, 7),
    GameScene::gridToScene(18, 6),
    GameScene::gridToScene(18, 5),
    GameScene::gridToScene(18, 4),
    GameScene::gridToScene(18, 3),
    GameScene::gridToScene(18, 2),
    GameScene::gridToScene(18, 1),
    GameScene::gridToScene(17, 1),
    GameScene::gridToScene(16, 1),
    GameScene::gridToScene(15, 1),
    GameScene::gridToScene(14, 1),
    GameScene::gridToScene(13, 1),
    GameScene::gridToScene(12, 1),
    GameScene::gridToScene(11, 1),
    GameScene::gridToScene(10, 1),
    GameScene::gridToScene(9, 1),
    GameScene::gridToScene(8, 1),
    GameScene::gridToScene(7, 1),
    GameScene::gridToScene(6, 1),
    GameScene::gridToScene(5, 1),
    GameScene::gridToScene(4, 1),
    GameScene::gridToScene(3, 1),
    GameScene::gridToScene(2, 1),
    GameScene::gridToScene(1, 1),
    GameScene::gridToScene(1, 2),
    GameScene::gridToScene(1, 3),
    GameScene::gridToScene(1, 4),
    GameScene::gridToScene(1, 5),
    GameScene::gridToScene(1, 6),
    GameScene::gridToScene(1, 7),
    GameScene::gridToScene(2, 7),
    GameScene::gridToScene(3, 7),
    GameScene::gridToScene(4, 7),
    GameScene::gridToScene(5, 7),
    GameScene::gridToScene(6, 7),
    GameScene::gridToScene(7, 7),
    GameScene::gridToScene(8, 7),
    GameScene::gridToScene(9, 7),
    GameScene::gridToScene(9, 8)
};


// 关卡3：Y字型两路汇合（左col3+右col17下行→row5汇合→col10直下）
static const QList<QPointF> LEVEL3_PATH_LEFT = {
    GameScene::gridToScene(3, -1),
    GameScene::gridToScene(3, 0),
    GameScene::gridToScene(3, 1),
    GameScene::gridToScene(3, 2),
    GameScene::gridToScene(4, 2),
    GameScene::gridToScene(5, 2),
    GameScene::gridToScene(6, 2),
    GameScene::gridToScene(7, 2),
    GameScene::gridToScene(7, 3),
    GameScene::gridToScene(7, 4),
    GameScene::gridToScene(7, 5),
    GameScene::gridToScene(8, 5),
    GameScene::gridToScene(9, 5),
    GameScene::gridToScene(10, 5),
    GameScene::gridToScene(10, 6),
    GameScene::gridToScene(10, 7),
    GameScene::gridToScene(10, 8),
    GameScene::gridToScene(10, 9),
};
static const QList<QPointF> LEVEL3_PATH_RIGHT = {
    GameScene::gridToScene(17, -1),
    GameScene::gridToScene(17, 0),
    GameScene::gridToScene(17, 1),
    GameScene::gridToScene(17, 2),
    GameScene::gridToScene(16, 2),
    GameScene::gridToScene(15, 2),
    GameScene::gridToScene(14, 2),
    GameScene::gridToScene(13, 2),
    GameScene::gridToScene(13, 3),
    GameScene::gridToScene(13, 4),
    GameScene::gridToScene(13, 5),
    GameScene::gridToScene(12, 5),
    GameScene::gridToScene(11, 5),
    GameScene::gridToScene(10, 5),
    GameScene::gridToScene(10, 6),
    GameScene::gridToScene(10, 7),
    GameScene::gridToScene(10, 8),
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

    // 加载瓦片图片
    loadTileImages();

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
    setupLevel(1);

    // 定时器 — 难度影响出兵和回蓝节奏
    int resourceInterval = (m_difficulty == Difficulty::Easy) ? 1500 : 1800;
    int spawnInterval = (m_difficulty == Difficulty::Easy) ? 3000 : 2000;

    resourceTimer = new QTimer(this);
    connect(resourceTimer, &QTimer::timeout, this, &GameScene::addResource);
    resourceTimer->start(resourceInterval);

    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &GameScene::spawnEnemy);
    spawnTimer->start(spawnInterval);

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

void GameScene::setDifficulty(Difficulty diff)
{
    m_difficulty = diff;
}

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

    // 从模板拷贝并随机化装饰瓦片（每次setupLevel都会重新随机）
    randomizeMapDecorations(level);

    switch (level) {
    case 1: {
        spawnXList = {
            static_cast<int>(gridToScene(0, 4).x())
        };
        waveEnemiesCount = 10;
        startResource = 100;

        pathsForLevel.append(LEVEL1_PATH);
        break;
    }
    case 2: {
        spawnXList = {
            static_cast<int>(gridToScene(9, 0).x()),
            static_cast<int>(gridToScene(10, 0).x())
        };
        waveEnemiesCount = 12;
        startResource = 120;

        pathsForLevel.append(LEVEL2_PATH_LEFT);
        pathsForLevel.append(LEVEL2_PATH_RIGHT);
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

    // 难度系数缩放
    if (m_difficulty == Difficulty::Easy) {
        waveEnemiesCount = qMax(5, static_cast<int>(waveEnemiesCount * 0.7));
        startResource = static_cast<int>(startResource * 1.5);
    }

    // 重置游戏状态
    resource = startResource;
    enemiesReached = 0;
    wave = 0;
    enemiesSpawned = 0;
    enemiesAlive = 0;
    totalKills = 0;
    isPaused = false;
    if (resourceText)        resourceText->setVisible(true);
    if (enemiesReachedText)  enemiesReachedText->setVisible(true);
    if (waveText)            waveText->setVisible(true);

    if (levelText)           levelText->setVisible(true);

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
    int amount = (m_difficulty == Difficulty::Easy) ? 15 : 12;
    resource += amount;
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
            // 触发对应卡牌的红色脉冲边框特效
            for (CardItem *card : cardPanel) {
                if (card->getCardType() == selectedCardType) {
                    card->flashError();
                    break;
                }
            }
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
            if (!isValidGridCell(snappedPos) || !canPlaceAnything(snappedPos)) {
                hasCardSelected = false;
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
    } else {
        hasCardSelected = false;
    }

    QGraphicsScene::mousePressEvent(event);
}

void GameScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QPointF pos = event->scenePos();
    if (pos.y() < GAME_AREA_H) {
        QGraphicsScene::wheelEvent(event);
        return;
    }

    const int cardWidth = 120;
    const int cardSpacing = 24;
    const int totalCards = 7;
    const int totalCardWidth = totalCards * (cardWidth + cardSpacing) - cardSpacing;

    int delta = event->delta();
    cardScrollOffset += delta * 0.5;

    const qreal minOffset = -(totalCardWidth + baseCardStartX - SCENE_W + 30);
    if (cardScrollOffset > 0)
        cardScrollOffset = 0;
    if (cardScrollOffset < minOffset)
        cardScrollOffset = minOffset;

    for (int i = 0; i < cardPanel.size(); ++i) {
        qreal x = baseCardStartX + i * (cardWidth + cardSpacing) + cardScrollOffset;
        qreal y = cardPanel[i]->pos().y();
        cardPanel[i]->setPos(x, y);
    }

    event->accept();
}


void GameScene::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Space: {
        isPaused = !isPaused;
        update();
        break;
    }

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

}

void GameScene::addTower(TowerType type, const QPointF &pos)
{
    int cost = CardItem::getCostFor(selectedCardType);
    Tower *tower = new Tower(type, pos, cost);
    towers.append(tower);
    connect(tower, &Tower::sellRequested, this, &GameScene::onSellTower);
    addItem(tower);
}

void GameScene::onSellTower(Tower *tower)
{
    if (!tower)
        return;

    // 返还50%圣水
    int refund = tower->getCost() / 2;
    resource += refund;

    // 更新HUD
    if (resourceText)
        resourceText->setPlainText(QString("圣水: %1").arg(resource));

    // 安全清理攻击轨迹
    tower->killAllTrailTimers();
    tower->cleanupTrails();

    // 从场景和列表中移除
    towers.removeAll(tower);
    removeItem(tower);
    delete tower;
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

    int tile = mapLayout[row][col];
    // 只有1-8是路径瓦片，0（草地）和9-13（装饰变体）可放置建筑
    return tile >= 1 && tile <= 8;
}

bool GameScene::canPlaceAnything(const QPointF &pos) const
{
    // 不能在UI区域上放置
    if (pos.y() >= GAME_AREA_H)
        return false;

    // 计算pos所在的网格坐标
    int col = static_cast<int>(pos.x()) / GRID_SIZE;
    int row = static_cast<int>(pos.y()) / GRID_SIZE;

    // 1. 塔位本身不能是路径
    if (isPathTile(col, row))
        return false;

    // 2. 8邻域中至少有一格是路径（塔必须紧邻路径才能放置）
    bool adjacentToPath = false;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0)
                continue;
            if (isPathTile(col + dc, row + dr)) {
                adjacentToPath = true;
                break;
            }
        }
        if (adjacentToPath)
            break;
    }
    if (!adjacentToPath)
        return false;

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
//fireball  spell effect
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
//freeze  spell effect
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
//lightenning  spell effect
void GameScene::applyLightning()
{
    const int lightningDamage = 25;
    int hitCount = 0;

    QList<QPair<qreal, Enemy *>> sorted;
    for (Enemy *e : enemies) {
        if (e->isDead())
            continue;
        qreal dist = GAME_AREA_H - e->pos().y();
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

    // 绘制瓦片地图（20×9 = 180个瓦片，每个80×80图片）
    drawTileMap(painter);

    // 场景边框
    painter->setPen(QPen(QColor(255, 255, 255, 80), 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, 0, SCENE_W, SCENE_H);

    // 游戏区域与UI区域分隔线
    painter->setPen(QPen(QColor(255, 255, 255, 60), 1.5));
    painter->drawLine(0, GAME_AREA_H, SCENE_W, GAME_AREA_H);
}

// ── 前景层：在所有 QGraphicsItem 上方绘制暂停覆盖层 ──
void GameScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    if (isPaused) {
        drawPauseOverlay(painter);
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
    // ── 整体面板背景 ──
    QGraphicsRectItem *panelBg = new QGraphicsRectItem(0, GAME_AREA_H, SCENE_W, UI_AREA_H);
    panelBg->setBrush(QColor(25, 25, 35, 235));
    panelBg->setPen(QPen(QColor(60, 60, 80), 1.5));
    panelBg->setZValue(4);
    addItem(panelBg);

    // ── 左 HUD 面板 (x=20~400, 全高) ──
    {
        QGraphicsRectItem *hudPanel = new QGraphicsRectItem(14, GAME_AREA_H + 12, 386, UI_AREA_H - 24);
        hudPanel->setBrush(QColor(40, 40, 55, 180));
        hudPanel->setPen(QPen(QColor(100, 100, 130, 80), 1));
        hudPanel->setZValue(5);
        addItem(hudPanel);

        const qreal hudX = 24;          // 文字起始 X
        const qreal row1Y = GAME_AREA_H + 24;
        const qreal row2Y = GAME_AREA_H + 64;
        const qreal row3Y = GAME_AREA_H + 104;

        // 行1：圣水 + 关卡
        resourceText = new QGraphicsTextItem();
        resourceText->setPlainText(QString("圣水: %1").arg(resource));
        resourceText->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        resourceText->setDefaultTextColor(QColor(255, 215, 0));
        resourceText->setPos(hudX, row1Y);
        resourceText->setZValue(6);
        addItem(resourceText);

        levelText = new QGraphicsTextItem();
        levelText->setPlainText(QString("关卡 %1").arg(currentLevel));
        levelText->setFont(QFont("Microsoft YaHei", 10));
        levelText->setDefaultTextColor(QColor(180, 180, 200));
        levelText->setPos(hudX + 180, row1Y + 4);
        levelText->setZValue(6);
        addItem(levelText);

        // 行2：波次 | 击杀 | 存活
        waveText = new QGraphicsTextItem();
        waveText->setPlainText(QString("波次: %1 | 击杀: %2 | 存活: %3")
                                   .arg(wave).arg(totalKills).arg(enemiesAlive));
        waveText->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        waveText->setDefaultTextColor(QColor(220, 220, 240));
        waveText->setPos(hudX, row2Y);
        waveText->setZValue(6);
        addItem(waveText);

        // 行3：漏掉 + 已选卡牌
        enemiesReachedText = new QGraphicsTextItem();
        enemiesReachedText->setPlainText(QString("损失血量: %1/%2").arg(enemiesReached).arg(MAX_ENEMIES_REACHED));
        enemiesReachedText->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        enemiesReachedText->setDefaultTextColor(QColor(255, 120, 100));
        enemiesReachedText->setPos(hudX, row3Y);
        enemiesReachedText->setZValue(6);
        addItem(enemiesReachedText);

    }

    // ── 右：卡牌行 (起点 x=430, y=730, 7张横向排列，支持滚轮滚动) ──
    const int cardWidth = 120;
    const int cardSpacing = 24;
    const int startX = 520;
    const int cardY = GAME_AREA_H + 90;  // 730 (卡片顶部)

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

// ======================== 色块地图系统（80×80网格，20列×9行） ========================

void GameScene::initMapLayouts()
{
    // ── 关卡1：单条水平直线 (row 4 全路径) ──
    static const int L1[9][20] = {
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    };
    memcpy(m_templateMap1, L1, sizeof(m_templateMap1));

    // ── 关卡2：回字型单路循环 ──
    static const int L2[9][20] = {
        {-1,0,0,0,0,0,0,0,8,1,1,8,0,0,0,0,0,0,0,-1},
        {0,6,2,2,2,2,2,2,2,7,7,2,2,2,2,2,2,2,5,0},
        {0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0},
        {0,1,0,-1,-1,-1,-1,-1,0,1,1,0,-1,-1,-1,-1,-1,-1,1,0},
        {0,1,0,-1,-1,-1,-1,-1,0,1,1,0,-1,-1,-1,-1,-1,-1,1,0},
        {0,1,0,-1,-1,-1,-1,-1,0,1,1,0,-1,-1,-1,-1,-1,-1,1,0},
        {0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0},
        {0,3,2,2,2,2,2,2,2,7,7,2,2,2,2,2,2,2,4,0},
        {-1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,-1},
    };
    memcpy(m_templateMap2, L2, sizeof(m_templateMap2));

    // ── 关卡3：双路汇合 ──
    static const int L3[9][20] = {
        {-1,-1,8,1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,8,1,0,-1},
        {-1,-1,0,1,0,0,0,0,-1,-1,-1,-1,-1,0,0,0,0,1,0,-1},
        {-1,-1,0,3,2,2,2,5,0,-1,-1,-1,0,6,2,2,2,4,0,-1},
        {-1,-1,-1,0,0,0,0,1,0,-1,-1,-1,0,1,0,0,0,0,-1,-1},
        {-1,-1,-1,-1,-1,-1,0,1,0,0,0,0,0,1,0,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,0,3,2,2,7,2,2,4,0,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,0,0,0,1,0,0,0,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,0,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,0,-1,-1,-1,-1,-1,-1,-1,-1},
    };
    memcpy(m_templateMap3, L3, sizeof(m_templateMap3));
}

// ======================== 装饰瓦片随机化 ========================

void GameScene::randomizeMapDecorations(int level)
{
    // 从模板拷贝到当前关卡的可变地图数组
    switch (level) {
    case 1: memcpy(m_map1, m_templateMap1, sizeof(m_map1)); break;
    case 2: memcpy(m_map2, m_templateMap2, sizeof(m_map2)); break;
    case 3: memcpy(m_map3, m_templateMap3, sizeof(m_map3)); break;
    default: return;
    }

    // 选择当前关卡的地图数据
    int (*mapLayout)[20] = nullptr;
    switch (level) {
    case 1: mapLayout = m_map1; break;
    case 2: mapLayout = m_map2; break;
    case 3: mapLayout = m_map3; break;
    default: return;
    }

    // 遍历将-1占位符替换为9~13随机装饰瓦片
    int replacedCount = 0;
    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            if (mapLayout[row][col] == TILE_DECORATION_RANDOM) {
                mapLayout[row][col] = (std::rand() % 5) + 9;  // 9, 10, 11, 12, 13
                ++replacedCount;
            }
        }
    }
    qDebug() << "[randomizeMapDecorations] Level" << level << "- replaced" << replacedCount << "decoration tiles";
}

// ======================== 瓦片图片加载 ========================

void GameScene::loadTileImages()
{
    m_tileGrass.load(":/images/assets/grass/grass.png");
    m_tileVertical.load(":/images/assets/grass/path_vertical.png");
    m_tileHorizontal.load(":/images/assets/grass/path_horizontal.png");
    m_tileTurn1.load(":/images/assets/grass/turn_1.png");
    m_tileTurn2.load(":/images/assets/grass/turn_2.png");
    m_tileTurn3.load(":/images/assets/grass/turn_3.png");
    m_tileTurn4.load(":/images/assets/grass/turn_4.png");
    m_tile7.load(":/images/assets/grass/path_cross.png");
    m_tile8.load(":/images/assets/grass/00.png");
    m_tile9.load(":/images/assets/grass/01.png");
    m_tile10.load(":/images/assets/grass/02.png");
    m_tile11.load(":/images/assets/grass/03.png");
    m_tile12.load(":/images/assets/grass/04.png");
    m_tile13.load(":/images/assets/grass/06.png");
}

// ======================== 瓦片地图绘制（80×80图片瓦片） ========================

void GameScene::drawTileMap(QPainter *painter)
{
    // 选择当前关卡的地图数据
    const int (*mapLayout)[20] = nullptr;
    switch (currentLevel) {
    case 1: mapLayout = m_map1; break;
    case 2: mapLayout = m_map2; break;
    case 3: mapLayout = m_map3; break;
    default: mapLayout = m_map1; break;
    }

    static const QColor gridLineColor(0, 0, 0, 30);    // 网格线半透明

    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            int tile = mapLayout[row][col];
            int x = col * GRID_SIZE;
            int y = row * GRID_SIZE;

            // 根据瓦片类型选择对应图片（80×80，与GRID_SIZE一致，无需缩放）
            switch (tile) {
            case 0: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileGrass);      break;
            case 1: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileVertical);   break;
            case 2: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileHorizontal); break;
            case 3: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileTurn1);      break;
            case 4: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileTurn2);      break;
            case 5: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileTurn3);      break;
            case 6: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileTurn4);      break;
            case 7: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile7);          break;
            case 8: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile8);          break;
            case 9: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile9);          break;
            case 10: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile10);        break;
            case 11: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile11);        break;
            case 12: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile12);        break;
            case 13: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tile13);        break;
            default: painter->drawPixmap(x, y, GRID_SIZE, GRID_SIZE, m_tileGrass);      break;
            }

            // 绘制网格线（所有格子统一细线，覆盖在图片上方）
            painter->setPen(QPen(gridLineColor, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(x, y, GRID_SIZE, GRID_SIZE);
        }
    }
}

// 绘制暂停覆盖层（游戏区域中央 400×350 半透明面板 + "PAUSED" 大字 + 提示）
void GameScene::drawPauseOverlay(QPainter *painter)
{
    // 半透明暗色遮罩覆盖整个游戏区域
    painter->fillRect(0, 0, SCENE_W, GAME_AREA_H, QColor(0, 0, 0, 120));

    // 中央面板 400×350
    const qreal panelW = 400.0;
    const qreal panelH = 350.0;
    const qreal panelX = (SCENE_W - panelW) / 2.0;
    const qreal panelY = (GAME_AREA_H - panelH) / 2.0;
    QRectF panelRect(panelX, panelY, panelW, panelH);

    painter->setPen(QPen(QColor(255, 255, 255, 40), 2));
    painter->setBrush(QColor(20, 20, 40, 210));
    painter->drawRoundedRect(panelRect, 12, 12);

    // "PAUSED" 大字
    painter->setPen(QColor(255, 255, 255, 230));
    QFont titleFont("Arial", 32, QFont::Bold);
    painter->setFont(titleFont);
    QRectF titleRect(panelX, panelY + 30, panelW, 60);
    painter->drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, "PAUSED");

    // 操作提示
    QFont hintFont("Microsoft YaHei", 14);
    painter->setFont(hintFont);
    painter->setPen(QColor(200, 200, 200, 200));
    QRectF hintRect(panelX, panelY + 110, panelW, 200);
    painter->drawText(hintRect, Qt::AlignHCenter | Qt::AlignTop,
        "按 空格键 继续游戏\n\n"
        "按 1/2/3 切换关卡\n"
        "按 Esc 返回主菜单");
}
