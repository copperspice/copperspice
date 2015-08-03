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

#ifndef QDECLARATIVETRANSLATE_P_H
#define QDECLARATIVETRANSLATE_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTranslatePrivate;

class QDeclarativeTranslate : public QGraphicsTransform
{
   DECL_CS_OBJECT(QDeclarativeTranslate)

   CS_PROPERTY_READ(x, x)
   CS_PROPERTY_WRITE(x, setX)
   CS_PROPERTY_NOTIFY(x, xChanged)
   CS_PROPERTY_READ(y, y)
   CS_PROPERTY_WRITE(y, setY)
   CS_PROPERTY_NOTIFY(y, yChanged)

 public:
   QDeclarativeTranslate(QObject *parent = 0);
   ~QDeclarativeTranslate();

   qreal x() const;
   void setX(qreal);

   qreal y() const;
   void setY(qreal);

   void applyTo(QMatrix4x4 *matrix) const;

   CS_SIGNAL_1(Public, void xChanged())
   CS_SIGNAL_2(xChanged)
   CS_SIGNAL_1(Public, void yChanged())
   CS_SIGNAL_2(yChanged)

 private:
   Q_DECLARE_PRIVATE(QDeclarativeTranslate)
   Q_DISABLE_COPY(QDeclarativeTranslate)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTranslate)


#endif
