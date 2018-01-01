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

#ifndef QMOUSEEVENTTRANSITION_H
#define QMOUSEEVENTTRANSITION_H

#include <QtCore/qeventtransition.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATEMACHINE

class QMouseEventTransitionPrivate;
class QPainterPath;

class Q_GUI_EXPORT QMouseEventTransition : public QEventTransition
{
   GUI_CS_OBJECT(QMouseEventTransition)

   GUI_CS_PROPERTY_READ(button, button)
   GUI_CS_PROPERTY_WRITE(button, setButton)
   GUI_CS_PROPERTY_READ(modifierMask, modifierMask)
   GUI_CS_PROPERTY_WRITE(modifierMask, setModifierMask)

 public:
   QMouseEventTransition(QState *sourceState = 0);
   QMouseEventTransition(QObject *object, QEvent::Type type,
                         Qt::MouseButton button, QState *sourceState = 0);
   ~QMouseEventTransition();

   Qt::MouseButton button() const;
   void setButton(Qt::MouseButton button);

   Qt::KeyboardModifiers modifierMask() const;
   void setModifierMask(Qt::KeyboardModifiers modifiers);

   QPainterPath hitTestPath() const;
   void setHitTestPath(const QPainterPath &path);

 protected:
   void onTransition(QEvent *event) override;
   bool eventTest(QEvent *event) override;

 private:
   Q_DISABLE_COPY(QMouseEventTransition)
   Q_DECLARE_PRIVATE(QMouseEventTransition)
};

#endif //QT_NO_STATEMACHINE

QT_END_NAMESPACE

#endif
