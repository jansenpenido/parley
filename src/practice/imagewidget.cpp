/***************************************************************************
    Copyright 2009 Daniel Laidig <d.laidig@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "imagewidget.h"
#include "../config-parley.h"

#include <QtGui/QPainter>
#include <QtGui/QPaintEngine>
#include <QtCore/QTimer>
#include <QtCore/QTimeLine>

#include <kdebug.h>

#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <QX11Info>
#undef KeyPress
#undef FocusOut
#endif

using namespace Practice;

// The functions centerPixmaps() and transition() are copied from kdelibs/plasma/paintutils.cpp, revision 1114033
// License: LGPLv2+
// Copyright 2005 by Aaron Seigo <aseigo@kde.org>
// Copyright 2008 by Andrew Lake <jamboarder@yahoo.com>
// Don't just modify the code here, if there are issues they should probably also be fixed in libplasma.

void centerPixmaps(QPixmap &from, QPixmap &to)
{
    if (from.size() == to.size()) {
        return;
    }
    QRect fromRect(from.rect());
    QRect toRect(to.rect());

    QRect actualRect = QRect(QPoint(0,0), fromRect.size().expandedTo(toRect.size()));
    fromRect.moveCenter(actualRect.center());
    toRect.moveCenter(actualRect.center());

    if (from.size() != actualRect.size()) {
        QPixmap result(actualRect.size());
        result.fill(Qt::transparent);
        QPainter p(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(fromRect.topLeft(), from);
        p.end();
        from = result;
    }

    if (to.size() != actualRect.size()) {
        QPixmap result(actualRect.size());
        result.fill(Qt::transparent);
        QPainter p(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(toRect.topLeft(), to);
        p.end();
        to = result;
    }
}

QPixmap transition(const QPixmap &from, const QPixmap &to, qreal amount)
{
    if (from.isNull() || to.isNull()) {
        return from;
    }

    QPixmap startPixmap(from);
    QPixmap targetPixmap(to);

    if (from.size() != to.size()) {
        centerPixmaps(startPixmap, targetPixmap);
    }

    //paint to in the center of from
    QRect toRect = to.rect();

    QColor color;
    color.setAlphaF(amount);


    // If the native paint engine supports Porter/Duff compositing and CompositionMode_Plus
    QPaintEngine *paintEngine = from.paintEngine();
    if (paintEngine &&
        paintEngine->hasFeature(QPaintEngine::PorterDuff) &&
        paintEngine->hasFeature(QPaintEngine::BlendModes)) {

        QPainter p;
        p.begin(&targetPixmap);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(targetPixmap.rect(), color);
        p.end();

        p.begin(&startPixmap);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(startPixmap.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawPixmap(toRect.topLeft(), targetPixmap);
        p.end();

        return startPixmap;
    }
#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
    // We have Xrender support
    else if (paintEngine && paintEngine->hasFeature(QPaintEngine::PorterDuff)) {
        // QX11PaintEngine doesn't implement CompositionMode_Plus in Qt 4.3,
        // which we need to be able to do a transition from one pixmap to
        // another.
        //
        // In order to avoid the overhead of converting the pixmaps to images
        // and doing the operation entirely in software, this function has a
        // specialized path for X11 that uses Xrender directly to do the
        // transition. This operation can be fully accelerated in HW.
        //
        // This specialization can be removed when QX11PaintEngine supports
        // CompositionMode_Plus.
        QPixmap source(targetPixmap), destination(startPixmap);

        source.detach();
        destination.detach();

        Display *dpy = QX11Info::display();

        XRenderPictFormat *format = XRenderFindStandardFormat(dpy, PictStandardA8);
        XRenderPictureAttributes pa;
        pa.repeat = 1; // RepeatNormal

        // Create a 1x1 8 bit repeating alpha picture
        Pixmap pixmap = XCreatePixmap(dpy, destination.handle(), 1, 1, 8);
        Picture alpha = XRenderCreatePicture(dpy, pixmap, format, CPRepeat, &pa);
        XFreePixmap(dpy, pixmap);

        // Fill the alpha picture with the opacity value
        XRenderColor xcolor;
        xcolor.alpha = quint16(0xffff * amount);
        XRenderFillRectangle(dpy, PictOpSrc, alpha, &xcolor, 0, 0, 1, 1);

        // Reduce the alpha of the destination with 1 - opacity
        XRenderComposite(dpy, PictOpOutReverse, alpha, None, destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        // Add source * opacity to the destination
        XRenderComposite(dpy, PictOpAdd, source.x11PictureHandle(), alpha,
                         destination.x11PictureHandle(),
                         toRect.x(), toRect.y(), 0, 0, 0, 0, destination.width(), destination.height());

        XRenderFreePicture(dpy, alpha);
        return destination;
    }
#endif
    else {
        // Fall back to using QRasterPaintEngine to do the transition.
        QImage under = startPixmap.toImage();
        QImage over  = targetPixmap.toImage();

        QPainter p;
        p.begin(&over);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(under.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawImage(toRect.topLeft(), over);
        p.end();

        return QPixmap::fromImage(under);
    }
}


ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent), m_scaling(true), m_onlyDownscaling(true), m_keepAspectRatio(Qt::KeepAspectRatio), m_fading(true)
{
    m_scaleTimer = new QTimer(this);
    m_scaleTimer->setSingleShot(true);
    m_scaleTimer->setInterval(500);

    m_animation = new QTimeLine(300, this);

    m_scaledPixmapOutOfDate = false;
    connect(m_scaleTimer, SIGNAL(timeout()), this, SLOT(scalePixmap()));
    connect(m_animation, SIGNAL(valueChanged(qreal)), this, SLOT(update()));
    connect(m_animation, SIGNAL(finished()), this, SLOT(animationFinished()));
}

void ImageWidget::setPixmap(const QPixmap& pixmap)
{
    kDebug() << "set new pixmap, size:" << pixmap.size();
    if (m_animation->state() == QTimeLine::Running) {
        m_animation->stop();
        animationFinished();
    }

    m_animationPixmap = m_scaledPixmap;
    m_originalPixmap = pixmap;
    m_scaledPixmap = QPixmap();
    m_scaledBackupPixmap = QPixmap();
    m_scaledPixmapOutOfDate = true;
    if (!m_scaling) {
        m_scaledPixmap = pixmap;
    }
    scalePixmap(true);
    if (m_fading) {
        m_animation->start();
    }
    update();
}

void ImageWidget::setScalingEnabled(bool scaling, bool onlyDownscaling)
{
    m_scaling = scaling;
    m_onlyDownscaling = onlyDownscaling;
}

void ImageWidget::setKeepAspectRatio(Qt::AspectRatioMode mode)
{
    m_keepAspectRatio = mode;
}

void ImageWidget::setFadingEnabled(bool fading)
{
    m_fading = fading;
}

void ImageWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);
    QPainter painter(this);
    if (m_scaling && m_scaledPixmapOutOfDate) {
        m_scaleTimer->start();
        scalePixmap(false);
    }
    QPixmap pm = m_scaledPixmap;
    if (m_animation->state() == QTimeLine::Running) {
        pm = transition(m_animationPixmap, m_scaledPixmap, m_animation->currentValue());
    }

    int x = (size().width() - pm.width()) / 2;
    int y = (size().height() - pm.height()) / 2;
    painter.drawPixmap(x, y, pm);
}

void ImageWidget::resizeEvent(QResizeEvent* e)
{
    if (!m_scaledPixmapOutOfDate) {
        m_scaledBackupPixmap = m_scaledPixmap;
    }
    // stop animations when resizing
    if (m_animation->state() == QTimeLine::Running) {
        m_animation->stop();
        animationFinished();
    }
    m_scaledPixmapOutOfDate = true;
    QWidget::resizeEvent(e);
    emit sizeChanged();
}

void ImageWidget::scalePixmap(bool smooth)
{
    bool scaleUp = m_originalPixmap.width() <= size().width() && m_originalPixmap.height() <= size().height();
    if ((m_onlyDownscaling && scaleUp) || m_originalPixmap.size() == size()) {
        kDebug() << "no need to scale pixmap";
        m_scaledPixmapOutOfDate = false;
        m_scaledPixmap = m_originalPixmap;
        m_scaledBackupPixmap = QPixmap();
    } else if (smooth) {
        kDebug() << "smooth scaling to" << size();
        if (m_originalPixmap.isNull() || size().isEmpty()) {
            m_scaledPixmapOutOfDate = false;
            m_scaledPixmap = QPixmap();
            update();
            return;
        }
        m_scaledPixmap = m_originalPixmap.scaled(size(), m_keepAspectRatio, Qt::SmoothTransformation);
        m_scaledBackupPixmap = QPixmap();
        m_scaledPixmapOutOfDate = false;
        update();
    } else {
        kDebug() << "fast scaling to" << size();
        // try to find out if it makes sense to use the scaled backup pixmap
        // if the scaled backup gets too small, we use the orignal image
        float ratio = 0;
        if (!size().isEmpty()) {
            ratio = qMin(float(m_scaledBackupPixmap.width())/size().width(),
                        float(m_scaledBackupPixmap.height())/size().height());
        }
        if (ratio > 0.4 && !m_scaledBackupPixmap.isNull()) {
            m_scaledPixmap = m_scaledBackupPixmap.scaled(size(), m_keepAspectRatio, Qt::FastTransformation);
        } else {
            if (m_originalPixmap.isNull() || size().isEmpty()) {
                m_scaledPixmap = QPixmap();
                return;
            }
            // use the original pixmap
            m_scaledPixmap = m_originalPixmap.scaled(size(), m_keepAspectRatio, Qt::FastTransformation);
            m_scaledBackupPixmap = m_scaledPixmap;
        }
        m_scaledPixmapOutOfDate = true;
    }
}

void ImageWidget::animationFinished()
{
    m_animationPixmap = QPixmap();
}

#include "imagewidget.moc"
