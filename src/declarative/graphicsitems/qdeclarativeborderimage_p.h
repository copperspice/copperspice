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

#ifndef QDECLARATIVEBORDERIMAGE_P_H
#define QDECLARATIVEBORDERIMAGE_P_H

#include <qdeclarativeimagebase_p.h>
#include <QtNetwork/qnetworkreply.h>

QT_BEGIN_NAMESPACE

class QDeclarativeScaleGrid;
class QDeclarativeGridScaledImage;
class QDeclarativeBorderImagePrivate;

class QDeclarativeBorderImage : public QDeclarativeImageBase
{
   CS_OBJECT(QDeclarativeBorderImage)
   CS_ENUM(TileMode)

   CS_PROPERTY_READ(*border, border)
   CS_PROPERTY_CONSTANT(*border)
   CS_PROPERTY_READ(horizontalTileMode, horizontalTileMode)
   CS_PROPERTY_WRITE(horizontalTileMode, setHorizontalTileMode)
   CS_PROPERTY_NOTIFY(horizontalTileMode, horizontalTileModeChanged)
   CS_PROPERTY_READ(verticalTileMode, verticalTileMode)
   CS_PROPERTY_WRITE(verticalTileMode, setVerticalTileMode)
   CS_PROPERTY_NOTIFY(verticalTileMode, verticalTileModeChanged)

 public:
   QDeclarativeBorderImage(QDeclarativeItem *parent = 0);
   ~QDeclarativeBorderImage();

   QDeclarativeScaleGrid *border();

   enum TileMode { Stretch = Qt::StretchTile, Repeat = Qt::RepeatTile, Round = Qt::RoundTile };

   TileMode horizontalTileMode() const;
   void setHorizontalTileMode(TileMode);

   TileMode verticalTileMode() const;
   void setVerticalTileMode(TileMode);

   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
   void setSource(const QUrl &url);

   void setSourceSize(const QSize &);

   CS_SIGNAL_1(Public, void horizontalTileModeChanged())
   CS_SIGNAL_2(horizontalTileModeChanged)
   CS_SIGNAL_1(Public, void verticalTileModeChanged())
   CS_SIGNAL_2(verticalTileModeChanged)

 protected:
   virtual void load();

 private:
   void setGridScaledImage(const QDeclarativeGridScaledImage &sci);

   CS_SLOT_1(Private, void doUpdate())
   CS_SLOT_2(doUpdate)
   CS_SLOT_1(Private, void requestFinished())
   CS_SLOT_2(requestFinished)
   CS_SLOT_1(Private, void sciRequestFinished())
   CS_SLOT_2(sciRequestFinished)

   Q_DISABLE_COPY(QDeclarativeBorderImage)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeBorderImage)
};

QT_END_NAMESPACE
QML_DECLARE_TYPE(QDeclarativeBorderImage)


#endif // QDECLARATIVEBORDERIMAGE_H
