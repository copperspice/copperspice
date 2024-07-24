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

#ifndef QGRAPHICSEFFECT_H
#define QGRAPHICSEFFECT_H

#include <qbrush.h>
#include <qcolor.h>
#include <qobject.h>
#include <qpoint.h>
#include <qrect.h>
#include <qscopedpointer.h>

#ifndef QT_NO_GRAPHICSEFFECT

class QGraphicsBlurEffectPrivate;
class QGraphicsColorizeEffectPrivate;
class QGraphicsDropShadowEffectPrivate;
class QGraphicsEffectPrivate;
class QGraphicsEffectSource;
class QGraphicsItem;
class QGraphicsOpacityEffectPrivate;
class QPainter;
class QPixmap;
class QStyleOption;

class Q_GUI_EXPORT QGraphicsEffect : public QObject
{
   GUI_CS_OBJECT(QGraphicsEffect)

   GUI_CS_FLAG(ChangeFlag, ChangeFlags)
   GUI_CS_PROPERTY_READ(enabled, isEnabled)
   GUI_CS_PROPERTY_WRITE(enabled, setEnabled)
   GUI_CS_PROPERTY_NOTIFY(enabled, enabledChanged)

 public:
   enum ChangeFlag {
      SourceAttached = 0x1,
      SourceDetached = 0x2,
      SourceBoundingRectChanged = 0x4,
      SourceInvalidated = 0x8
   };
   using ChangeFlags = QFlags<ChangeFlag>;

   enum PixmapPadMode {
      NoPad,
      PadToTransparentBorder,
      PadToEffectiveBoundingRect
   };

   QGraphicsEffect(QObject *parent = nullptr);

   QGraphicsEffect(const QGraphicsEffect &) = delete;
   QGraphicsEffect &operator=(const QGraphicsEffect &) = delete;

   virtual ~QGraphicsEffect();

   virtual QRectF boundingRectFor(const QRectF &rectF) const;
   QRectF boundingRect() const;

   bool isEnabled() const;

   QGraphicsEffectSource *source() const; // internal

   GUI_CS_SLOT_1(Public, void setEnabled(bool enable))
   GUI_CS_SLOT_2(setEnabled)
   GUI_CS_SLOT_1(Public, void update())
   GUI_CS_SLOT_2(update)

   GUI_CS_SIGNAL_1(Public, void enabledChanged(bool enabled))
   GUI_CS_SIGNAL_2(enabledChanged, enabled)

 protected:
   QGraphicsEffect(QGraphicsEffectPrivate &d, QObject *parent = nullptr);
   virtual void draw(QPainter *painter) = 0;
   virtual void sourceChanged(ChangeFlags flags);
   void updateBoundingRect();

   bool sourceIsPixmap() const;
   QRectF sourceBoundingRect(Qt::CoordinateSystem system = Qt::LogicalCoordinates) const;
   void drawSource(QPainter *painter);

   QPixmap sourcePixmap(Qt::CoordinateSystem system = Qt::LogicalCoordinates, QPoint *offset = nullptr,
                        PixmapPadMode mode = PadToEffectiveBoundingRect) const;

   QScopedPointer<QGraphicsEffectPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QGraphicsEffect)

   friend class QGraphicsItem;
   friend class QGraphicsItemPrivate;
   friend class QGraphicsScenePrivate;
   friend class QWidget;
   friend class QWidgetPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsEffect::ChangeFlags)

class Q_GUI_EXPORT QGraphicsColorizeEffect : public QGraphicsEffect
{
   GUI_CS_OBJECT(QGraphicsColorizeEffect)

   GUI_CS_PROPERTY_READ(color, color)
   GUI_CS_PROPERTY_WRITE(color, setColor)
   GUI_CS_PROPERTY_NOTIFY(color, colorChanged)
   GUI_CS_PROPERTY_READ(strength, strength)
   GUI_CS_PROPERTY_WRITE(strength, setStrength)
   GUI_CS_PROPERTY_NOTIFY(strength, strengthChanged)

 public:
   QGraphicsColorizeEffect(QObject *parent = nullptr);

   QGraphicsColorizeEffect(const QGraphicsColorizeEffect &) = delete;
   QGraphicsColorizeEffect &operator=(const QGraphicsColorizeEffect &) = delete;

   ~QGraphicsColorizeEffect();

   QColor color() const;
   qreal strength() const;

