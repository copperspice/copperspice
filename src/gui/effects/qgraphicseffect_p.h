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

#ifndef QGRAPHICSEFFECT_P_H
#define QGRAPHICSEFFECT_P_H

#include <qgraphicseffect.h>

#include <qpixmapcache.h>
#include <qscopedpointer.h>

#include <qpixmapfilter_p.h>

#ifndef QT_NO_GRAPHICSEFFECT

class QGraphicsEffectSourcePrivate;

class Q_GUI_EXPORT QGraphicsEffectSource : public QObject
{
   GUI_CS_OBJECT(QGraphicsEffectSource)

 public:
    QGraphicsEffectSource(const QGraphicsEffectSource &) = delete;
    QGraphicsEffectSource &operator=(const QGraphicsEffectSource &) = delete;

   ~QGraphicsEffectSource();
   const QGraphicsItem *graphicsItem() const;
   const QWidget *widget() const;
   const QStyleOption *styleOption() const;

   bool isPixmap() const;
   void draw(QPainter *painter);
   void update();

   QRectF boundingRect(Qt::CoordinateSystem coordinateSystem = Qt::LogicalCoordinates) const;
   QRect deviceRect() const;
   QPixmap pixmap(Qt::CoordinateSystem system = Qt::LogicalCoordinates,
                  QPoint *offset = nullptr, QGraphicsEffect::PixmapPadMode mode = QGraphicsEffect::PadToEffectiveBoundingRect) const;

 protected:
   QGraphicsEffectSource(QGraphicsEffectSourcePrivate &dd, QObject *parent = nullptr);

   QScopedPointer<QGraphicsEffectSourcePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QGraphicsEffectSource)

   friend class QGraphicsEffect;
   friend class QGraphicsEffectPrivate;
   friend class QGraphicsScenePrivate;
   friend class QGraphicsItem;
   friend class QGraphicsItemPrivate;
   friend class QWidget;
   friend class QWidgetPrivate;
};

class QGraphicsEffectSourcePrivate
{
   Q_DECLARE_PUBLIC(QGraphicsEffectSource)

 public:
   QGraphicsEffectSourcePrivate()
      : m_cachedSystem(Qt::DeviceCoordinates) , m_cachedMode(QGraphicsEffect::PadToTransparentBorder) {}

   enum InvalidateReason {
      TransformChanged,
      EffectRectChanged,
      SourceChanged
   };

   virtual ~QGraphicsEffectSourcePrivate();

   virtual void detach() = 0;
   virtual QRectF boundingRect(Qt::CoordinateSystem system) const = 0;
   virtual QRect deviceRect() const = 0;
   virtual const QGraphicsItem *graphicsItem() const = 0;
   virtual const QWidget *widget() const = 0;
   virtual const QStyleOption *styleOption() const = 0;
   virtual void draw(QPainter *p) = 0;
   virtual void update() = 0;
   virtual bool isPixmap() const = 0;
   virtual QPixmap pixmap(Qt::CoordinateSystem system, QPoint *offset = nullptr,
                          QGraphicsEffect::PixmapPadMode mode = QGraphicsEffect::PadToTransparentBorder) const = 0;
   virtual void effectBoundingRectChanged() = 0;

   void setCachedOffset(const QPoint &offset);
   void invalidateCache(InvalidateReason reason = SourceChanged) const;
   Qt::CoordinateSystem currentCachedSystem() const {
      return m_cachedSystem;
   }
   QGraphicsEffect::PixmapPadMode currentCachedMode() const {
      return m_cachedMode;
   }

   friend class QGraphicsScenePrivate;
   friend class QGraphicsItem;
   friend class QGraphicsItemPrivate;

 protected:
   QGraphicsEffectSource *q_ptr;

 private:
   mutable Qt::CoordinateSystem m_cachedSystem;
   mutable QGraphicsEffect::PixmapPadMode m_cachedMode;
   mutable QPoint m_cachedOffset;
   mutable QPixmapCache::Key m_cacheKey;
};


class Q_GUI_EXPORT QGraphicsEffectPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsEffect)

 public:
   QGraphicsEffectPrivate() : source(nullptr), isEnabled(1) {}
   virtual ~QGraphicsEffectPrivate() {}

   void setGraphicsEffectSource(QGraphicsEffectSource *newSource);

   QGraphicsEffectSource *source;
   QRectF boundingRect;
   quint32 isEnabled : 1;
   quint32 padding : 31; // feel free to use

 protected:
   QGraphicsEffect *q_ptr;

};

class QGraphicsColorizeEffectPrivate : public QGraphicsEffectPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsColorizeEffect)

 public:
   QGraphicsColorizeEffectPrivate()
      : opaque(true)
   {
      filter = new QPixmapColorizeFilter;
   }

   ~QGraphicsColorizeEffectPrivate()
   {
      delete filter;
   }

   QPixmapColorizeFilter *filter;
   quint32 opaque : 1;
   quint32 padding : 31;
};

class QGraphicsBlurEffectPrivate : public QGraphicsEffectPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsBlurEffect)

 public:
   QGraphicsBlurEffectPrivate() : filter(new QPixmapBlurFilter)
   {
   }

   ~QGraphicsBlurEffectPrivate()
   {
      delete filter;
   }

   QPixmapBlurFilter *filter;
};

class QGraphicsDropShadowEffectPrivate : public QGraphicsEffectPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsDropShadowEffect)

 public:
   QGraphicsDropShadowEffectPrivate() : filter(new QPixmapDropShadowFilter)
   {
   }

   ~QGraphicsDropShadowEffectPrivate()
   {
      delete filter;
   }

   QPixmapDropShadowFilter *filter;
};

class QGraphicsOpacityEffectPrivate : public QGraphicsEffectPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsOpacityEffect)

 public:
   QGraphicsOpacityEffectPrivate()
      : opacity(qreal(0.7)), isFullyTransparent(0), isFullyOpaque(0), hasOpacityMask(0) {}
   ~QGraphicsOpacityEffectPrivate() {}

   qreal opacity;
   QBrush opacityMask;
   uint isFullyTransparent : 1;
   uint isFullyOpaque : 1;
   uint hasOpacityMask : 1;
};

#endif // QT_NO_GRAPHICSEFFECT

#endif

