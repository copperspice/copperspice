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

#include "private/qdeclarativetranslate_p.h"
#include <private/qgraphicstransform_p.h>
#include <QDebug>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTranslatePrivate : public QGraphicsTransformPrivate
{
 public:
   QDeclarativeTranslatePrivate()
      : x(0), y(0) {}
   qreal x;
   qreal y;
};

/*!
    Constructs an empty QDeclarativeTranslate object with the given \a parent.
*/
QDeclarativeTranslate::QDeclarativeTranslate(QObject *parent)
   : QGraphicsTransform(*new QDeclarativeTranslatePrivate, parent)
{
}

/*!
    Destroys the graphics scale.
*/
QDeclarativeTranslate::~QDeclarativeTranslate()
{
}

/*!
    \property QDeclarativeTranslate::x
    \brief the horizontal translation.

    The translation can be any real number; the default value is 0.0.

    \sa y
*/
qreal QDeclarativeTranslate::x() const
{
   Q_D(const QDeclarativeTranslate);
   return d->x;
}
void QDeclarativeTranslate::setX(qreal x)
{
   Q_D(QDeclarativeTranslate);
   if (d->x == x) {
      return;
   }
   d->x = x;
   update();
   emit xChanged();
}

/*!
    \property QDeclarativeTranslate::y
    \brief the vertical translation.

    The translation can be any real number; the default value is 0.0.

    \sa x
*/
qreal QDeclarativeTranslate::y() const
{
   Q_D(const QDeclarativeTranslate);
   return d->y;
}
void QDeclarativeTranslate::setY(qreal y)
{
   Q_D(QDeclarativeTranslate);
   if (d->y == y) {
      return;
   }
   d->y = y;
   update();
   emit yChanged();
}

void QDeclarativeTranslate::applyTo(QMatrix4x4 *matrix) const
{
   Q_D(const QDeclarativeTranslate);
   matrix->translate(d->x, d->y, 0);
}

QT_END_NAMESPACE
