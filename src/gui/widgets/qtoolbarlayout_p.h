/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTOOLBARLAYOUT_P_H
#define QTOOLBARLAYOUT_P_H

#include <QtGui/qlayout.h>
#include <qlayoutengine_p.h>
#include <QVector>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TOOLBAR

class QAction;
class QToolBarExtension;
class QMenu;

class QToolBarItem : public QWidgetItem
{
 public:
   QToolBarItem(QWidget *widget);
   bool isEmpty() const;

   QAction *action;
   bool customWidget;
};

class QToolBarLayout : public QLayout
{
   GUI_CS_OBJECT(QToolBarLayout)

 public:
   QToolBarLayout(QWidget *parent = 0);
   ~QToolBarLayout();

   void addItem(QLayoutItem *item);
   QLayoutItem *itemAt(int index) const;
   QLayoutItem *takeAt(int index);
   int count() const;

   bool isEmpty() const;
   void invalidate();
   Qt::Orientations expandingDirections() const;

   void setGeometry(const QRect &r);
   QSize minimumSize() const;
   QSize sizeHint() const;

   void insertAction(int index, QAction *action);
   int indexOf(QAction *action) const;
   int indexOf(QWidget *widget) const {
      return QLayout::indexOf(widget);
   }

   bool layoutActions(const QSize &size);
   QSize expandedSize(const QSize &size) const;
   bool expanded, animating;

   void setUsePopupMenu(bool set); // Yeah, there's no getter, but it's internal.
   void checkUsePopupMenu();

   bool movable() const;
   void updateMarginAndSpacing();
   bool hasExpandFlag() const;

   GUI_CS_SLOT_1(Public, void setExpanded(bool b))
   GUI_CS_SLOT_2(setExpanded)

 private:
   QList<QToolBarItem *> items;
   QSize hint, minSize;
   bool dirty, expanding, empty, expandFlag;
   QVector<QLayoutStruct> geomArray;
   QRect handRect;
   QToolBarExtension *extension;

   void updateGeomArray() const;
   QToolBarItem *createItem(QAction *action);
   QMenu *popupMenu;
};

#endif // QT_NO_TOOLBAR

QT_END_NAMESPACE

#endif // QTOOLBARLAYOUT_P_H
