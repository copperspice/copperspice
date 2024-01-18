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

#ifndef QGRAPHICSVIDEOITEM_H
#define QGRAPHICSVIDEOITEM_H

#include <qgraphicsitem.h>

#include <qvideowidget.h>
#include <qmediabindableinterface.h>

class QVideoSurfaceFormat;
class QGraphicsVideoItemPrivate;

class Q_MULTIMEDIA_EXPORT QGraphicsVideoItem : public QGraphicsObject, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QGraphicsVideoItem, QGraphicsObject)

   CS_INTERFACES(QMediaBindableInterface)

   MULTI_CS_PROPERTY_READ(mediaObject, mediaObject)
   MULTI_CS_PROPERTY_WRITE(mediaObject, cs_setMediaObject)

   MULTI_CS_PROPERTY_READ(aspectRatioMode, aspectRatioMode)
   MULTI_CS_PROPERTY_WRITE(aspectRatioMode, setAspectRatioMode)

   MULTI_CS_PROPERTY_READ(offset, offset)
   MULTI_CS_PROPERTY_WRITE(offset, setOffset)

   MULTI_CS_PROPERTY_READ(size, size)
   MULTI_CS_PROPERTY_WRITE(size, setSize)

   MULTI_CS_PROPERTY_READ(nativeSize, nativeSize)
   MULTI_CS_PROPERTY_NOTIFY(nativeSize, nativeSizeChanged)

 public:
   explicit QGraphicsVideoItem(QGraphicsItem *parent = nullptr);
   ~QGraphicsVideoItem();

   QMediaObject *mediaObject() const override;

   Qt::AspectRatioMode aspectRatioMode() const;
   void setAspectRatioMode(Qt::AspectRatioMode mode);

   QPointF offset() const;
   void setOffset(const QPointF &offset);

   QSizeF size() const;
   void setSize(const QSizeF &size);

   QSizeF nativeSize() const;

   QRectF boundingRect() const override;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   MULTI_CS_SIGNAL_1(Public, void nativeSizeChanged(const QSizeF &size))
   MULTI_CS_SIGNAL_2(nativeSizeChanged, size)

 protected:
   void timerEvent(QTimerEvent *event) override;
   QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

   bool setMediaObject(QMediaObject *object) override;

   QGraphicsVideoItemPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QGraphicsVideoItem)

   // wrapper
   void cs_setMediaObject(QMediaObject *object) {
      setMediaObject(object);
   }

   MULTI_CS_SLOT_1(Private, void _q_present())
   MULTI_CS_SLOT_2(_q_present)

   MULTI_CS_SLOT_1(Private, void _q_updateNativeSize())
   MULTI_CS_SLOT_2(_q_updateNativeSize)

   MULTI_CS_SLOT_1(Private, void _q_serviceDestroyed())
   MULTI_CS_SLOT_2(_q_serviceDestroyed)
};

#endif

