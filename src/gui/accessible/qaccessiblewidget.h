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

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#include <qaccessibleobject.h>

#ifndef QT_NO_ACCESSIBILITY

class QAccessibleWidgetPrivate;

class Q_GUI_EXPORT QAccessibleWidget : public QAccessibleObject, public QAccessibleActionInterface
{
 public:
   explicit QAccessibleWidget(QWidget *widget, QAccessible::Role role = QAccessible::Client, const QString &name = QString());

   QAccessibleWidget(const QAccessibleWidget &) = delete;
   QAccessibleWidget &operator=(const QAccessibleWidget &) = delete;

   bool isValid() const override;
   QWindow *window() const override;
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;

   QVector<QPair<QAccessibleInterface *, QAccessible::Relation>>
         relations(QAccessible::Relation match = QAccessible::AllRelations) const override;

   QAccessibleInterface *focusChild() const override;

   QRect rect() const override;

   QAccessibleInterface *parent() const override;
   QAccessibleInterface *child(int index) const override;

   QString text(QAccessible::Text text) const override;
   QAccessible::Role role() const override;
   QAccessible::State state() const override;

   QColor foregroundColor() const override;
   QColor backgroundColor() const override;

   void *interface_cast(QAccessible::InterfaceType type) override;
   QStringList actionNames() const override;
   void doAction(const QString &actionName) override;
   QStringList keyBindingsForAction(const QString &actionName) const override;

 protected:
   ~QAccessibleWidget();
   QWidget *widget() const;
   QObject *parentObject() const;

   void addControllingSignal(const QString &signal);
   void addControllingSignal(const QMetaMethod &signal);

 private:
   QAccessibleWidgetPrivate *d;
};

#endif // QT_NO_ACCESSIBILITY

#endif
