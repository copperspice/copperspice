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

#include <qgesturerecognizer.h>
#include <qgesture_p.h>
#include <qgesturemanager_p.h>

#ifndef QT_NO_GESTURES

QGestureRecognizer::QGestureRecognizer()
{
}

QGestureRecognizer::~QGestureRecognizer()
{
}

QGesture *QGestureRecognizer::create(QObject *target)
{
   (void) target;
   return new QGesture;
}

void QGestureRecognizer::reset(QGesture *gesture)
{
   if (gesture) {
      QGesturePrivate *d = gesture->d_func();
      d->state = Qt::NoGesture;
      d->hotSpot = QPointF();
      d->sceneHotSpot = QPointF();
      d->isHotSpotSet = false;
   }
}

Qt::GestureType QGestureRecognizer::registerRecognizer(QGestureRecognizer *recognizer)
{
   return QGestureManager::instance()->registerGestureRecognizer(recognizer);
}

void QGestureRecognizer::unregisterRecognizer(Qt::GestureType type)
{
   QGestureManager::instance()->unregisterGestureRecognizer(type);
}



#endif // QT_NO_GESTURES
