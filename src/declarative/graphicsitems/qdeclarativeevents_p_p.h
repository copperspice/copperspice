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

#ifndef QDECLARATIVEEVENTS_P_P_H
#define QDECLARATIVEEVENTS_P_P_H

#include <qdeclarative.h>
#include <QtCore/qobject.h>
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

class QDeclarativeKeyEvent : public QObject
{
   DECL_CS_OBJECT(QDeclarativeKeyEvent)
   DECL_CS_PROPERTY_READ(key, key)
   DECL_CS_PROPERTY_READ(text, text)
   DECL_CS_PROPERTY_READ(modifiers, modifiers)
   DECL_CS_PROPERTY_READ(isAutoRepeat, isAutoRepeat)
   DECL_CS_PROPERTY_READ(count, count)
   DECL_CS_PROPERTY_READ(accepted, isAccepted)
   DECL_CS_PROPERTY_WRITE(accepted, setAccepted)

 public:
   QDeclarativeKeyEvent(QEvent::Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text = QString(),
                        bool autorep = false, ushort count = 1)
      : event(type, key, modifiers, text, autorep, count) {
      event.setAccepted(false);
   }
   QDeclarativeKeyEvent(const QKeyEvent &ke)
      : event(ke) {
      event.setAccepted(false);
   }

   int key() const {
      return event.key();
   }
   QString text() const {
      return event.text();
   }
   int modifiers() const {
      return event.modifiers();
   }
   bool isAutoRepeat() const {
      return event.isAutoRepeat();
   }
   int count() const {
      return event.count();
   }

   bool isAccepted() {
      return event.isAccepted();
   }
   void setAccepted(bool accepted) {
      event.setAccepted(accepted);
   }

 private:
   QKeyEvent event;
};

class QDeclarativeMouseEvent : public QObject
{
   DECL_CS_OBJECT(QDeclarativeMouseEvent)
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_READ(button, button)
   DECL_CS_PROPERTY_READ(buttons, buttons)
   DECL_CS_PROPERTY_READ(modifiers, modifiers)
   DECL_CS_PROPERTY_READ(wasHeld, wasHeld)
   DECL_CS_PROPERTY_READ(isClick, isClick)
   DECL_CS_PROPERTY_READ(accepted, isAccepted)
   DECL_CS_PROPERTY_WRITE(accepted, setAccepted)

 public:
   QDeclarativeMouseEvent(int x, int y, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers
                          , bool isClick = false, bool wasHeld = false)
      : _x(x), _y(y), _button(button), _buttons(buttons), _modifiers(modifiers)
      , _wasHeld(wasHeld), _isClick(isClick), _accepted(true) {}

   int x() const {
      return _x;
   }
   int y() const {
      return _y;
   }
   int button() const {
      return _button;
   }
   int buttons() const {
      return _buttons;
   }
   int modifiers() const {
      return _modifiers;
   }
   bool wasHeld() const {
      return _wasHeld;
   }
   bool isClick() const {
      return _isClick;
   }

   // only for internal usage
   void setX(int x) {
      _x = x;
   }
   void setY(int y) {
      _y = y;
   }

   bool isAccepted() {
      return _accepted;
   }
   void setAccepted(bool accepted) {
      _accepted = accepted;
   }

 private:
   int _x;
   int _y;
   Qt::MouseButton _button;
   Qt::MouseButtons _buttons;
   Qt::KeyboardModifiers _modifiers;
   bool _wasHeld;
   bool _isClick;
   bool _accepted;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeKeyEvent)
QML_DECLARE_TYPE(QDeclarativeMouseEvent)

#endif // QDECLARATIVEEVENTS_P_H
