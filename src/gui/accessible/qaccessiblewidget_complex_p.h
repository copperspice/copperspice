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

#ifndef COMPLEXWIDGETS_H
#define COMPLEXWIDGETS_H

#include <qpointer.h>
#include <qaccessiblewidget.h>
#include <qabstractitemview.h>

#ifndef QT_NO_ACCESSIBILITY

class QAbstractButton;
class QHeaderView;
class QTabBar;
class QComboBox;
class QTitleBar;
class QAbstractScrollArea;
class QScrollArea;

#ifndef QT_NO_SCROLLAREA
class QAccessibleAbstractScrollArea : public QAccessibleWidget
{
 public:
   explicit QAccessibleAbstractScrollArea(QWidget *widget);

   enum AbstractScrollAreaElement {
      Self = 0,
      Viewport,
      HorizontalContainer,
      VerticalContainer,
      CornerWidget,
      Undefined
   };

   QAccessibleInterface *child(int index) const override;
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   bool isValid() const override;
   QAccessibleInterface *childAt(int x, int y) const override;

   //protected:
   QAbstractScrollArea *abstractScrollArea() const;

 private:
   QWidgetList accessibleChildren() const;
   AbstractScrollAreaElement elementType(QWidget *widget) const;
   bool isLeftToRight() const;
};

class QAccessibleScrollArea : public QAccessibleAbstractScrollArea
{
 public:
   explicit QAccessibleScrollArea(QWidget *widget);
};
#endif

#ifndef QT_NO_TABBAR
class QAccessibleTabBar : public QAccessibleWidget
{
 public:
   explicit QAccessibleTabBar(QWidget *w);
   ~QAccessibleTabBar();

   int childCount() const override;
   QString text(QAccessible::Text t) const override;

   QAccessibleInterface *child(int index) const override;
   int indexOfChild(const QAccessibleInterface *child) const override;

 protected:
   QTabBar *tabBar() const;
   mutable QHash<int, QAccessible::Id> m_childInterfaces;
};
#endif

#ifndef QT_NO_COMBOBOX
class QAccessibleComboBox : public QAccessibleWidget
{
 public:
   explicit QAccessibleComboBox(QWidget *w);

   int childCount() const override;
   QAccessibleInterface *childAt(int x, int y) const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   QAccessibleInterface *child(int index) const override;

   QString text(QAccessible::Text t) const override;

   // QAccessibleActionInterface
   QStringList actionNames() const override;
   QString localizedActionDescription(const QString &actionName) const override;
   void doAction(const QString &actionName) override;
   QStringList keyBindingsForAction(const QString &actionName) const override;

 protected:
   QComboBox *comboBox() const;
};
#endif

#endif // QT_NO_ACCESSIBILITY

#endif
