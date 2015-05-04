/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDeclarativePaintedItem_P_H
#define QDeclarativePaintedItem_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativePaintedItemPrivate;
class QDeclarativePaintedItem : public QDeclarativeItem
{
   CS_OBJECT(QDeclarativePaintedItem)

   CS_PROPERTY_READ(contentsSize, contentsSize)
   CS_PROPERTY_WRITE(contentsSize, setContentsSize)
   CS_PROPERTY_NOTIFY(contentsSize, contentsSizeChanged)
   CS_PROPERTY_READ(fillColor, fillColor)
   CS_PROPERTY_WRITE(fillColor, setFillColor)
   CS_PROPERTY_NOTIFY(fillColor, fillColorChanged)
   CS_PROPERTY_READ(pixelCacheSize, pixelCacheSize)
   CS_PROPERTY_WRITE(pixelCacheSize, setPixelCacheSize)
   CS_PROPERTY_READ(smoothCache, smoothCache)
   CS_PROPERTY_WRITE(smoothCache, setSmoothCache)
   CS_PROPERTY_READ(contentsScale, contentsScale)
   CS_PROPERTY_WRITE(contentsScale, setContentsScale)
   CS_PROPERTY_NOTIFY(contentsScale, contentsScaleChanged)

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

   CS_SIGNAL_1(Public, void fillColorChanged())
   CS_SIGNAL_2(fillColorChanged)
   CS_SIGNAL_1(Public, void contentsSizeChanged())
   CS_SIGNAL_2(contentsSizeChanged)
   CS_SIGNAL_1(Public, void contentsScaleChanged())
   CS_SIGNAL_2(contentsScaleChanged)

 protected:
   QDeclarativePaintedItem(QDeclarativePaintedItemPrivate &dd, QDeclarativeItem *parent);

   virtual void drawContents(QPainter *p, const QRect &) = 0;
   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

   void setCacheFrozen(bool);
   QRectF boundingRect() const;

   CS_SLOT_1(Protected, void dirtyCache(const QRect &un_named_arg1))
   CS_SLOT_2(dirtyCache)
   CS_SLOT_1(Protected, void clearCache())
   CS_SLOT_2(clearCache)

 private:
   Q_DISABLE_COPY(QDeclarativePaintedItem)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativePaintedItem)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePaintedItem)

#endif
