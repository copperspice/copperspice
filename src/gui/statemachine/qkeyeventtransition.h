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

#ifndef QKEYEVENTTRANSITION_H
#define QKEYEVENTTRANSITION_H

#include <qeventtransition.h>

#ifndef QT_NO_STATEMACHINE

class QKeyEventTransitionPrivate;

class Q_GUI_EXPORT QKeyEventTransition : public QEventTransition
{
   GUI_CS_OBJECT(QKeyEventTransition)

   GUI_CS_PROPERTY_READ(key, key)
   GUI_CS_PROPERTY_WRITE(key, setKey)
   GUI_CS_PROPERTY_READ(modifierMask, modifierMask)
   GUI_CS_PROPERTY_WRITE(modifierMask, setModifierMask)

 public:
   QKeyEventTransition(QState *sourceState = nullptr);
   QKeyEventTransition(QObject *object, QEvent::Type type, int key, QState *sourceState = nullptr);

   QKeyEventTransition(const QKeyEventTransition &) = delete;
   QKeyEventTransition &operator=(const QKeyEventTransition &) = delete;

   ~QKeyEventTransition();

   int key() const;
   void setKey(int key);

   Qt::KeyboardModifiers modifierMask() const;
   void setModifierMask(Qt::KeyboardModifiers modifiers);

 protected:
   void onTransition(QEvent *event) override;
   bool eventTest(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QKeyEventTransition)
};

#endif //QT_NO_STATEMACHINE

#endif
