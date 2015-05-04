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
   CS_OBJECT(QDeclarativeFlipable)

   CS_ENUM(Side)
   CS_PROPERTY_READ(*front, front)
   CS_PROPERTY_WRITE(*front, setFront)
   CS_PROPERTY_NOTIFY(*front, frontChanged)
   CS_PROPERTY_READ(*back, back)
   CS_PROPERTY_WRITE(*back, setBack)
   CS_PROPERTY_NOTIFY(*back, backChanged)
   CS_PROPERTY_READ(side, side)
   CS_PROPERTY_NOTIFY(side, sideChanged)
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

   CS_SIGNAL_1(Public, void frontChanged())
   CS_SIGNAL_2(frontChanged)
   CS_SIGNAL_1(Public, void backChanged())
   CS_SIGNAL_2(backChanged)
   CS_SIGNAL_1(Public, void sideChanged())
   CS_SIGNAL_2(sideChanged)

 private :
   CS_SLOT_1(Private, void retransformBack())
   CS_SLOT_2(retransformBack)

   Q_DISABLE_COPY(QDeclarativeFlipable)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeFlipable)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFlipable)


#endif // QDECLARATIVEFLIPABLE_H
