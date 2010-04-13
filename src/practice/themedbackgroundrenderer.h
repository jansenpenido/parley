/***************************************************************************
    Copyright 2010 Daniel Laidig <laidig@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICE_THEMEDBACKGROUNDRENDERER_H
#define PRACTICE_THEMEDBACKGROUNDRENDERER_H

#include <QObject>

#include "imagecache.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <ksvgrenderer.h>

class QMargins;

namespace Practice {

class ThemedBackgroundRenderer : public QObject
{
    Q_OBJECT

public:
    enum ScaleBase {
        NoScale,
        Horizontal,
        Vertical,
        Rect
    };

    enum Edge {
        Top,
        Bottom,
        Left,
        Right,
        Center
    };

    enum Align {
        Corner,
        LeftTop, // left or top (depending on orientation of the edge)
        Centered,
        RightBottom // right or bottom (depending on orientation of the edge)
    };

    ThemedBackgroundRenderer(QObject* parent = 0);
    ~ThemedBackgroundRenderer();

    void setSvgFilename(const QString& filename);

    QSizeF getSizeForId(const QString& id);
    QPixmap getPixmapForId(const QString& id, QSize size = QSize());
    QMargins contentMargins();

    QPixmap getScaledBackground();
    
public Q_SLOTS:
    void setSize(const QSize& size);
    void clearRects();
    void addRect(const QString& name, const QRect& rect);
    void updateBackground();

    void updateBackgroundTimeout();

    void renderingFinished();

signals:
    void backgroundChanged(QPixmap pixmap);
   
private:
    QImage renderBackground(bool fastScale);
    void renderRect(const QString& name, const QRect& rect, QPainter *p, bool fastScale);
    void renderItem(const QString& id, const QRect& rect, QPainter *p, bool fastScale, ScaleBase scaleBase, Qt::AspectRatioMode aspectRatio, Edge edge, Align align, bool inside);
    QRect scaleRect(QRectF itemRect, const QRect& baseRect, ScaleBase scaleBase, Qt::AspectRatioMode aspectRatio);
    QRect alignRect(QRect itemRect, const QRect& baseRect, Edge edge, Align align, bool inside);

    ImageCache m_cache;
    QFuture<QImage> m_future;
    QFutureWatcher<QImage> m_watcher;
    KSvgRenderer m_renderer;
    QList<QPair<QString, QRect> > m_rects;
    QSize m_size;
    bool m_queuedRequest;
    bool m_isFastScaledRender;
    QTimer m_timer;
};

}

#endif // PRACTICE_THEMEDBACKGROUNDRENDERER_H
