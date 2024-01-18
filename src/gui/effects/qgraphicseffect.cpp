/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qgraphicseffect_p.h>

#include <qgraphicsitem.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qrect.h>
#include <qdebug.h>

#include <qdrawhelper_p.h>
#include <qgraphics_item_p.h>

#ifndef QT_NO_GRAPHICSEFFECT

QGraphicsEffectSource::QGraphicsEffectSource(QGraphicsEffectSourcePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QGraphicsEffectSource::~QGraphicsEffectSource()
{ }

QRectF QGraphicsEffectSource::boundingRect(Qt::CoordinateSystem system) const
{
   return d_func()->boundingRect(system);
}

QRectF QGraphicsEffect::sourceBoundingRect(Qt::CoordinateSystem system) const
{
   Q_D(const QGraphicsEffect);
   if (d->source) {
      return d->source->boundingRect(system);
   }
   return QRectF();
}

const QGraphicsItem *QGraphicsEffectSource::graphicsItem() const
{
   return d_func()->graphicsItem();
}

const QWidget *QGraphicsEffectSource::widget() const
{
   return d_func()->widget();
}

const QStyleOption *QGraphicsEffectSource::styleOption() const
{
   return d_func()->styleOption();
}

void QGraphicsEffectSource::draw(QPainter *painter)
{
   Q_D(const QGraphicsEffectSource);

   QPixmap pm;
   if (QPixmapCache::find(d->m_cacheKey, &pm)) {
      QTransform restoreTransform;
      if (d->m_cachedSystem == Qt::DeviceCoordinates) {
         restoreTransform = painter->worldTransform();
         painter->setWorldTransform(QTransform());
      }

      painter->drawPixmap(d->m_cachedOffset, pm);

      if (d->m_cachedSystem == Qt::DeviceCoordinates) {
         painter->setWorldTransform(restoreTransform);
      }
   } else {
      d_func()->draw(painter);
   }
}

void QGraphicsEffect::drawSource(QPainter *painter)
{
   Q_D(const QGraphicsEffect);
   if (d->source) {
      d->source->draw(painter);
   }
}

void QGraphicsEffectSource::update()
{
   d_func()->update();
}

bool QGraphicsEffectSource::isPixmap() const
{
   return d_func()->isPixmap();
}

bool QGraphicsEffect::sourceIsPixmap() const
{
   return source() ? source()->isPixmap() : false;
}

QPixmap QGraphicsEffectSource::pixmap(Qt::CoordinateSystem system, QPoint *offset,
                                      QGraphicsEffect::PixmapPadMode mode) const
{
   Q_D(const QGraphicsEffectSource);

   // Shortcut, no cache for childless pixmap items...
   const QGraphicsItem *item = graphicsItem();
   if (system == Qt::LogicalCoordinates && mode == QGraphicsEffect::NoPad && item && isPixmap()) {
      const QGraphicsPixmapItem *pixmapItem = static_cast<const QGraphicsPixmapItem *>(item);
      if (offset) {
         *offset = pixmapItem->offset().toPoint();
      }
      return pixmapItem->pixmap();
   }

   if (system == Qt::DeviceCoordinates && item
         && !static_cast<const QGraphicsItemEffectSourcePrivate *>(d_func())->info) {
      qWarning("QGraphicsEffectSource::pixmap() Device context is missing, unable to create the pixmap");
      return QPixmap();
   }

   QPixmap pm;
   if (item && d->m_cachedSystem == system && d->m_cachedMode == mode) {
      QPixmapCache::find(d->m_cacheKey, &pm);
   }

   if (pm.isNull()) {
      pm = d->pixmap(system, &d->m_cachedOffset, mode);
      d->m_cachedSystem = system;
      d->m_cachedMode = mode;

      d->invalidateCache();
      d->m_cacheKey = QPixmapCache::insert(pm);
   }

   if (offset) {
      *offset = d->m_cachedOffset;
   }

   return pm;
}

QPixmap QGraphicsEffect::sourcePixmap(Qt::CoordinateSystem system, QPoint *offset,
                                      QGraphicsEffect::PixmapPadMode mode) const
{
   Q_D(const QGraphicsEffect);
   if (d->source) {
      return d->source->pixmap(system, offset, mode);
   }
   return QPixmap();
}

QGraphicsEffectSourcePrivate::~QGraphicsEffectSourcePrivate()
{
   invalidateCache();
}

void QGraphicsEffectSourcePrivate::setCachedOffset(const QPoint &offset)
{
   m_cachedOffset = offset;
}

void QGraphicsEffectSourcePrivate::invalidateCache(InvalidateReason reason) const
{
   if (m_cachedMode != QGraphicsEffect::PadToEffectiveBoundingRect
         && (reason == EffectRectChanged
             || (reason == TransformChanged && m_cachedSystem == Qt::LogicalCoordinates))) {
      return;
   }

   QPixmapCache::remove(m_cacheKey);
}

QGraphicsEffect::QGraphicsEffect(QObject *parent)
   : QObject(parent), d_ptr(new QGraphicsEffectPrivate)
{
   d_ptr->q_ptr = this;
}

QGraphicsEffect::QGraphicsEffect(QGraphicsEffectPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QGraphicsEffect::~QGraphicsEffect()
{
   Q_D(QGraphicsEffect);
   d->setGraphicsEffectSource(nullptr);
}

QRectF QGraphicsEffect::boundingRect() const
{
   Q_D(const QGraphicsEffect);
   if (d->source) {
      return boundingRectFor(d->source->boundingRect());
   }
   return QRectF();
}

QRectF QGraphicsEffect::boundingRectFor(const QRectF &rect) const
{
   return rect;
}

bool QGraphicsEffect::isEnabled() const
{
   Q_D(const QGraphicsEffect);
   return d->isEnabled;
}

void QGraphicsEffect::setEnabled(bool enable)
{
   Q_D(QGraphicsEffect);
   if (d->isEnabled == enable) {
      return;
   }

   d->isEnabled = enable;
   if (d->source) {
      d->source->d_func()->effectBoundingRectChanged();
      d->source->d_func()->invalidateCache();
   }
   emit enabledChanged(enable);
}

void QGraphicsEffect::update()
{
   Q_D(QGraphicsEffect);
   if (d->source) {
      d->source->update();
   }
}

QGraphicsEffectSource *QGraphicsEffect::source() const
{
   Q_D(const QGraphicsEffect);
   return d->source;
}

void QGraphicsEffect::updateBoundingRect()
{
   Q_D(QGraphicsEffect);
   if (d->source) {
      d->source->d_func()->effectBoundingRectChanged();
      d->source->d_func()->invalidateCache(QGraphicsEffectSourcePrivate::EffectRectChanged);
   }
}

void QGraphicsEffect::sourceChanged(ChangeFlags flags)
{
   (void) flags;
}

QGraphicsColorizeEffect::QGraphicsColorizeEffect(QObject *parent)
   : QGraphicsEffect(*new QGraphicsColorizeEffectPrivate, parent)
{
}

QGraphicsColorizeEffect::~QGraphicsColorizeEffect()
{
}

QColor QGraphicsColorizeEffect::color() const
{
   Q_D(const QGraphicsColorizeEffect);
   return d->filter->color();
}

void QGraphicsColorizeEffect::setColor(const QColor &color)
{
   Q_D(QGraphicsColorizeEffect);
   if (d->filter->color() == color) {
      return;
   }

   d->filter->setColor(color);
   update();
   emit colorChanged(color);
}

qreal QGraphicsColorizeEffect::strength() const
{
   Q_D(const QGraphicsColorizeEffect);
   return d->filter->strength();
}

void QGraphicsColorizeEffect::setStrength(qreal strength)
{
   Q_D(QGraphicsColorizeEffect);
   if (qFuzzyCompare(d->filter->strength(), strength)) {
      return;
   }

   d->filter->setStrength(strength);
   d->opaque = !qFuzzyIsNull(strength);
   update();
   emit strengthChanged(strength);
}

void QGraphicsColorizeEffect::draw(QPainter *painter)
{
   Q_D(QGraphicsColorizeEffect);

   if (!d->opaque) {
      drawSource(painter);
      return;
   }

   QPoint offset;
   if (sourceIsPixmap()) {
      // No point in drawing in device coordinates (pixmap will be scaled anyways).
      const QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset, NoPad);
      if (!pixmap.isNull()) {
         d->filter->draw(painter, offset, pixmap);
      }

      return;
   }

   // Draw pixmap in deviceCoordinates to avoid pixmap scaling.
   const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
   if (pixmap.isNull()) {
      return;
   }

   QTransform restoreTransform = painter->worldTransform();
   painter->setWorldTransform(QTransform());
   d->filter->draw(painter, offset, pixmap);
   painter->setWorldTransform(restoreTransform);
}

QGraphicsBlurEffect::QGraphicsBlurEffect(QObject *parent)
   : QGraphicsEffect(*new QGraphicsBlurEffectPrivate, parent)
{
   Q_D(QGraphicsBlurEffect);
   d->filter->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
}

QGraphicsBlurEffect::~QGraphicsBlurEffect()
{
}

qreal QGraphicsBlurEffect::blurRadius() const
{
   Q_D(const QGraphicsBlurEffect);
   return d->filter->radius();
}

void QGraphicsBlurEffect::setBlurRadius(qreal radius)
{
   Q_D(QGraphicsBlurEffect);
   if (qFuzzyCompare(d->filter->radius(), radius)) {
      return;
   }

   d->filter->setRadius(radius);
   updateBoundingRect();
   emit blurRadiusChanged(radius);
}

QGraphicsBlurEffect::BlurHints QGraphicsBlurEffect::blurHints() const
{
   Q_D(const QGraphicsBlurEffect);
   return d->filter->blurHints();
}

void QGraphicsBlurEffect::setBlurHints(QGraphicsBlurEffect::BlurHints hints)
{
   Q_D(QGraphicsBlurEffect);
   if (d->filter->blurHints() == hints) {
      return;
   }

   d->filter->setBlurHints(hints);
   emit blurHintsChanged(hints);
}

QRectF QGraphicsBlurEffect::boundingRectFor(const QRectF &rect) const
{
   Q_D(const QGraphicsBlurEffect);
   return d->filter->boundingRectFor(rect);
}

void QGraphicsBlurEffect::draw(QPainter *painter)
{
   Q_D(QGraphicsBlurEffect);
   if (d->filter->radius() < 1) {
      drawSource(painter);
      return;
   }

   PixmapPadMode mode = PadToEffectiveBoundingRect;
   if (painter->paintEngine()->type() == QPaintEngine::OpenGL2) {
      mode = NoPad;
   }

   QPoint offset;
   QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset, mode);
   if (pixmap.isNull()) {
      return;
   }

   d->filter->draw(painter, offset, pixmap);
}

