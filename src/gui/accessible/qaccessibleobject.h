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

#ifndef QACCESSIBLEOBJECT_H
#define QACCESSIBLEOBJECT_H

#include <qaccessible.h>

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleObjectPrivate;
class QObject;

class Q_GUI_EXPORT QAccessibleObject : public QAccessibleInterface
{
 public:
   explicit QAccessibleObject(QObject *object);

   QAccessibleObject(const QAccessibleObject &) = delete;
   QAccessibleObject &operator=(const QAccessibleObject &) = delete;

   bool isValid() const override;
   QObject *object() const override;

   // properties
   QRect rect() const override;
   void setText(QAccessible::Text text, const QString &str) override;
   QAccessibleInterface *childAt(int x, int y) const override;

 protected:
   virtual ~QAccessibleObject();

 private:
   QAccessibleObjectPrivate *d;
};

class Q_GUI_EXPORT QAccessibleApplication : public QAccessibleObject
{
 public:
   QAccessibleApplication();

   QWindow *window() const override;

   // relations
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *) const override;
   QAccessibleInterface *focusChild() const override;

   // navigation
   QAccessibleInterface *parent() const override;
   QAccessibleInterface *child(int index) const override;

   // properties and state
   QString text(QAccessible::Text t) const override;
   QAccessible::Role role() const override;
   QAccessible::State state() const override;
};

#endif

#endif
