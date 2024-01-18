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

#ifndef QDECLARATIVEIMAGE_P_H
#define QDECLARATIVEIMAGE_P_H

#include <qdeclarativeimagebase_p.h>
#include <QtNetwork/qnetworkreply.h>

QT_BEGIN_NAMESPACE

class QDeclarativeImagePrivate;

class QDeclarativeImage : public QDeclarativeImageBase
{
   DECL_CS_OBJECT(QDeclarativeImage)
   DECL_CS_ENUM(FillMode)

   DECL_CS_PROPERTY_READ(fillMode, fillMode)
   DECL_CS_PROPERTY_WRITE(fillMode, setFillMode)
   DECL_CS_PROPERTY_NOTIFY(fillMode, fillModeChanged)
   DECL_CS_PROPERTY_READ(paintedWidth, paintedWidth)
   DECL_CS_PROPERTY_NOTIFY(paintedWidth, paintedGeometryChanged)
   DECL_CS_PROPERTY_READ(paintedHeight, paintedHeight)
   DECL_CS_PROPERTY_NOTIFY(paintedHeight, paintedGeometryChanged)

 public:
   QDeclarativeImage(QDeclarativeItem *parent = 0);
   ~QDeclarativeImage();

   enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop, Tile, TileVertically, TileHorizontally };
   FillMode fillMode() const;
   void setFillMode(FillMode);

   QPixmap pixmap() const;
   void setPixmap(const QPixmap &);

   qreal paintedWidth() const;
   qreal paintedHeight() const;

   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
   QRectF boundingRect() const;

   DECL_CS_SIGNAL_1(Public, void fillModeChanged())
   DECL_CS_SIGNAL_2(fillModeChanged)
   DECL_CS_SIGNAL_1(Public, void paintedGeometryChanged())
   DECL_CS_SIGNAL_2(paintedGeometryChanged)

 protected:
   QDeclarativeImage(QDeclarativeImagePrivate &dd, QDeclarativeItem *parent);
   void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
   void pixmapChange();
   void updatePaintedGeometry();

 private:
   Q_DISABLE_COPY(QDeclarativeImage)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeImage)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeImage)


#endif // QDECLARATIVEIMAGE_H