QGraphicsDropShadowEffect::QGraphicsDropShadowEffect(QObject *parent)
   : QGraphicsEffect(*new QGraphicsDropShadowEffectPrivate, parent)
{
}

QGraphicsDropShadowEffect::~QGraphicsDropShadowEffect()
{
}

QPointF QGraphicsDropShadowEffect::offset() const
{
   Q_D(const QGraphicsDropShadowEffect);
   return d->filter->offset();
}

void QGraphicsDropShadowEffect::setOffset(const QPointF &offset)
{
   Q_D(QGraphicsDropShadowEffect);
   if (d->filter->offset() == offset) {
      return;
   }

   d->filter->setOffset(offset);
   updateBoundingRect();
   emit offsetChanged(offset);
}

qreal QGraphicsDropShadowEffect::blurRadius() const
{
   Q_D(const QGraphicsDropShadowEffect);
   return d->filter->blurRadius();
}

void QGraphicsDropShadowEffect::setBlurRadius(qreal blurRadius)
{
   Q_D(QGraphicsDropShadowEffect);
   if (qFuzzyCompare(d->filter->blurRadius(), blurRadius)) {
      return;
   }

   d->filter->setBlurRadius(blurRadius);
   updateBoundingRect();
   emit blurRadiusChanged(blurRadius);
}

