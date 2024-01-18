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

#ifndef QDECLARATIVEMouseArea_P_P_H
#define QDECLARATIVEMouseArea_P_P_H

#include <qdeclarativeitem_p.h>
#include <qdatetime.h>
#include <qbasictimer.h>
#include <qgraphicssceneevent.h>

QT_BEGIN_NAMESPACE

class QDeclarativeMouseAreaPrivate : public QDeclarativeItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeMouseArea)

 public:
   QDeclarativeMouseAreaPrivate()
      : absorb(true), hovered(false), pressed(false), longPress(false),
        moved(false), stealMouse(false), doubleClick(false), preventStealing(false), drag(0) {
   }

   ~QDeclarativeMouseAreaPrivate();

   void init() {
      Q_Q(QDeclarativeMouseArea);
      q->setAcceptedMouseButtons(Qt::LeftButton);
      q->setFiltersChildEvents(true);
   }

   void saveEvent(QGraphicsSceneMouseEvent *event) {
      lastPos = event->pos();
      lastScenePos = event->scenePos();
      lastButton = event->button();
      lastButtons = event->buttons();
      lastModifiers = event->modifiers();
   }

   bool isPressAndHoldConnected() {
      Q_Q(QDeclarativeMouseArea);
      static int idx = QObjectPrivate::get(q)->signalIndex("pressAndHold(QDeclarativeMouseEvent*)");
      return QObjectPrivate::get(q)->isSignalConnected(idx);
   }

   bool isDoubleClickConnected() {
      Q_Q(QDeclarativeMouseArea);
      static int idx = QObjectPrivate::get(q)->signalIndex("doubleClicked(QDeclarativeMouseEvent*)");
      return QObjectPrivate::get(q)->isSignalConnected(idx);
   }

   bool absorb : 1;
   bool hovered : 1;
   bool pressed : 1;
   bool longPress : 1;
   bool moved : 1;
   bool stealMouse : 1;
   bool doubleClick : 1;
   bool preventStealing : 1;
   QDeclarativeDrag *drag;
   QPointF startScene;
   qreal startX;
   qreal startY;
   QPointF lastPos;
   QDeclarativeNullableValue<QPointF> lastScenePos;
   Qt::MouseButton lastButton;
   Qt::MouseButtons lastButtons;
   Qt::KeyboardModifiers lastModifiers;
   QBasicTimer pressAndHoldTimer;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEMOUSEREGION_P_H
