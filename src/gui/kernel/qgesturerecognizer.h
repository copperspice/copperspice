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

#ifndef QGESTURERECOGNIZER_H
#define QGESTURERECOGNIZER_H

#include <qglobal.h>
#include <qnamespace.h>

#ifndef QT_NO_GESTURES

class QObject;
class QEvent;
class QGesture;

class Q_GUI_EXPORT QGestureRecognizer
{
 public:
   enum ResultFlag {
      Ignore           = 0x0001,
      MayBeGesture     = 0x0002,
      TriggerGesture   = 0x0004,
      FinishGesture    = 0x0008,
      CancelGesture    = 0x0010,
      ResultState_Mask = 0x00ff,

      ConsumeEventHint           = 0x0100,
      // StoreEventHint          = 0x0200,
      // ReplayStoredEventsHint  = 0x0400,
      // DiscardStoredEventsHint = 0x0800,

      ResultHint_Mask = 0xff00
   };
   using Result = QFlags<ResultFlag>;

   QGestureRecognizer();
   virtual ~QGestureRecognizer();

   virtual QGesture *create(QObject *target);
   virtual Result recognize(QGesture *gesture, QObject *watched, QEvent *event) = 0;
   virtual void reset(QGesture *gesture);

   static Qt::GestureType registerRecognizer(QGestureRecognizer *recognizer);
   static void unregisterRecognizer(Qt::GestureType type);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGestureRecognizer::Result)

#endif // QT_NO_GESTURES

#endif // QGESTURERECOGNIZER_H
