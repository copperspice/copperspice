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

#ifndef QBASICMOUSEEVENTTRANSITION_P_H
#define QBASICMOUSEEVENTTRANSITION_P_H

#include <QtCore/qabstracttransition.h>

#ifndef QT_NO_STATEMACHINE

#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

class QPainterPath;
class QBasicMouseEventTransitionPrivate;

class QBasicMouseEventTransition : public QAbstractTransition
{
   GUI_CS_OBJECT(QBasicMouseEventTransition)

 public:
   QBasicMouseEventTransition(QState *sourceState = 0);
   QBasicMouseEventTransition(QEvent::Type type, Qt::MouseButton button, QState *sourceState = 0);
   ~QBasicMouseEventTransition();

   QEvent::Type eventType() const;
   void setEventType(QEvent::Type type);

   Qt::MouseButton button() const;
   void setButton(Qt::MouseButton button);

   Qt::KeyboardModifiers modifierMask() const;
   void setModifierMask(Qt::KeyboardModifiers modifiers);

   QPainterPath hitTestPath() const;
   void setHitTestPath(const QPainterPath &path);

 protected:
   bool eventTest(QEvent *event) override;
   void onTransition(QEvent *) override;

 private:
   Q_DISABLE_COPY(QBasicMouseEventTransition)
   Q_DECLARE_PRIVATE(QBasicMouseEventTransition)
};

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE

#endif
