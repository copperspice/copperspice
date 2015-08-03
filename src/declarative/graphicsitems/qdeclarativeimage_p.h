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

#ifndef QDECLARATIVEIMAGE_P_H
#define QDECLARATIVEIMAGE_P_H

#include <qdeclarativeimagebase_p.h>
#include <QtNetwork/qnetworkreply.h>

QT_BEGIN_NAMESPACE

class QDeclarativeImagePrivate;

class QDeclarativeImage : public QDeclarativeImageBase
{
   DECL_CS_OBJECT(QDeclarativeImage)
   CS_ENUM(FillMode)

   CS_PROPERTY_READ(fillMode, fillMode)
   CS_PROPERTY_WRITE(fillMode, setFillMode)
   CS_PROPERTY_NOTIFY(fillMode, fillModeChanged)
   CS_PROPERTY_READ(paintedWidth, paintedWidth)
   CS_PROPERTY_NOTIFY(paintedWidth, paintedGeometryChanged)
   CS_PROPERTY_READ(paintedHeight, paintedHeight)
   CS_PROPERTY_NOTIFY(paintedHeight, paintedGeometryChanged)

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

   CS_SIGNAL_1(Public, void fillModeChanged())
   CS_SIGNAL_2(fillModeChanged)
   CS_SIGNAL_1(Public, void paintedGeometryChanged())
   CS_SIGNAL_2(paintedGeometryChanged)

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
