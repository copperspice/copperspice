/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QTOOLBARAREALAYOUT_P_H
#define QTOOLBARAREALAYOUT_P_H

#include <QList>
#include <QSize>
#include <QRect>

static inline int pick(Qt::Orientation o, const QPoint &pos)
{
   return o == Qt::Horizontal ? pos.x() : pos.y();
}

static inline int pick(Qt::Orientation o, const QSize &size)
{
   return o == Qt::Horizontal ? size.width() : size.height();
}

static inline int &rpick(Qt::Orientation o, QPoint &pos)
{
   return o == Qt::Horizontal ? pos.rx() : pos.ry();
}

static inline int &rpick(Qt::Orientation o, QSize &size)
{
   return o == Qt::Horizontal ? size.rwidth() : size.rheight();
}

static inline QSizePolicy::Policy pick(Qt::Orientation o, const QSizePolicy &policy)
{
   return o == Qt::Horizontal ? policy.horizontalPolicy() : policy.verticalPolicy();
}

static inline int perp(Qt::Orientation o, const QPoint &pos)
{
   return o == Qt::Vertical ? pos.x() : pos.y();
}

static inline int perp(Qt::Orientation o, const QSize &size)
{
   return o == Qt::Vertical ? size.width() : size.height();
}

static inline int &rperp(Qt::Orientation o, QPoint &pos)
{
   return o == Qt::Vertical ? pos.rx() : pos.ry();
}

static inline int &rperp(Qt::Orientation o, QSize &size)
{
   return o == Qt::Vertical ? size.rwidth() : size.rheight();
}

#ifndef QT_NO_TOOLBAR

class QToolBar;
class QLayoutItem;
class QMainWindow;
class QStyleOptionToolBar;

class QToolBarAreaLayoutItem
{

 public:
   QToolBarAreaLayoutItem(QLayoutItem *item = 0)
      : widgetItem(item), pos(0), size(-1), preferredSize(-1), gap(false) {}

   bool skip() const;
   QSize minimumSize() const;
   QSize sizeHint() const;
   QSize realSizeHint() const;

   void resize(Qt::Orientation o, int newSize) {
      newSize = qMax(pick(o, minimumSize()), newSize);
      int sizeh = pick(o, sizeHint());
      if (newSize == sizeh) {
         preferredSize = -1;
         size = sizeh;
      } else {
         preferredSize = newSize;
      }
   }

   void extendSize(Qt::Orientation o, int extent) {
      int newSize = qMax(pick(o, minimumSize()), (preferredSize > 0 ? preferredSize : pick(o, sizeHint())) + extent);
      int sizeh = pick(o, sizeHint());
      if (newSize == sizeh) {
         preferredSize = -1;
         size = sizeh;
      } else {
         preferredSize = newSize;
      }
   }

   QLayoutItem *widgetItem;
   int pos;
   int size;
   int preferredSize;
   bool gap;
};

class QToolBarAreaLayoutLine
{
 public:
   QToolBarAreaLayoutLine(Qt::Orientation orientation);

   QSize sizeHint() const;
   QSize minimumSize() const;

   void fitLayout();
   bool skip() const;

   QRect rect;
   Qt::Orientation o;

   QList<QToolBarAreaLayoutItem> toolBarItems;
};

class QToolBarAreaLayoutInfo
{
 public:
   QToolBarAreaLayoutInfo(QInternal::DockPosition pos = QInternal::TopDock);

   QList<QToolBarAreaLayoutLine> lines;

   QSize sizeHint() const;
   QSize minimumSize() const;

   void fitLayout();

   QLayoutItem *insertToolBar(QToolBar *before, QToolBar *toolBar);
   void insertItem(QToolBar *before, QLayoutItem *item);
   void removeToolBar(QToolBar *toolBar);
   void insertToolBarBreak(QToolBar *before);
   void removeToolBarBreak(QToolBar *before);
   void moveToolBar(QToolBar *toolbar, int pos);

   QList<int> gapIndex(const QPoint &pos, int *maxDistance) const;
   bool insertGap(const QList<int> &path, QLayoutItem *item);
   void clear();
   QRect itemRect(const QList<int> &path) const;
   int distance(const QPoint &pos) const;

   QRect rect;
   Qt::Orientation o;
   QInternal::DockPosition dockPos;
   bool dirty;
};

class QToolBarAreaLayout
{
 public:
   enum { // sentinel values used to validate state data
      ToolBarStateMarker = 0xfe,
      ToolBarStateMarkerEx = 0xfc
   };

   QRect rect;
   const QMainWindow *mainWindow;
   QToolBarAreaLayoutInfo docks[4];
   bool visible;

   QToolBarAreaLayout(const QMainWindow *win);

   QRect fitLayout();

   QSize minimumSize(const QSize &centerMin) const;
   QRect rectHint(const QRect &r) const;
   QSize sizeHint(const QSize &center) const;
   void apply(bool animate);

   QLayoutItem *itemAt(int *x, int index) const;
   QLayoutItem *takeAt(int *x, int index);
   void deleteAllLayoutItems();

   QLayoutItem *insertToolBar(QToolBar *before, QToolBar *toolBar);
   void removeToolBar(QToolBar *toolBar);
   QLayoutItem *addToolBar(QInternal::DockPosition pos, QToolBar *toolBar);
   void insertToolBarBreak(QToolBar *before);
   void removeToolBarBreak(QToolBar *before);
   void addToolBarBreak(QInternal::DockPosition pos);
   void moveToolBar(QToolBar *toolbar, int pos);

   void insertItem(QInternal::DockPosition pos, QLayoutItem *item);
   void insertItem(QToolBar *before, QLayoutItem *item);

   QInternal::DockPosition findToolBar(QToolBar *toolBar) const;
   bool toolBarBreak(QToolBar *toolBar) const;

   void getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const;

   QList<int> indexOf(QWidget *toolBar) const;
   QList<int> gapIndex(const QPoint &pos) const;
   QList<int> currentGapIndex() const;
   bool insertGap(const QList<int> &path, QLayoutItem *item);
   void remove(const QList<int> &path);
   void remove(QLayoutItem *item);
   void clear();
   QToolBarAreaLayoutItem *item(const QList<int> &path);
   QRect itemRect(const QList<int> &path) const;
   QLayoutItem *plug(const QList<int> &path);
   QLayoutItem *unplug(const QList<int> &path, QToolBarAreaLayout *other);

   void saveState(QDataStream &stream) const;
   bool restoreState(QDataStream &stream, const QList<QToolBar *> &toolBars, uchar tmarker, bool testing = false);
   bool isEmpty() const;
};

#endif // QT_NO_TOOLBAR
#endif // QTOOLBARAREALAYOUT_P_H
