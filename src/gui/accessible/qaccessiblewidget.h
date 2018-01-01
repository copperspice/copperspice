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

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#include <QtGui/qaccessibleobject.h>
#include <QPicture>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleWidgetPrivate;

class Q_GUI_EXPORT QAccessibleWidget : public QAccessibleObject
{
 public:
   explicit QAccessibleWidget(QWidget *o, Role r = Client, const QString &name = QString());

   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   Relation relationTo(int child, const QAccessibleInterface *other, int otherChild) const override;

   int childAt(int x, int y) const override;
   QRect rect(int child) const override;
   int navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const override;

   QString text(Text t, int child) const override;
   Role role(int child) const override;
   State state(int child) const override;

#ifndef QT_NO_ACTION
   int userActionCount(int child) const override;
   QString actionText(int action, Text t, int child) const override;
   bool doAction(int action, int child, const QVariantList &params) override;
#endif

 protected:
   ~QAccessibleWidget();
   QWidget *widget() const;
   QObject *parentObject() const;

   void addControllingSignal(const QString &signal);
   void setValue(const QString &value);
   void setDescription(const QString &desc);
   void setHelp(const QString &help);
   void setAccelerator(const QString &accel);

 private:
   friend class QAccessibleWidgetEx;
   QAccessibleWidgetPrivate *d;
   Q_DISABLE_COPY(QAccessibleWidget)
};

class Q_GUI_EXPORT QAccessibleWidgetEx : public QAccessibleObjectEx
{
 public:
   explicit QAccessibleWidgetEx(QWidget *o, Role r = Client, const QString &name = QString());

   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   Relation relationTo(int child, const QAccessibleInterface *other, int otherChild) const override;

   int childAt(int x, int y) const override;
   QRect rect(int child) const override;
   int navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const override;

   QString text(Text t, int child) const override;
   Role role(int child) const override;
   State state(int child) const override;

   QString actionText(int action, Text t, int child) const override;
   bool doAction(int action, int child, const QVariantList &params) override;

   QVariant invokeMethodEx(Method method, int child, const QVariantList &params) override;

 protected:
   ~QAccessibleWidgetEx();
   QWidget *widget() const;
   QObject *parentObject() const;

   void addControllingSignal(const QString &signal);
   void setValue(const QString &value);
   void setDescription(const QString &desc);
   void setHelp(const QString &help);
   void setAccelerator(const QString &accel);

 private:
   QAccessibleWidgetPrivate *d;
   Q_DISABLE_COPY(QAccessibleWidgetEx)
};

#endif

QT_END_NAMESPACE

#endif // QACCESSIBLEWIDGET_H
