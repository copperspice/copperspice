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

#ifndef QACCESSIBLEOBJECT_H
#define QACCESSIBLEOBJECT_H

#include <QtGui/qaccessible.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleObjectPrivate;
class QObject;

class Q_GUI_EXPORT QAccessibleObject : public QAccessibleInterface
{
 public:
   explicit QAccessibleObject(QObject *object);

   bool isValid() const override;
   QObject *object() const override;

   // properties
   QRect rect(int child) const override;
   void setText(Text t, int child, const QString &text) override;

   // actions
   int userActionCount(int child) const override;
   bool doAction(int action, int child, const QVariantList &params) override;
   QString actionText(int action, Text t, int child) const override;

 protected:
   virtual ~QAccessibleObject();

 private:
   friend class QAccessibleObjectEx;
   QAccessibleObjectPrivate *d;
   Q_DISABLE_COPY(QAccessibleObject)
};

class Q_GUI_EXPORT QAccessibleObjectEx : public QAccessibleInterfaceEx
{
 public:
   explicit QAccessibleObjectEx(QObject *object);

   bool isValid() const override;
   QObject *object() const override;

   // properties
   QRect rect(int child) const override;
   void setText(Text t, int child, const QString &text) override; 

   // actions
   int userActionCount(int child) const override;
   bool doAction(int action, int child, const QVariantList &params) override;
   QString actionText(int action, Text t, int child) const override;

 protected:
   virtual ~QAccessibleObjectEx();

 private:
   QAccessibleObjectPrivate *d;
   Q_DISABLE_COPY(QAccessibleObjectEx)
};

class Q_GUI_EXPORT QAccessibleApplication : public QAccessibleObject
{
 public:
   QAccessibleApplication();

   // relations
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *) const override;
   Relation relationTo(int, const QAccessibleInterface *, int) const override;

   // navigation
   int childAt(int x, int y) const override;
   int navigate(RelationFlag, int, QAccessibleInterface **) const override;

   // properties and state
   QString text(Text t, int child) const override;
   Role role(int child) const override;
   State state(int child) const override;

   // actions
   int userActionCount(int child) const override;
   bool doAction(int action, int child, const QVariantList &params) override;
   QString actionText(int action, Text t, int child) const override;
};

#endif

QT_END_NAMESPACE

#endif // QACCESSIBLEOBJECT_H