QColor QGraphicsDropShadowEffect::color() const
{
   Q_D(const QGraphicsDropShadowEffect);
   return d->filter->color();
}

void QGraphicsDropShadowEffect::setColor(const QColor &color)
{
   Q_D(QGraphicsDropShadowEffect);
   if (d->filter->color() == color) {
      return;
   }

   d->filter->setColor(color);
   update();
   emit colorChanged(color);
}

QRectF QGraphicsDropShadowEffect::boundingRectFor(const QRectF &rect) const
{
   Q_D(const QGraphicsDropShadowEffect);
   return d->filter->boundingRectFor(rect);
}

void QGraphicsDropShadowEffect::draw(QPainter *painter)
{
   Q_D(QGraphicsDropShadowEffect);
   if (d->filter->blurRadius() <= 0 && d->filter->offset().isNull()) {
      drawSource(painter);
      return;
   }

   PixmapPadMode mode = PadToEffectiveBoundingRect;
   if (painter->paintEngine()->type() == QPaintEngine::OpenGL2) {
      mode = NoPad;
   }

   // Draw pixmap in device coordinates to avoid pixmap scaling.
   QPoint offset;
   const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset, mode);
   if (pixmap.isNull()) {
      return;
   }

   QTransform restoreTransform = painter->worldTransform();
   painter->setWorldTransform(QTransform());
   d->filter->draw(painter, offset, pixmap);
   painter->setWorldTransform(restoreTransform);
}

