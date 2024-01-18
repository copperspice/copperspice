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

#ifndef QSTANDARDGESTURES_P_H
#define QSTANDARDGESTURES_P_H

#include <qgesturerecognizer.h>
#include <qgesture_p.h>

#ifndef QT_NO_GESTURES



class QPanGestureRecognizer : public QGestureRecognizer
{
 public:
   explicit QPanGestureRecognizer(int pointCount = 2) : m_pointCount(pointCount)
   {}

   QGesture *create(QObject *target) override;
   QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event) override;
   void reset(QGesture *state) override;
 private:
   const int m_pointCount;
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

#endif // QT_NO_GESTURES
#endif // QSTANDARDGESTURES_P_H
