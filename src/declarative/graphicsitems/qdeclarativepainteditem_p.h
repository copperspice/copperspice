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

#ifndef QDeclarativePaintedItem_P_H
#define QDeclarativePaintedItem_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativePaintedItemPrivate;
class QDeclarativePaintedItem : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativePaintedItem)

   DECL_CS_PROPERTY_READ(contentsSize, contentsSize)
   DECL_CS_PROPERTY_WRITE(contentsSize, setContentsSize)
   DECL_CS_PROPERTY_NOTIFY(contentsSize, contentsSizeChanged)
   DECL_CS_PROPERTY_READ(fillColor, fillColor)
   DECL_CS_PROPERTY_WRITE(fillColor, setFillColor)
   DECL_CS_PROPERTY_NOTIFY(fillColor, fillColorChanged)
   DECL_CS_PROPERTY_READ(pixelCacheSize, pixelCacheSize)
   DECL_CS_PROPERTY_WRITE(pixelCacheSize, setPixelCacheSize)
   DECL_CS_PROPERTY_READ(smoothCache, smoothCache)
   DECL_CS_PROPERTY_WRITE(smoothCache, setSmoothCache)
   DECL_CS_PROPERTY_READ(contentsScale, contentsScale)
   DECL_CS_PROPERTY_WRITE(contentsScale, setContentsScale)
   DECL_CS_PROPERTY_NOTIFY(contentsScale, contentsScaleChanged)

 public:
   QDeclarativePaintedItem(QDeclarativeItem *parent = 0);
   ~QDeclarativePaintedItem();

   QSize contentsSize() const;
   void setContentsSize(const QSize &);

   qreal contentsScale() const;
   void setContentsScale(qreal);

   int pixelCacheSize() const;
   void setPixelCacheSize(int pixels);

   bool smoothCache() const;
   void setSmoothCache(bool on);

   QColor fillColor() const;
   void setFillColor(const QColor &);

   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

   DECL_CS_SIGNAL_1(Public, void fillColorChanged())
   DECL_CS_SIGNAL_2(fillColorChanged)
   DECL_CS_SIGNAL_1(Public, void contentsSizeChanged())
   DECL_CS_SIGNAL_2(contentsSizeChanged)
   DECL_CS_SIGNAL_1(Public, void contentsScaleChanged())
   DECL_CS_SIGNAL_2(contentsScaleChanged)

 protected:
   QDeclarativePaintedItem(QDeclarativePaintedItemPrivate &dd, QDeclarativeItem *parent);

   virtual void drawContents(QPainter *p, const QRect &) = 0;
   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

   void setCacheFrozen(bool);
   QRectF boundingRect() const;

   DECL_CS_SLOT_1(Protected, void dirtyCache(const QRect &un_named_arg1))
   DECL_CS_SLOT_2(dirtyCache)
   DECL_CS_SLOT_1(Protected, void clearCache())
   DECL_CS_SLOT_2(clearCache)

 private:
   Q_DISABLE_COPY(QDeclarativePaintedItem)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativePaintedItem)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePaintedItem)

#endif
