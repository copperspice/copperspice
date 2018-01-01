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

#include <qgesturerecognizer.h>
#include <qgesture_p.h>
#include <qgesturemanager_p.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

QGestureRecognizer::QGestureRecognizer()
{
}

/*!
    Destroys the gesture recognizer.
*/
QGestureRecognizer::~QGestureRecognizer()
{
}

/*!
    This function is called by Qt to create a new QGesture object for the
    given \a target (QWidget or QGraphicsObject).

    Reimplement this function to create a custom QGesture-derived gesture
    object if necessary.

    The application takes ownership of the created gesture object.
*/
QGesture *QGestureRecognizer::create(QObject *target)
{
   Q_UNUSED(target);
   return new QGesture;
}

/*!
    This function is called by the framework to reset a given \a gesture.

    Reimplement this function to implement additional requirements for custom QGesture
    objects. This may be necessary if you implement a custom QGesture whose properties
    need special handling when the gesture is reset.
*/
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

/*!
    \fn QGestureRecognizer::recognize(QGesture *gesture, QObject *watched, QEvent *event)

    Handles the given \a event for the \a watched object, updating the state of the \a gesture
    object as required, and returns a suitable result for the current recognition step.

    This function is called by the framework to allow the recognizer to filter input events
    dispatched to QWidget or QGraphicsObject instances that it is monitoring.

    The result reflects how much of the gesture has been recognized. The state of the
    \a gesture object is set depending on the result.

    \sa QGestureRecognizer::Result
*/

/*!
    Registers the given \a recognizer in the gesture framework and returns a gesture ID
    for it.

    The application takes ownership of the \a recognizer and returns the gesture type
    ID associated with it. For gesture recognizers which handle custom QGesture
    objects (i.e., those which return Qt::CustomGesture in a QGesture::gestureType()
    function) the return value is a generated gesture ID with the Qt::CustomGesture
    flag set.

    \sa unregisterRecognizer(), QGestureRecognizer::create(), QGesture
*/
Qt::GestureType QGestureRecognizer::registerRecognizer(QGestureRecognizer *recognizer)
{
   return QGestureManager::instance()->registerGestureRecognizer(recognizer);
}

/*!
    Unregisters all gesture recognizers of the specified \a type.

    \sa registerRecognizer()
*/
void QGestureRecognizer::unregisterRecognizer(Qt::GestureType type)
{
   QGestureManager::instance()->unregisterGestureRecognizer(type);
}

QT_END_NAMESPACE

#endif // QT_NO_GESTURES
