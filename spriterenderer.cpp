#include "spriterenderer.h"
#include <QtMath>
#include <QDebug>

// 全局静态缓存
QMap<QString, QPixmap> SpriteRenderer::s_cache;

SpriteRenderer::SpriteRenderer()
    : m_renderSize(0, 0)
{
}

SpriteRenderer::~SpriteRenderer()
{
}

QPixmap SpriteRenderer::cachedPixmap(const QString &resourcePath)
{
    if (s_cache.contains(resourcePath))
        return s_cache[resourcePath];

    QPixmap pix(resourcePath);
    if (pix.isNull()) {
        qWarning() << "SpriteRenderer: 无法加载资源:" << resourcePath;
        return QPixmap();
    }

    s_cache[resourcePath] = pix;
    return pix;
}

bool SpriteRenderer::load(const QString &resourcePath)
{
    m_originalPixmap = cachedPixmap(resourcePath);
    if (m_originalPixmap.isNull())
        return false;

    // 如果已有目标尺寸，立即缩放
    if (m_renderSize.isValid() && m_renderSize.width() > 0 && m_renderSize.height() > 0) {
        rescale();
    }
    return true;
}

void SpriteRenderer::setRenderSize(const QSize &size)
{
    if (m_renderSize == size)
        return;
    m_renderSize = size;
    if (!m_originalPixmap.isNull()) {
        rescale();
    }
}

void SpriteRenderer::setRenderSize(int width, int height)
{
    setRenderSize(QSize(width, height));
}

void SpriteRenderer::rescale()
{
    if (m_originalPixmap.isNull() || !m_renderSize.isValid() || m_renderSize.isEmpty())
        return;

    m_scaledPixmap = m_originalPixmap.scaled(
        m_renderSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
}

void SpriteRenderer::draw(QPainter *painter, const QPointF &center) const
{
    draw(painter, center.x(), center.y());
}

void SpriteRenderer::draw(QPainter *painter, qreal x, qreal y) const
{
    if (m_scaledPixmap.isNull() || !painter)
        return;

    QPointF topLeft(x - m_scaledPixmap.width() / 2.0,
                    y - m_scaledPixmap.height() / 2.0);
    painter->drawPixmap(topLeft, m_scaledPixmap);
}