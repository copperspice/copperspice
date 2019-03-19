/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qmultitouch_mac_p.h>
#include <qcursor.h>

QT_BEGIN_NAMESPACE

QHash<qint64, QCocoaTouch *> QCocoaTouch::_currentTouches;
QPointF QCocoaTouch::_screenReferencePos;
QPointF QCocoaTouch::_trackpadReferencePos;
int QCocoaTouch::_idAssignmentCount = 0;
int QCocoaTouch::_touchCount = 0;
bool QCocoaTouch::_updateInternalStateOnly = true;

QCocoaTouch::QCocoaTouch(NSTouch *nstouch)
{
   if (_currentTouches.size() == 0) {
      _idAssignmentCount = 0;
   }

   _touchPoint.setId(_idAssignmentCount++);
   _touchPoint.setPressure(1.0);
   _identity = qint64([nstouch identity]);
   _currentTouches.insert(_identity, this);
   updateTouchData(nstouch, NSTouchPhaseBegan);
}

QCocoaTouch::~QCocoaTouch()
{
   _currentTouches.remove(_identity);
}

void QCocoaTouch::updateTouchData(NSTouch *nstouch, NSTouchPhase phase)
{
   if (_touchCount == 1) {
      _touchPoint.setState(toTouchPointState(phase) | Qt::TouchPointPrimary);
   } else {
      _touchPoint.setState(toTouchPointState(phase));
   }

   // From the normalized position on the trackpad, calculate
   // where on screen the touchpoint should be according to the
   // reference position:
   NSPoint npos = [nstouch normalizedPosition];
   QPointF qnpos = QPointF(npos.x, 1 - npos.y);
   _touchPoint.setNormalizedPos(qnpos);

   if (_touchPoint.id() == 0 && phase == NSTouchPhaseBegan) {
      _trackpadReferencePos = qnpos;
      _screenReferencePos = QCursor::pos();
   }

   NSSize dsize = [nstouch deviceSize];
   float ppiX = (qnpos.x() - _trackpadReferencePos.x()) * dsize.width;
   float ppiY = (qnpos.y() - _trackpadReferencePos.y()) * dsize.height;
   QPointF relativePos = _trackpadReferencePos - QPointF(ppiX, ppiY);
   _touchPoint.setScreenPos(_screenReferencePos - relativePos);
}

QCocoaTouch *QCocoaTouch::findQCocoaTouch(NSTouch *nstouch)
{
   qint64 identity = qint64([nstouch identity]);
   if (_currentTouches.contains(identity)) {
      return _currentTouches.value(identity);
   }
   return 0;
}

Qt::TouchPointState QCocoaTouch::toTouchPointState(NSTouchPhase nsState)
{
   Qt::TouchPointState qtState = Qt::TouchPointReleased;
   switch (nsState) {
      case NSTouchPhaseBegan:
         qtState = Qt::TouchPointPressed;
         break;
      case NSTouchPhaseMoved:
         qtState = Qt::TouchPointMoved;
         break;
      case NSTouchPhaseStationary:
         qtState = Qt::TouchPointStationary;
         break;
      case NSTouchPhaseEnded:
      case NSTouchPhaseCancelled:
         qtState = Qt::TouchPointReleased;
         break;
      default:
         break;
   }
   return qtState;
}

QList<QTouchEvent::TouchPoint>
QCocoaTouch::getCurrentTouchPointList(NSEvent *event, bool acceptSingleTouch)
{
   QMap<int, QTouchEvent::TouchPoint> touchPoints;
   NSSet *ended = [event touchesMatchingPhase: NSTouchPhaseEnded | NSTouchPhaseCancelled inView: nil];
   NSSet *active = [event
                    touchesMatchingPhase: NSTouchPhaseBegan | NSTouchPhaseMoved | NSTouchPhaseStationary
                    inView: nil];
   _touchCount = [active count];

   // First: remove touches that were ended by the user. If we are
   // currently not accepting single touches, a corresponding 'begin'
   // has never been send to the app for these events.
   // So should therefore not send the following removes either.

   for (int i = 0; i < int([ended count]); ++i) {
      NSTouch *touch = [[ended allObjects] objectAtIndex: i];
      QCocoaTouch *qcocoaTouch = findQCocoaTouch(touch);
      if (qcocoaTouch) {
         qcocoaTouch->updateTouchData(touch, [touch phase]);
         if (!_updateInternalStateOnly) {
            touchPoints.insert(qcocoaTouch->_touchPoint.id(), qcocoaTouch->_touchPoint);
         }
         delete qcocoaTouch;
      }
   }

   bool wasUpdateInternalStateOnly = _updateInternalStateOnly;
   _updateInternalStateOnly = !acceptSingleTouch && _touchCount < 2;

   // Next: update, or create, existing touches.
   // We always keep track of all touch points, even
   // when not accepting single touches.

   for (int i = 0; i < int([active count]); ++i) {
      NSTouch *touch = [[active allObjects] objectAtIndex: i];
      QCocoaTouch *qcocoaTouch = findQCocoaTouch(touch);
      if (!qcocoaTouch) {
         qcocoaTouch = new QCocoaTouch(touch);
      } else {
         qcocoaTouch->updateTouchData(touch, wasUpdateInternalStateOnly ? NSTouchPhaseBegan : [touch phase]);
      }
      if (!_updateInternalStateOnly) {
         touchPoints.insert(qcocoaTouch->_touchPoint.id(), qcocoaTouch->_touchPoint);
      }
   }

   // Next: sadly, we need to check that our touch hash is in
   // sync with cocoa. This is typically not the case after a system
   // gesture happend (like a four-finger-swipe to show expose).

   if (_touchCount != _currentTouches.size()) {
      // Remove all instances, and basically start from scratch:
      touchPoints.clear();

      for (QCocoaTouch * qcocoaTouch : _currentTouches.values()) {
         if (!_updateInternalStateOnly) {
            qcocoaTouch->_touchPoint.setState(Qt::TouchPointReleased);
            touchPoints.insert(qcocoaTouch->_touchPoint.id(), qcocoaTouch->_touchPoint);
         }

         delete qcocoaTouch;
      }

      _currentTouches.clear();
      _updateInternalStateOnly = !acceptSingleTouch;
      return touchPoints.values();
   }

   // Finally: If this call _started_ to reject single
   // touches, we need to fake a relase for the remaining
   // touch now (and refake a begin for it later, if needed).

   if (_updateInternalStateOnly && !wasUpdateInternalStateOnly && !_currentTouches.isEmpty()) {
      QCocoaTouch *qcocoaTouch = _currentTouches.values().first();
      qcocoaTouch->_touchPoint.setState(Qt::TouchPointReleased);
      touchPoints.insert(qcocoaTouch->_touchPoint.id(), qcocoaTouch->_touchPoint);
      // Since this last touch also will end up beeing the first
      // touch (if the user adds a second finger without lifting
      // the first), we promote it to be the primary touch:
      qcocoaTouch->_touchPoint.setId(0);
      _idAssignmentCount = 1;
   }

   return touchPoints.values();
}

QT_END_NAMESPACE
