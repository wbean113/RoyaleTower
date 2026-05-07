#pragma once

#include <QPixmap>
#include <QPainter>
#include <QSize>
#include <QMap>
#include <QString>

/**
 * @brief 统一精灵渲染器
 *
 * 职责：
 * - 从 Qt 资源系统加载 PNG 透明图片
 * - 缓存原始 QPixmap（全局共享）和缩放后的 QPixmap（每实例）
 * - 使用中心点坐标绘制（drawX = cx - w/2, drawY = cy - h/2）
 * - 保持宽高比 + SmoothTransformation 平滑缩放
 *
 * 用法：
 * @code
 *   SpriteRenderer sprite;
 *   sprite.load(":/enemies/scout.png");
 *   sprite.setRenderSize(48, 48);
 *   // 在 paint() 中:
 *   sprite.draw(painter, QPointF(0, 0));  // 以物品本地原点为中心
 * @endcode
 */
class SpriteRenderer {
public:
    SpriteRenderer();
    ~SpriteRenderer();

    // ---- 加载 ----
    /// 从 Qt 资源路径加载原始 pixmap（全局缓存，多次加载同一路径只读取一次）
    bool load(const QString &resourcePath);
    bool isLoaded() const { return !m_originalPixmap.isNull(); }

    // ---- 渲染尺寸 ----
    /// 设置目标渲染尺寸（维持宽高比）
    void setRenderSize(const QSize &size);
    void setRenderSize(int width, int height);
    QSize renderSize() const { return m_renderSize; }

    // ---- 绘制 ----
    /// 以 center 为中心绘制缩放后的精灵（center 是此物品本地坐标系的点）
    void draw(QPainter *painter, const QPointF &center) const;
    void draw(QPainter *painter, qreal x, qreal y) const;

    // ---- 访问器 ----
    const QPixmap &originalPixmap() const { return m_originalPixmap; }
    const QPixmap &scaledPixmap() const { return m_scaledPixmap; }

    /// 静态辅助：直接从资源路径获取全局缓存的 pixmap
    static QPixmap cachedPixmap(const QString &resourcePath);

private:
    QPixmap m_originalPixmap;
    QPixmap m_scaledPixmap;
    QSize    m_renderSize;

    void rescale();

    // 全局共享的原始 pixmap 缓存（避免重复读取文件）
    static QMap<QString, QPixmap> s_cache;
};