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

#ifndef QDOCKAREALAYOUT_P_H
#define QDOCKAREALAYOUT_P_H

#include <qrect.h>
#include <qpair.h>
#include <qlist.h>
#include <qvector.h>
#include <qlayout.h>

#ifndef QT_NO_DOCKWIDGET

class QLayoutItem;
class QWidget;
class QLayoutItem;
class QDockAreaLayoutInfo;
class QPlaceHolderItem;
class QDockWidget;
class QMainWindow;
class QWidgetAnimator;
class QMainWindowLayout;
class QTabBar;

struct QLayoutStruct;

// The classes in this file represent the tree structure that represents all the docks
// Also see the wiki internal documentation
// At the root of the tree is: QDockAreaLayout, which handles all 4 sides, so there is only one.
// For each side it has one QDockAreaLayoutInfo child. (See QDockAreaLayout::docks.)
// The QDockAreaLayoutInfo have QDockAreaLayoutItems as children (See QDockAreaLayoutInfo::item_list),
// which then has one QDockAreaLayoutInfo as a child. (QDockAreaLayoutItem::subInfo) or
// a widgetItem if this is a node of the tree (QDockAreaLayoutItem::widgetItem)
//
// A path indetifies uniquely one object in this tree, the first number being the side and all the following
// indexes into the QDockAreaLayoutInfo::item_list.

struct QDockAreaLayoutItem {
   enum ItemFlags {
      NoFlags  = 0,
      GapItem  = 1,
      KeepSize = 2
   };

   QDockAreaLayoutItem(QLayoutItem *_widgetItem = nullptr);
   QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo);
   QDockAreaLayoutItem(QPlaceHolderItem *_placeHolderItem);
   QDockAreaLayoutItem(const QDockAreaLayoutItem &other);
   ~QDockAreaLayoutItem();

   QDockAreaLayoutItem &operator = (const QDockAreaLayoutItem &other);

   bool skip() const;
   QSize minimumSize() const;
   QSize maximumSize() const;
   QSize sizeHint() const;
   bool expansive(Qt::Orientation o) const;
   bool hasFixedSize(Qt::Orientation o) const;

   QLayoutItem *widgetItem;
   QDockAreaLayoutInfo *subinfo;
   QPlaceHolderItem *placeHolderItem;
   int pos;
   int size;
   uint flags;
};

class QPlaceHolderItem
{
 public:
   QPlaceHolderItem() : hidden(false), window(false) {}
   QPlaceHolderItem(QWidget *w);

   QString objectName;
   bool hidden, window;
   QRect topLevelRect;
};

class QDockAreaLayoutInfo
{
 public:
   static constexpr const uchar SequenceMarker = 0xfc;
   static constexpr const uchar TabMarker      = 0xfa;
   static constexpr const uchar WidgetMarker   = 0xfb;

   enum TabMode {
      NoTabs,
      AllowTabs,
      ForceTabs
   };

   QDockAreaLayoutInfo();
   QDockAreaLayoutInfo(const int *_sep, QInternal::DockPosition _dockPos, Qt::Orientation _o,
      int tbhape, QMainWindow *window);

   QSize minimumSize() const;
   QSize maximumSize() const;
   QSize sizeHint() const;
   QSize size() const;

   bool insertGap(const QList<int> &path, QLayoutItem *dockWidgetItem);
   QLayoutItem *plug(const QList<int> &path);
   QLayoutItem *unplug(const QList<int> &path);

   QList<int> gapIndex(const QPoint &pos, bool nestingEnabled, TabMode tabMode) const;
   void remove(const QList<int> &path);
   void unnest(int index);
   void split(int index, Qt::Orientation orientation, QLayoutItem *dockWidgetItem);
   void tab(int index, QLayoutItem *dockWidgetItem);

   QDockAreaLayoutItem &item(const QList<int> &path);
   QDockAreaLayoutInfo *info(const QList<int> &path);
   QDockAreaLayoutInfo *info(QWidget *widget);

   void saveState(QDataStream &stream) const;
   bool restoreState(QDataStream &stream, QList<QDockWidget *> &widgets, bool testing);

   void fitItems();
   bool expansive(Qt::Orientation o) const;
   int changeSize(int index, int size, bool below);
   QRect itemRect(int index) const;
   QRect itemRect(const QList<int> &path) const;
   QRect separatorRect(int index) const;
   QRect separatorRect(const QList<int> &path) const;

   void clear();
   bool isEmpty() const;
   bool hasFixedSize() const;
   QList<int> findSeparator(const QPoint &pos) const;
   int next(int idx) const;
   int prev(int idx) const;

   QList<int> indexOf(QWidget *widget) const;
   QList<int> indexOfPlaceHolder(const QString &objectName) const;

   void apply(bool animate);

