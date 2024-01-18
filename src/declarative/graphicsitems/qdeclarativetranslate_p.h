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

#ifndef QDECLARATIVETRANSLATE_P_H
#define QDECLARATIVETRANSLATE_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTranslatePrivate;

class QDeclarativeTranslate : public QGraphicsTransform
{
   DECL_CS_OBJECT(QDeclarativeTranslate)

   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_NOTIFY(x, xChanged)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_NOTIFY(y, yChanged)

 public:
   QDeclarativeTranslate(QObject *parent = nullptr);
   ~QDeclarativeTranslate();

   qreal x() const;
   void setX(qreal);

   qreal y() const;
   void setY(qreal);

   void applyTo(QMatrix4x4 *matrix) const;

   DECL_CS_SIGNAL_1(Public, void xChanged())
   DECL_CS_SIGNAL_2(xChanged)
   DECL_CS_SIGNAL_1(Public, void yChanged())
   DECL_CS_SIGNAL_2(yChanged)

 private:
   Q_DECLARE_PRIVATE(QDeclarativeTranslate)
   Q_DISABLE_COPY(QDeclarativeTranslate)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTranslate)


#endif
