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

#ifndef QMULTITOUCH_MAC_P_H
#define QMULTITOUCH_MAC_P_H

#import <Cocoa/Cocoa.h>

#include <qevent.h>
#include <qhash.h>
#include <QtCore>

QT_BEGIN_NAMESPACE

class QCocoaTouch
{
 public:
   static QList<QTouchEvent::TouchPoint> getCurrentTouchPointList(NSEvent *event, bool acceptSingleTouch);
   static void setMouseInDraggingState(bool inDraggingState);

 private:
   static QHash<qint64, QCocoaTouch *> _currentTouches;
   static QPointF _screenReferencePos;
   static QPointF _trackpadReferencePos;
   static int _idAssignmentCount;
   static int _touchCount;
   static bool _updateInternalStateOnly;

   QTouchEvent::TouchPoint _touchPoint;
   qint64 _identity;

   QCocoaTouch(NSTouch *nstouch);
   ~QCocoaTouch();

   void updateTouchData(NSTouch *nstouch, NSTouchPhase phase);
   static QCocoaTouch *findQCocoaTouch(NSTouch *nstouch);
   static Qt::TouchPointState toTouchPointState(NSTouchPhase nsState);
};


QT_END_NAMESPACE

#endif // QMULTITOUCH_MAC_P_H