   void paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip, const QPoint &mouse) const;
   QRegion separatorRegion() const;
   int separatorMove(int index, int delta);

   QLayoutItem *itemAt(int *x, int index) const;
   QLayoutItem *takeAt(int *x, int index);
   void deleteAllLayoutItems();

   QMainWindowLayout *mainWindowLayout() const;

   const int *sep;
   mutable QVector<QWidget *> separatorWidgets;
   QInternal::DockPosition dockPos;
   Qt::Orientation o;
   QRect rect;
   QMainWindow *mainWindow;
   QList<QDockAreaLayoutItem> item_list;

#ifndef QT_NO_TABBAR
   void updateSeparatorWidgets() const;
   QSet<QWidget *> usedSeparatorWidgets() const;
#endif

#ifndef QT_NO_TABBAR
   quintptr currentTabId() const;
   void setCurrentTab(QWidget *widget);
   void setCurrentTabId(quintptr id);
   QRect tabContentRect() const;
   bool tabbed;
   QTabBar *tabBar;
   int tabBarShape;

   void reparentWidgets(QWidget *p);
   bool updateTabBar() const;
   void setTabBarShape(int shape);
   QSize tabBarMinimumSize() const;
   QSize tabBarSizeHint() const;

   QSet<QTabBar *> usedTabBars() const;
   int tabIndexToListIndex(int) const;
   void moveTab(int from, int to);
#endif
};

class QDockAreaLayout
{
 public:
   // when a dock area is empty, how "wide" is it?
   static constexpr const int EmptyDropAreaSize = 80;

   enum DockMarker {
      DockWidgetStateMarker       = 0xfd,
      FloatingDockWidgetTabMarker = 0xf9
   };

   Qt::DockWidgetArea corners[4];      // use a Qt::Corner for indexing
   QRect rect;
   QLayoutItem *centralWidgetItem;
   QMainWindow *mainWindow;
   QRect centralWidgetRect;
   QDockAreaLayout(QMainWindow *win);
   QDockAreaLayoutInfo docks[4];
   int sep;                            // separator extent

   // determines if we should use the sizehint for the dock areas
   // (true until the layout is restored or the central widget is set)
   bool fallbackToSizeHints;

   mutable QVector<QWidget *> separatorWidgets;

   bool isValid() const;

   static QRect constrainedRect(QRect rect, QWidget *widget);

   void saveState(QDataStream &stream) const;
   bool restoreState(QDataStream &stream, const QList<QDockWidget *> &widgets, bool testing = false);

   QList<int> indexOfPlaceHolder(const QString &objectName) const;
   QList<int> indexOf(QWidget *dockWidget) const;
   QList<int> gapIndex(const QPoint &pos) const;
   QList<int> findSeparator(const QPoint &pos) const;

   QDockAreaLayoutItem &item(const QList<int> &path);
   QDockAreaLayoutInfo *info(const QList<int> &path);
   const QDockAreaLayoutInfo *info(const QList<int> &path) const;
   QDockAreaLayoutInfo *info(QWidget *widget);
   QRect itemRect(const QList<int> &path) const;
   QRect separatorRect(int index) const;
   QRect separatorRect(const QList<int> &path) const;

   bool insertGap(const QList<int> &path, QLayoutItem *dockWidgetItem);
   QLayoutItem *plug(const QList<int> &path);
   QLayoutItem *unplug(const QList<int> &path);
   void remove(const QList<int> &path);
   void removePlaceHolder(const QString &name);

   void fitLayout();

   void clear();

   QSize sizeHint() const;
   QSize minimumSize() const;

   void addDockWidget(QInternal::DockPosition pos, QDockWidget *dockWidget, Qt::Orientation orientation);
   bool restoreDockWidget(QDockWidget *dockWidget);
   void splitDockWidget(QDockWidget *after, QDockWidget *dockWidget,
      Qt::Orientation orientation);
   void tabifyDockWidget(QDockWidget *first, QDockWidget *second);
   void resizeDocks(const QList<QDockWidget *> &docks, const QList<int> &sizes, Qt::Orientation o);

   void apply(bool animate);

   void paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip,
      const QPoint &mouse) const;
   QRegion separatorRegion() const;
   int separatorMove(const QList<int> &separator, const QPoint &origin, const QPoint &dest);

#ifndef QT_NO_TABBAR
   void updateSeparatorWidgets() const;
#endif

   QLayoutItem *itemAt(int *x, int index) const;
   QLayoutItem *takeAt(int *x, int index);
   void deleteAllLayoutItems();

   void getGrid(QVector<QLayoutStruct> *ver_struct_list, QVector<QLayoutStruct> *hor_struct_list);
   void setGrid(QVector<QLayoutStruct> *ver_struct_list, QVector<QLayoutStruct> *hor_struct_list);

   QRect gapRect(const QList<int> &path) const;

   void keepSize(QDockWidget *w);

#ifndef QT_NO_TABBAR
   QSet<QTabBar *> usedTabBars() const;
   QSet<QWidget *> usedSeparatorWidgets() const;
#endif

   void styleChangedEvent();
};

#endif // QT_NO_QDOCKWIDGET

#endif // QDOCKAREALAYOUT_P_H
