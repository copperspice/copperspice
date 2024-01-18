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

#ifndef QTOOLBARLAYOUT_P_H
#define QTOOLBARLAYOUT_P_H

#include <qlayout.h>
#include <qlayoutengine_p.h>
#include <QVector>

#ifndef QT_NO_TOOLBAR

class QAction;
class QToolBarExtension;
class QMenu;

class QToolBarItem : public QWidgetItem
{
 public:
   QToolBarItem(QWidget *widget);
   bool isEmpty() const override;

   QAction *action;
   bool customWidget;
};

class QToolBarLayout : public QLayout
{
   GUI_CS_OBJECT(QToolBarLayout)

 public:
   QToolBarLayout(QWidget *parent = nullptr);
   ~QToolBarLayout();

   void addItem(QLayoutItem *item) override;
   QLayoutItem *itemAt(int index) const override;
   QLayoutItem *takeAt(int index) override;
   int count() const override;

   bool isEmpty() const override;
   void invalidate() override;
   Qt::Orientations expandingDirections() const override;

   void setGeometry(const QRect &rect) override;
   QSize minimumSize() const override;
   QSize sizeHint() const override;

   void insertAction(int index, QAction *action);
   int indexOf(QAction *action) const;

   int indexOf(QWidget *widget) const override {
      return QLayout::indexOf(widget);
   }

   bool layoutActions(const QSize &size);
   QSize expandedSize(const QSize &size) const;
   bool expanded, animating;

   void setUsePopupMenu(bool set);    // there's no getter, this is internal but public
   void checkUsePopupMenu();

   bool movable() const;
   void updateMarginAndSpacing();
   bool hasExpandFlag() const;

   void updateMacBorderMetrics();
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


#endif // QTOOLBARLAYOUT_P_H
