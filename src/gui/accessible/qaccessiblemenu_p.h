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

#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include <qaccessiblewidget.h>
#include <qpointer.h>

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_MENU
class QMenu;
class QMenuBar;
class QAction;

class QAccessibleMenu : public QAccessibleWidget
{
 public:
   explicit QAccessibleMenu(QWidget *w);

   int childCount() const override;
   QAccessibleInterface *childAt(int x, int y) const override;

   QString text(QAccessible::Text t) const override;
   QAccessible::Role role() const override;
   QAccessibleInterface *child(int index) const override;
   QAccessibleInterface *parent() const override;
   int indexOfChild( const QAccessibleInterface *child ) const override;

 protected:
   QMenu *menu() const;
};

#ifndef QT_NO_MENUBAR
class QAccessibleMenuBar : public QAccessibleWidget
{
 public:
   explicit QAccessibleMenuBar(QWidget *w);

   QAccessibleInterface *child(int index) const override;
   int childCount() const override;

   int indexOfChild(const QAccessibleInterface *child) const override;

 protected:
   QMenuBar *menuBar() const;
};
#endif // QT_NO_MENUBAR


class QAccessibleMenuItem : public QAccessibleInterface, public QAccessibleActionInterface
{
 public:
   explicit QAccessibleMenuItem(QWidget *owner, QAction *w);

   ~QAccessibleMenuItem();
   void *interface_cast(QAccessible::InterfaceType t) override;

   int childCount() const override;
   QAccessibleInterface *childAt(int x, int y) const override;
   bool isValid() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;

   QAccessibleInterface *parent() const override;
   QAccessibleInterface *child(int index) const override;
   QObject *object() const override;
   QWindow *window() const override;

   QRect rect() const override;
   QAccessible::Role role() const override;
   void setText(QAccessible::Text t, const QString &text) override;
   QAccessible::State state() const override;
   QString text(QAccessible::Text t) const override;

   // QAccessibleActionInterface
   QStringList actionNames() const override;
   void doAction(const QString &actionName) override;
   QStringList keyBindingsForAction(const QString &actionName) const override;

   QWidget *owner() const;

 protected:
   QAction *action() const;

 private:
   QAction *m_action;
   QPointer<QWidget> m_owner; // can hold either QMenu or the QMenuBar that contains the action
};

#endif // QT_NO_MENU

#endif // QT_NO_ACCESSIBILITY
#endif // QACCESSIBLEMENU_H
