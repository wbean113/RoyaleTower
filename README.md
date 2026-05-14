# RoyaleTower — 皇家塔防

基于 Qt 6 的 2D 图形化塔防游戏 | 南开大学高级语言程序设计大作业项目

![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![Qt](https://img.shields.io/badge/Qt-6.11.0-green)
![Language](https://img.shields.io/badge/C%2B%2B-17-blue)
![License](https://img.shields.io/badge/license-MIT-orange)

---

## 游戏截图

<img width="1584" height="713" alt="图片" src="https://github.com/user-attachments/assets/41aae698-f76f-4055-83fe-ceadef5806cd" />


---
## 提交说明

由于中途教给git托管，而本人能力有限，网络环境波动，导致中间大量提交记录遗失。本人已于2026.5.13重建代码仓库，如果需要查看更古早的开发记录，请移步本人的另一仓库 [Tower-Royale](https://github.com/wbean113/Tower-Royale)，其中具备相对更古早一些但是残缺的记录。

## 游戏简介

**RoyaleTower** 是一款经典的塔防策略游戏。玩家在圣水资源有限的条件下，合理布置四种防御塔，释放三种法术，抵御一波波敌人的进攻。敌人沿预设路径前进，到达终点则累积失血值，失血值满则游戏失败。成功消灭所有波次的敌人（包括最终 BOSS）即为胜利。

### 核心特色

- **3 个精心设计的关卡**：直线型、回字型、双路汇合型，难度递增
- **4 种防御塔**：箭塔（高攻速）、炮塔（范围伤害）、冰塔（减速）、魔塔（高伤害）
- **3 种主动法术**：火球术（范围伤害，距离衰减）、冰冻术（群体冻结）、闪电术（单体高伤）
- **6 种敌人**：侦察兵、士兵、坦克、Boss、King Ⅰ、King Ⅱ，各有不同的血量/速度/抗性
- **波次系统**：每关 3+ 波敌人，波次越高敌人越强
- **难度选择**：简单 / 困难，影响敌人强度与资源获取速率
- **动态 BGM**：4 首背景音乐，不支持切换
- **暂停/恢复**：空格键暂停，支持中途切换关卡或返回菜单

---

## 技术栈

| 层级 | 技术 |
|------|------|
| **语言** | C++17 |
| **框架** | Qt 6.11.0 (Widgets + Multimedia) |
| **构建工具** | qmake (Qt Creator 10.0.2) |
| **图形渲染** | QGraphicsScene / QGraphicsView |
| **音频** | QMediaPlayer (Qt Multimedia) |
| **架构模式** | 面向对象 + 信号槽解耦 |

### 类继承架构

```
QGraphicsScene       →  GameScene          (游戏核心场景)
QGraphicsView        →  GameView           (自定义视图)
QMainWindow          →  MainWindow         (游戏窗口)
QGraphicsEllipseItem →  Enemy              (敌人基类)
QGraphicsRectItem    →  Tower              (防御塔)
QGraphicsPixmapItem  →  CardItem           (卡牌UI)
QGraphicsRectItem    →  SpellEffectItem    (法术特效)
```

---

## 编译与运行

### 环境要求

- **Qt 6.x** (推荐 6.5+)
- **C++17 兼容编译器** (GCC 9+ / MSVC 2019+ / Clang 10+)
- **Qt Multimedia** 模块（用于背景音乐播放）

### 从源码构建

```bash
# 1. 克隆仓库
git clone https://github.com/your-username/RoyaleTower.git
cd RoyaleTower

# 2. 使用 qmake 生成 Makefile
qmake RoyaleTower.pro

# 3. 编译 (Linux/macOS)
make

# 或 (Windows MinGW)
mingw32-make

# 4. 运行
./RoyaleTower    # Linux/macOS
RoyaleTower.exe  # Windows
```

也可直接用 **Qt Creator** 打开 `RoyaleTower.pro`，点击运行即可。

---

## 操作说明

| 操作 | 方式 |
|------|------|
| **放置防御塔** | 点击卡牌 → 点击草地格 |
| **释放法术** | 点击法术卡牌 → 点击目标位置 |
| **暂停 / 恢复** | 按 `Space` 空格键 |
| **切换关卡** | 暂停时按 `1` / `2` / `3` |
| **返回主菜单** | 暂停时按 `Esc` |

---

## 项目结构

```
RoyaleTower/
├── main.cpp                 # 入口，管理StartUI/LevelSelectUI/MainWindow生命周期
├── mainwindow.cpp/h         # 游戏主窗口
├── gamescene.cpp/h          # 核心场景：地图、敌人、防御塔、法术、波次
├── gameview.cpp/h           # 自定义QGraphicsView（鼠标事件转发）
├── enemy.cpp/h              # 敌人类（路径跟随、状态效果、血量）
├── towerr.cpp/h             # 防御塔类（索敌、攻击、弹道）
├── carditem.cpp/h           # 卡牌UI（冷却、点击激活）
├── spriterenderer.cpp/h     # 精灵渲染器
├── bgm.h                    # 背景音乐封装（QMediaPlayer）
├── startui.cpp/h/ui         # 开始界面
├── levelselectui.cpp/h/ui   # 关卡选择界面
├── resources.qrc            # Qt资源文件
├── RoyaleTower.pro          # qmake项目文件
├── assets/
│   ├── grass/               # 20×9 草地瓦片图片（80×80px）
│   └── atscene/             # 场景装饰资源
├── cards/                   # 7张卡牌图标
├── enemies/                 # 6种敌人精灵图
├── hit effects of spells/   # 3种法术特效图
├── images/                  # 背景图片
└── music/                   # 4首BGM (MP3)
```

---

## 核心算法

### 网格坐标转换
```
sceneX = col × GRID_SIZE + GRID_SIZE / 2
sceneY = row × GRID_SIZE + GRID_SIZE / 2
```
`GRID_SIZE = 80`（每个瓦片像素尺寸）。

### 防御塔索敌
遍历所有存活敌人，计算欧式距离，取射程内距离最近的作为目标。

### 敌人路径跟随
沿瓦片路径逐格移动，每帧更新位置。到达路径点切换阈值<1像素。



---

## 开发中的 Bug 修复

| Bug | 描述 | 修复方案 |
|-----|------|----------|
| **SpellEffectItem 竞态条件** | `QTimer::singleShot` 异步回调与 `clearSpellEffects()` 同步遍历冲突，导致随机崩溃 | 移除异步回调，改为游戏主循环同步管理生命周期，清理间隔从1000ms优化至100ms |
| **initMapLayouts 未定义符号** | 地图数组中 `x` 占位符未定义导致编译失败 | 引入 `TILE_DECORATION_RANDOM = -1` 语义常量替换，`drawTileMap()` 添加 `default` 防御分支 |



---

## 许可证

本项目采用 [MIT License](LICENSE)。

---

## 作者

**南开大学 计算机大类 BBDZ**

> 本项目为高级语言程序设计课程大作业，旨在实践面向对象程序设计、Qt图形化开发、游戏逻辑架构与单元测试。
