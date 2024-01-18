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
   DECL_CS_OBJECT(QDeclarativeBorderImage)
   DECL_CS_ENUM(TileMode)

   DECL_CS_PROPERTY_READ(*border, border)
   DECL_CS_PROPERTY_CONSTANT(*border)
   DECL_CS_PROPERTY_READ(horizontalTileMode, horizontalTileMode)
   DECL_CS_PROPERTY_WRITE(horizontalTileMode, setHorizontalTileMode)
   DECL_CS_PROPERTY_NOTIFY(horizontalTileMode, horizontalTileModeChanged)
   DECL_CS_PROPERTY_READ(verticalTileMode, verticalTileMode)
   DECL_CS_PROPERTY_WRITE(verticalTileMode, setVerticalTileMode)
   DECL_CS_PROPERTY_NOTIFY(verticalTileMode, verticalTileModeChanged)

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

   DECL_CS_SIGNAL_1(Public, void horizontalTileModeChanged())
   DECL_CS_SIGNAL_2(horizontalTileModeChanged)
   DECL_CS_SIGNAL_1(Public, void verticalTileModeChanged())
   DECL_CS_SIGNAL_2(verticalTileModeChanged)

 protected:
   virtual void load();

 private:
   void setGridScaledImage(const QDeclarativeGridScaledImage &sci);

   DECL_CS_SLOT_1(Private, void doUpdate())
   DECL_CS_SLOT_2(doUpdate)
   DECL_CS_SLOT_1(Private, void requestFinished())
   DECL_CS_SLOT_2(requestFinished)
   DECL_CS_SLOT_1(Private, void sciRequestFinished())
   DECL_CS_SLOT_2(sciRequestFinished)

   Q_DISABLE_COPY(QDeclarativeBorderImage)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeBorderImage)
};

QT_END_NAMESPACE
QML_DECLARE_TYPE(QDeclarativeBorderImage)


#endif // QDECLARATIVEBORDERIMAGE_H
