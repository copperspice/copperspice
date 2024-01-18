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

#ifndef QDECLARATIVEFLIPABLE_P_H
#define QDECLARATIVEFLIPABLE_P_H

#include <qdeclarativeitem.h>
#include <QtCore/QObject>
#include <QtGui/QTransform>
#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

class QDeclarativeFlipablePrivate;

class QDeclarativeFlipable : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeFlipable)

   DECL_CS_ENUM(Side)
   DECL_CS_PROPERTY_READ(*front, front)
   DECL_CS_PROPERTY_WRITE(*front, setFront)
   DECL_CS_PROPERTY_NOTIFY(*front, frontChanged)
   DECL_CS_PROPERTY_READ(*back, back)
   DECL_CS_PROPERTY_WRITE(*back, setBack)
   DECL_CS_PROPERTY_NOTIFY(*back, backChanged)
   DECL_CS_PROPERTY_READ(side, side)
   DECL_CS_PROPERTY_NOTIFY(side, sideChanged)
   //### flipAxis
   //### flipRotation

 public:
   QDeclarativeFlipable(QDeclarativeItem *parent = 0);
   ~QDeclarativeFlipable();

   QGraphicsObject *front();
   void setFront(QGraphicsObject *);

   QGraphicsObject *back();
   void setBack(QGraphicsObject *);

   enum Side { Front, Back };
   Side side() const;

   DECL_CS_SIGNAL_1(Public, void frontChanged())
   DECL_CS_SIGNAL_2(frontChanged)
   DECL_CS_SIGNAL_1(Public, void backChanged())
   DECL_CS_SIGNAL_2(backChanged)
   DECL_CS_SIGNAL_1(Public, void sideChanged())
   DECL_CS_SIGNAL_2(sideChanged)

 private :
   DECL_CS_SLOT_1(Private, void retransformBack())
   DECL_CS_SLOT_2(retransformBack)

   Q_DISABLE_COPY(QDeclarativeFlipable)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeFlipable)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFlipable)


#endif // QDECLARATIVEFLIPABLE_H
