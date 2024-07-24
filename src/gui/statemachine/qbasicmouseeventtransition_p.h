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

#ifndef QBASICMOUSEEVENTTRANSITION_P_H
#define QBASICMOUSEEVENTTRANSITION_P_H

#include <qabstracttransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qevent.h>

class QBasicMouseEventTransitionPrivate;
class QPainterPath;

class QBasicMouseEventTransition : public QAbstractTransition
{
   GUI_CS_OBJECT(QBasicMouseEventTransition)

 public:
   QBasicMouseEventTransition(QState *sourceState = nullptr);
   QBasicMouseEventTransition(QEvent::Type type, Qt::MouseButton button, QState *sourceState = nullptr);

   QBasicMouseEventTransition(const QBasicMouseEventTransition &) = delete;
   QBasicMouseEventTransition &operator=(const QBasicMouseEventTransition &) = delete;

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
   Q_DECLARE_PRIVATE(QBasicMouseEventTransition)
};

#endif //QT_NO_STATEMACHINE

#endif
