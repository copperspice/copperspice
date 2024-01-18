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

#ifndef QDECLARATIVEIMPLICITSIZEITEM_P_H
#define QDECLARATIVEIMPLICITSIZEITEM_P_H

#include <qdeclarativepainteditem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeImplicitSizeItemPrivate;
class QDeclarativeImplicitSizePaintedItemPrivate;

class QDeclarativeImplicitSizeItem : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeImplicitSizeItem)

   DECL_CS_PROPERTY_READ(implicitWidth, implicitWidth)
   DECL_CS_PROPERTY_NOTIFY(implicitWidth, implicitWidthChanged)
   DECL_CS_PROPERTY_REVISION(implicitWidth, 1)
   DECL_CS_PROPERTY_READ(implicitHeight, implicitHeight)
   DECL_CS_PROPERTY_NOTIFY(implicitHeight, implicitHeightChanged)
   DECL_CS_PROPERTY_REVISION(implicitHeight, 1)

 public:
   QDeclarativeImplicitSizeItem(QDeclarativeItem *parent = 0);

   DECL_CS_SIGNAL_1(Public, void implicitWidthChanged())
   DECL_CS_SIGNAL_2(implicitWidthChanged)
   DECL_CS_REVISION(implicitWidthChanged, 1)

   DECL_CS_SIGNAL_1(Public, void implicitHeightChanged())
   DECL_CS_SIGNAL_2(implicitHeightChanged)
   DECL_CS_REVISION(implicitHeightChanged, 1)

 protected:
   QDeclarativeImplicitSizeItem(QDeclarativeImplicitSizeItemPrivate &dd, QDeclarativeItem *parent);

 private:
   Q_DISABLE_COPY(QDeclarativeImplicitSizeItem)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeImplicitSizeItem)
};


class QDeclarativeImplicitSizePaintedItem : public QDeclarativePaintedItem
{
   DECL_CS_OBJECT(QDeclarativeImplicitSizePaintedItem)
   DECL_CS_PROPERTY_READ(implicitWidth, implicitWidth)
   DECL_CS_PROPERTY_NOTIFY(implicitWidth, implicitWidthChanged)
   DECL_CS_PROPERTY_REVISION(implicitWidth, 1)
   DECL_CS_PROPERTY_READ(implicitHeight, implicitHeight)
   DECL_CS_PROPERTY_NOTIFY(implicitHeight, implicitHeightChanged)
   DECL_CS_PROPERTY_REVISION(implicitHeight, 1)

 public:
   QDeclarativeImplicitSizePaintedItem(QDeclarativeItem *parent = 0);

   DECL_CS_SIGNAL_1(Public, void implicitWidthChanged())
   DECL_CS_SIGNAL_2(implicitWidthChanged)
   DECL_CS_REVISION(implicitWidthChanged, 1)

   DECL_CS_SIGNAL_1(Public, void implicitHeightChanged())
   DECL_CS_SIGNAL_2(implicitHeightChanged)
   DECL_CS_REVISION(implicitHeightChanged, 1)

 protected:
   QDeclarativeImplicitSizePaintedItem(QDeclarativeImplicitSizePaintedItemPrivate &dd, QDeclarativeItem *parent);
   virtual void drawContents(QPainter *, const QRect &) {};

 private:
   Q_DISABLE_COPY(QDeclarativeImplicitSizePaintedItem)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeImplicitSizePaintedItem)
};

QT_END_NAMESPACE

#endif // QDECLARATIVEIMPLICITSIZEITEM_H