   GUI_CS_SLOT_1(Public, void setColor(const QColor &c))
   GUI_CS_SLOT_2(setColor)
   GUI_CS_SLOT_1(Public, void setStrength(qreal strength))
   GUI_CS_SLOT_2(setStrength)

   GUI_CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(colorChanged, color)
   GUI_CS_SIGNAL_1(Public, void strengthChanged(qreal strength))
   GUI_CS_SIGNAL_2(strengthChanged, strength)

 protected:
   void draw(QPainter *painter) override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsColorizeEffect)
};

class Q_GUI_EXPORT QGraphicsBlurEffect : public QGraphicsEffect
{
   GUI_CS_OBJECT(QGraphicsBlurEffect)

   GUI_CS_FLAG(BlurHint, BlurHints)
   GUI_CS_PROPERTY_READ(blurRadius, blurRadius)
   GUI_CS_PROPERTY_WRITE(blurRadius, setBlurRadius)
   GUI_CS_PROPERTY_NOTIFY(blurRadius, blurRadiusChanged)
   GUI_CS_PROPERTY_READ(blurHints, blurHints)
   GUI_CS_PROPERTY_WRITE(blurHints, setBlurHints)
   GUI_CS_PROPERTY_NOTIFY(blurHints, blurHintsChanged)

 public:
   enum BlurHint {
      PerformanceHint = 0x00,
      QualityHint = 0x01,
      AnimationHint = 0x02
   };
   using BlurHints = QFlags<BlurHint>;

   QGraphicsBlurEffect(QObject *parent = nullptr);

   QGraphicsBlurEffect(const QGraphicsBlurEffect &) = delete;
   QGraphicsBlurEffect &operator=(const QGraphicsBlurEffect &) = delete;

   ~QGraphicsBlurEffect();

   QRectF boundingRectFor(const QRectF &rect) const override;
   qreal blurRadius() const;
   BlurHints blurHints() const;

   GUI_CS_SLOT_1(Public, void setBlurRadius(qreal blurRadius))
   GUI_CS_SLOT_2(setBlurRadius)
   GUI_CS_SLOT_1(Public, void setBlurHints(BlurHints hints))
   GUI_CS_SLOT_2(setBlurHints)

   GUI_CS_SIGNAL_1(Public, void blurRadiusChanged(qreal blurRadius))
   GUI_CS_SIGNAL_2(blurRadiusChanged, blurRadius)
   GUI_CS_SIGNAL_1(Public, void blurHintsChanged(BlurHints hints))
   GUI_CS_SIGNAL_2(blurHintsChanged, hints)

 protected:
   void draw(QPainter *painter) override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsBlurEffect)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsBlurEffect::BlurHints)

class Q_GUI_EXPORT QGraphicsDropShadowEffect : public QGraphicsEffect
{
   GUI_CS_OBJECT(QGraphicsDropShadowEffect)

   GUI_CS_PROPERTY_READ(offset, offset)
   GUI_CS_PROPERTY_WRITE(offset, cs_setOffset)
   GUI_CS_PROPERTY_NOTIFY(offset, offsetChanged)

   GUI_CS_PROPERTY_READ(xOffset, xOffset)
   GUI_CS_PROPERTY_WRITE(xOffset, setXOffset)
   GUI_CS_PROPERTY_NOTIFY(xOffset, offsetChanged)

   GUI_CS_PROPERTY_READ(yOffset, yOffset)
   GUI_CS_PROPERTY_WRITE(yOffset, setYOffset)
   GUI_CS_PROPERTY_NOTIFY(yOffset, offsetChanged)

   GUI_CS_PROPERTY_READ(blurRadius, blurRadius)
   GUI_CS_PROPERTY_WRITE(blurRadius, setBlurRadius)
   GUI_CS_PROPERTY_NOTIFY(blurRadius, blurRadiusChanged)

   GUI_CS_PROPERTY_READ(color, color)
   GUI_CS_PROPERTY_WRITE(color, setColor)
   GUI_CS_PROPERTY_NOTIFY(color, colorChanged)

 public:
   QGraphicsDropShadowEffect(QObject *parent = nullptr);

   QGraphicsDropShadowEffect(const QGraphicsDropShadowEffect &) = delete;
   QGraphicsDropShadowEffect &operator=(const QGraphicsDropShadowEffect &) = delete;

   ~QGraphicsDropShadowEffect();

