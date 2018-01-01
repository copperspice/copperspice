/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSTANDARDGESTURES_P_H
#define QSTANDARDGESTURES_P_H

#include <qgesturerecognizer.h>
#include <qgesture_p.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

class QPanGestureRecognizer : public QGestureRecognizer
{
 public:
   QPanGestureRecognizer();

   QGesture *create(QObject *target) override;
   QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
   void reset(QGesture *state) override;
};

class QPinchGestureRecognizer : public QGestureRecognizer
{
 public:
   QPinchGestureRecognizer();

   QGesture *create(QObject *target) override;
   QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
   void reset(QGesture *state) override;
};

class QSwipeGestureRecognizer : public QGestureRecognizer
{
 public:
   QSwipeGestureRecognizer();

   QGesture *create(QObject *target) override;
   QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
   void reset(QGesture *state) override;
};

class QTapGestureRecognizer : public QGestureRecognizer
{
 public:
   QTapGestureRecognizer();

   QGesture *create(QObject *target) override;
   QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
   void reset(QGesture *state) override;
};

class QTapAndHoldGestureRecognizer : public QGestureRecognizer
{
 public:
   QTapAndHoldGestureRecognizer();

   QGesture *create(QObject *target) override;
   QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
   void reset(QGesture *state) override;
};

QT_END_NAMESPACE

#endif // QT_NO_GESTURES

#endif // QSTANDARDGESTURES_P_H