QGraphicsOpacityEffect::QGraphicsOpacityEffect(QObject *parent)
   : QGraphicsEffect(*new QGraphicsOpacityEffectPrivate, parent)
{
}

QGraphicsOpacityEffect::~QGraphicsOpacityEffect()
{
}

qreal QGraphicsOpacityEffect::opacity() const
{
   Q_D(const QGraphicsOpacityEffect);
   return d->opacity;
}

void QGraphicsOpacityEffect::setOpacity(qreal opacity)
{
   Q_D(QGraphicsOpacityEffect);
   opacity = qBound(qreal(0.0), opacity, qreal(1.0));

   if (qFuzzyCompare(d->opacity, opacity)) {
      return;
   }

   d->opacity = opacity;
   if ((d->isFullyTransparent = qFuzzyIsNull(d->opacity))) {
      d->isFullyOpaque = 0;
   } else {
      d->isFullyOpaque = qFuzzyIsNull(d->opacity - 1);
   }
   update();
   emit opacityChanged(opacity);
}

QBrush QGraphicsOpacityEffect::opacityMask() const
{
   Q_D(const QGraphicsOpacityEffect);
   return d->opacityMask;
}

void QGraphicsOpacityEffect::setOpacityMask(const QBrush &mask)
{
   Q_D(QGraphicsOpacityEffect);
   if (d->opacityMask == mask) {
      return;
   }

   d->opacityMask = mask;
   d->hasOpacityMask = (mask.style() != Qt::NoBrush);
   update();

   emit opacityMaskChanged(mask);
}

void QGraphicsOpacityEffect::draw(QPainter *painter)
{
   Q_D(QGraphicsOpacityEffect);

   // Transparent; nothing to draw.
   if (d->isFullyTransparent) {
      return;
   }

   // Opaque; draw directly without going through a pixmap.
   if (d->isFullyOpaque && !d->hasOpacityMask) {
      drawSource(painter);
      return;
   }

   QPoint offset;
   Qt::CoordinateSystem system = sourceIsPixmap() ? Qt::LogicalCoordinates : Qt::DeviceCoordinates;
   QPixmap pixmap = sourcePixmap(system, &offset, QGraphicsEffect::NoPad);
   if (pixmap.isNull()) {
      return;
   }

   painter->save();
   painter->setOpacity(d->opacity);

   if (d->hasOpacityMask) {
      QPainter pixmapPainter(&pixmap);
      pixmapPainter.setRenderHints(painter->renderHints());
      pixmapPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
      if (system == Qt::DeviceCoordinates) {
         QTransform worldTransform = painter->worldTransform();
         worldTransform *= QTransform::fromTranslate(-offset.x(), -offset.y());
         pixmapPainter.setWorldTransform(worldTransform);
         pixmapPainter.fillRect(sourceBoundingRect(), d->opacityMask);
      } else {
         pixmapPainter.translate(-offset);
         pixmapPainter.fillRect(pixmap.rect(), d->opacityMask);
      }
   }

   if (system == Qt::DeviceCoordinates) {
      painter->setWorldTransform(QTransform());
   }

   painter->drawPixmap(offset, pixmap);
   painter->restore();
}

void QGraphicsEffectPrivate::setGraphicsEffectSource(QGraphicsEffectSource *newSource)
{
   QGraphicsEffect::ChangeFlags flags;
   if (source) {
      flags |= QGraphicsEffect::SourceDetached;
      source->d_func()->invalidateCache();
      source->d_func()->detach();
      delete source;
   }

   source = newSource;
   if (newSource) {
      flags |= QGraphicsEffect::SourceAttached;
   }

   q_func()->sourceChanged(flags);
}

void  QGraphicsDropShadowEffect::setOffset(qreal dx, qreal dy)
{
   setOffset(QPointF(dx, dy));
}

void  QGraphicsDropShadowEffect::setOffset(qreal d)
{
   setOffset(QPointF(d, d));
}

#endif //QT_NO_GRAPHICSEFFECT