   QRectF boundingRectFor(const QRectF &rect) const override;
   QPointF offset() const;

   inline qreal xOffset() const;
   inline qreal yOffset() const;

   qreal blurRadius() const;
   QColor color() const;

   GUI_CS_SLOT_1(Public, void setOffset(const QPointF &ofs))
   GUI_CS_SLOT_OVERLOAD(setOffset, (const QPointF &))

   GUI_CS_SLOT_1(Public, void setOffset(qreal dx, qreal dy))
   GUI_CS_SLOT_OVERLOAD(setOffset, (qreal, qreal))

   GUI_CS_SLOT_1(Public, void setOffset(qreal d))
   GUI_CS_SLOT_OVERLOAD(setOffset, (qreal))

   GUI_CS_SLOT_1(Public, inline void setXOffset(qreal dx))
   GUI_CS_SLOT_2(setXOffset)

   GUI_CS_SLOT_1(Public, inline void setYOffset(qreal dy))
   GUI_CS_SLOT_2(setYOffset)

   GUI_CS_SLOT_1(Public, void setBlurRadius(qreal blurRadius))
   GUI_CS_SLOT_2(setBlurRadius)

   GUI_CS_SLOT_1(Public, void setColor(const QColor &color))
   GUI_CS_SLOT_2(setColor)

   GUI_CS_SIGNAL_1(Public, void offsetChanged(const QPointF &offset))
   GUI_CS_SIGNAL_2(offsetChanged, offset)

   GUI_CS_SIGNAL_1(Public, void blurRadiusChanged(qreal blurRadius))
   GUI_CS_SIGNAL_2(blurRadiusChanged, blurRadius)

   GUI_CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(colorChanged, color)

   // wrapper for static method
   inline void cs_setOffset(const QPointF &ofs);

 protected:
   void draw(QPainter *painter) override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsDropShadowEffect)
};

class Q_GUI_EXPORT QGraphicsOpacityEffect: public QGraphicsEffect
{
   GUI_CS_OBJECT(QGraphicsOpacityEffect)

   GUI_CS_PROPERTY_READ(opacity, opacity)
   GUI_CS_PROPERTY_WRITE(opacity, setOpacity)
   GUI_CS_PROPERTY_NOTIFY(opacity, opacityChanged)

   GUI_CS_PROPERTY_READ(opacityMask, opacityMask)
   GUI_CS_PROPERTY_WRITE(opacityMask, setOpacityMask)
   GUI_CS_PROPERTY_NOTIFY(opacityMask, opacityMaskChanged)

 public:
   QGraphicsOpacityEffect(QObject *parent = nullptr);

   QGraphicsOpacityEffect(const QGraphicsOpacityEffect &) = delete;
   QGraphicsOpacityEffect &operator=(const QGraphicsOpacityEffect &) = delete;

   ~QGraphicsOpacityEffect();

   qreal opacity() const;
   QBrush opacityMask() const;

   GUI_CS_SLOT_1(Public, void setOpacity(qreal opacity))
   GUI_CS_SLOT_2(setOpacity)
   GUI_CS_SLOT_1(Public, void setOpacityMask(const QBrush &mask))
   GUI_CS_SLOT_2(setOpacityMask)

   GUI_CS_SIGNAL_1(Public, void opacityChanged(qreal opacity))
   GUI_CS_SIGNAL_2(opacityChanged, opacity)
   GUI_CS_SIGNAL_1(Public, void opacityMaskChanged(const QBrush &mask))
   GUI_CS_SIGNAL_2(opacityMaskChanged, mask)

 protected:
   void draw(QPainter *painter) override;

 private:
   Q_DECLARE_PRIVATE(QGraphicsOpacityEffect)
};

inline qreal QGraphicsDropShadowEffect::xOffset() const
{
   return offset().x();
}

inline qreal QGraphicsDropShadowEffect::yOffset() const
{
   return offset().y();
}

inline void QGraphicsDropShadowEffect::setXOffset(qreal dx)
{
   setOffset(QPointF(dx, yOffset()));
}

inline void QGraphicsDropShadowEffect::setYOffset(qreal dy)
{
   setOffset(QPointF(xOffset(), dy));
}

inline void QGraphicsDropShadowEffect::cs_setOffset(const QPointF &ofs)
{
   setOffset(ofs);
}

#endif //QT_NO_GRAPHICSEFFECT

#endif

