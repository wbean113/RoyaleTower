#pragma once

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QList>
#include <QMap>
#include <QGraphicsSceneWheelEvent>
#include <QKeyEvent>
#include <QPixmap>
#include "enemy.h"
#include "towerr.h"
#include "carditem.h"



enum class Difficulty { Easy, Hard };

class SpellEffectItem;

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();

    void setLevelForMenu(int level);
    void setDifficulty(Difficulty diff);

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

public slots:
    void addResource();
    void spawnEnemy();
    void updateGame();
    void gameover();

    // 卡牌系统
    void onCardClicked(CardType type);
    // 冷却处理
    void updateCardCooldowns();
    // 清除法术特效
    void clearSpellEffects();

signals:
    void requestBackToMenu();

public:
    // === 地图网格常量 ===
    static constexpr int GRID_SIZE = 80;       // 每格80px
    static constexpr int MAP_COLS = 20;        // 20列
    static constexpr int MAP_ROWS = 9;         // 9行
    static constexpr int GAME_AREA_W = 1600;   // 游戏区域宽度
    static constexpr int GAME_AREA_H = 720;    // 游戏区域高度
    static constexpr int UI_AREA_H = 180;      // UI区域高度
    static constexpr int SCENE_W = 1600;       // 场景总宽
    static constexpr int SCENE_H = 900;        // 场景总高
    static constexpr int END_Y = 720;          // 敌人到达终点Y坐标
    static constexpr int TILE_DECORATION_RANDOM = -1;  // 装饰瓦片随机占位符

    // 辅助：将网格坐标(col,row)转换为场景像素中心坐标
    static QPointF gridToScene(int col, int row);

private:
    Difficulty m_difficulty = Difficulty::Hard;

    // === 定时器 ===
    QTimer *resourceTimer;
    QTimer *spawnTimer;
    QTimer *gameLoopTimer;
    QTimer *cooldownTimer;
    QTimer *spellEffectTimer;

    // === 游戏对象 ===
    QList<Enemy *> enemies;
    QList<Tower *> towers;
    QList<SpellEffectItem *> spellEffects;

    // === 卡牌面板 ===
    QList<CardItem *> cardPanel;
    QMap<CardType, int> cardCooldowns;   // 冷却剩余 ms
    QList<CardType> deck;
    qreal baseCardStartX = 0;
    qreal cardScrollOffset = 0;

    // === 放置网格 ===
    QList<QPointF> gridCells;
    void buildGridCells();
    QPointF snapToNearestCell(const QPointF &pos) const;

    // === HUD ===
    QGraphicsTextItem *resourceText = nullptr;
    QGraphicsTextItem *enemiesReachedText = nullptr;
    QGraphicsTextItem *waveText = nullptr;
    QGraphicsTextItem *levelText = nullptr;

    // === 暂停状态 ===
    bool isPaused = false;

    // === 游戏状态 ===
    int resource = 100;
    int enemiesReached = 0;
    int wave = 0;
    int enemiesSpawned = 0;
    int enemiesAlive = 0;
    int totalKills = 0;

    static const int MAX_ENEMIES_REACHED = 15;

    // === King/Boss 系统 ===
    int m_finalWave = 0;           // 当前关卡的最终波次
    bool m_kingSpawned = false;    // 防止重复生成 king
    bool m_kingAlive = false;      // king 是否存活
    bool m_gameEnded = false;      // 游戏已结束（防止重复触发胜负）

    // === 关卡系统 ===
    int currentLevel = 1;
    int waveEnemiesCount = 10;        // 当前关卡每波敌人数量
    int startResource = 100;          // 当前关卡初始圣水
    QList<int> spawnXList;            // 当前关卡所有出生点X坐标
    QList<QList<QPointF>> pathsForLevel;  // 当前关卡所有路线

    void setupLevel(int level);
    void resetGame();
    const QList<QPointF> &getPathForLaneX(int x) const;

    // === 选中状态 ===
    CardType selectedCardType;
    bool hasCardSelected = false;

    // === 内部方法 ===
    void setupBackground();
    void setupCardPanel();
    void setupHUD();
    void addTower(TowerType type, const QPointF &pos);
    void onSellTower(Tower *tower);
    void castSpell(CardType spellType, const QPointF &pos);
    bool canPlaceAnything(const QPointF &pos) const;
    bool isValidGridCell(const QPointF &pos) const;

    // 生成敌人
    EnemyType randomEnemyType(int laneX = -1) const;
    int randomLaneX() const;
    void spawnNextWave();
    void spawnKing();
    void gamewin();

    // 法术效果
    void applyFireball(const QPointF &pos);
    void applyFreeze();
    void applyLightning();

    // 奖励
    void addKillReward(int amount);

    // ── 网格地图系统（80×80，20列×9行） ──
    int m_templateMap1[9][20];  // 关卡1模板（含-1占位符，不可变）
    int m_templateMap2[9][20];  // 关卡2模板
    int m_templateMap3[9][20];  // 关卡3模板
    int m_map1[9][20];   // 关卡1瓦片数据（0=草地, 1-6=路径）
    int m_map2[9][20];   // 关卡2瓦片数据
    int m_map3[9][20];   // 关卡3瓦片数据
    void initMapLayouts();
    void randomizeMapDecorations(int level);
    void loadTileImages();
    void drawTileMap(QPainter *painter);
    void drawPauseOverlay(QPainter *painter);
    bool isPathTile(int col, int row) const;

    // ── 瓦片图片（80×80） ──
    QPixmap m_tileGrass;       // 0: 草地
    QPixmap m_tileVertical;    // 1: 垂直路
    QPixmap m_tileHorizontal;  // 2: 水平路
    QPixmap m_tileTurn1;       // 3: 转弯1
    QPixmap m_tileTurn2;       // 4: 转弯2
    QPixmap m_tileTurn3;       // 5: 转弯3
    QPixmap m_tileTurn4;       // 6: 转弯4

    QPixmap m_tile8;       // 00: 开始点旁边
    QPixmap m_tile9;    // 01:
    QPixmap m_tile10;  // 02:
    QPixmap m_tile11;       // 03:
    QPixmap m_tile12;       // 04:
    QPixmap m_tile13;       // 05:
    QPixmap m_tile7;       // 7: 十字路口
};