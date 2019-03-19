/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QMAINWINDOWLAYOUT_P_H
#define QMAINWINDOWLAYOUT_P_H

#include <qmainwindow.h>

#ifndef QT_NO_MAINWINDOW

#include <QtGui/qlayout.h>
#include <QtGui/qtabbar.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>
#include <QtCore/qbasictimer.h>
#include <qlayoutengine_p.h>
#include <qwidgetanimator_p.h>
#include <qdockarealayout_p.h>
#include <qtoolbararealayout_p.h>

//#define Q_DEBUG_MAINWINDOW_LAYOUT

#if defined(Q_DEBUG_MAINWINDOW_LAYOUT) && !defined(QT_NO_DOCKWIDGET)
QT_BEGIN_NAMESPACE
class QTextStream;
Q_GUI_EXPORT void qt_dumpLayout(QTextStream &qout, QMainWindow *window);
QT_END_NAMESPACE
#endif

#ifdef Q_OS_MAC

using CFTypeRef    = const void *;
using CFStringRef  = const struct __CFString *;

#include <qunifiedtoolbarsurface_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

class QToolBar;
class QRubberBand;

/* This data structure represents the state of all the tool-bars and dock-widgets. It's value based
   so it can be easilly copied into a temporary variable. All operations are performed without moving
   any widgets. Only when we are sure we have the desired state, we call apply(), which moves the widgets.
*/

class QMainWindowLayoutState
{

 public:
   QRect rect;
   QMainWindow *mainWindow;

   QMainWindowLayoutState(QMainWindow *win);

#ifndef QT_NO_TOOLBAR
   QToolBarAreaLayout toolBarAreaLayout;
#endif

#ifndef QT_NO_DOCKWIDGET
   QDockAreaLayout dockAreaLayout;
#else
   QLayoutItem *centralWidgetItem;
   QRect centralWidgetRect;
#endif

   void apply(bool animated);
   void deleteAllLayoutItems();
   void deleteCentralWidgetItem();

   QSize sizeHint() const;
   QSize minimumSize() const;
   void fitLayout();

   QLayoutItem *itemAt(int index, int *x) const;
   QLayoutItem *takeAt(int index, int *x);
   QList<int> indexOf(QWidget *widget) const;
   QLayoutItem *item(const QList<int> &path);
   QRect itemRect(const QList<int> &path) const;
   QRect gapRect(const QList<int> &path) const; // ### get rid of this, use itemRect() instead

   bool contains(QWidget *widget) const;

   void setCentralWidget(QWidget *widget);
   QWidget *centralWidget() const;

   QList<int> gapIndex(QWidget *widget, const QPoint &pos) const;
   bool insertGap(const QList<int> &path, QLayoutItem *item);
   void remove(const QList<int> &path);
   void remove(QLayoutItem *item);
   void clear();
   bool isValid() const;

   QLayoutItem *plug(const QList<int> &path);
   QLayoutItem *unplug(const QList<int> &path, QMainWindowLayoutState *savedState = 0);

   void saveState(QDataStream &stream) const;
   bool checkFormat(QDataStream &stream, bool pre43);
   bool restoreState(QDataStream &stream, const QMainWindowLayoutState &oldState);
};

class QMainWindowLayout : public QLayout
{
   GUI_CS_OBJECT(QMainWindowLayout)

 public:
   QMainWindowLayoutState layoutState, savedState;

   QMainWindowLayout(QMainWindow *mainwindow, QLayout *parentLayout);
   ~QMainWindowLayout();

   QMainWindow::DockOptions dockOptions;
   void setDockOptions(QMainWindow::DockOptions opts);
   bool usesHIToolBar(QToolBar *toolbar) const;

   void timerEvent(QTimerEvent *e) override;

   // status bar
   QLayoutItem *statusbar;

#ifndef QT_NO_STATUSBAR
   QStatusBar *statusBar() const;
   void setStatusBar(QStatusBar *sb);
#endif

   // central widget
   QWidget *centralWidget() const;
   void setCentralWidget(QWidget *cw);

#ifndef QT_NO_TOOLBAR
   // toolbars
   void addToolBarBreak(Qt::ToolBarArea area);
   void insertToolBarBreak(QToolBar *before);
   void removeToolBarBreak(QToolBar *before);

   void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar, bool needAddChildWidget = true);
   void insertToolBar(QToolBar *before, QToolBar *toolbar);
   Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;
   bool toolBarBreak(QToolBar *toolBar) const;
   void getStyleOptionInfo(QStyleOptionToolBar *option, QToolBar *toolBar) const;
   void removeToolBar(QToolBar *toolbar);
   void toggleToolBarsVisible();
   void moveToolBar(QToolBar *toolbar, int pos);
#endif


#ifndef QT_NO_DOCKWIDGET
   // dock widgets
   void setCorner(Qt::Corner corner, Qt::DockWidgetArea area);
   Qt::DockWidgetArea corner(Qt::Corner corner) const;
   void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget, Qt::Orientation orientation);
   void splitDockWidget(QDockWidget *after, QDockWidget *dockwidget, Qt::Orientation orientation);
   void tabifyDockWidget(QDockWidget *first, QDockWidget *second);
   Qt::DockWidgetArea dockWidgetArea(QDockWidget *dockwidget) const;
   void raise(QDockWidget *widget);
   void setVerticalTabsEnabled(bool enabled);
   bool restoreDockWidget(QDockWidget *dockwidget);

#ifndef QT_NO_TABBAR
   bool _documentMode;
   bool documentMode() const;
   void setDocumentMode(bool enabled);

   QTabBar *getTabBar();
   QSet<QTabBar *> usedTabBars;
   QList<QTabBar *> unusedTabBars;
   bool verticalTabsEnabled;

   QWidget *getSeparatorWidget();
   QSet<QWidget *> usedSeparatorWidgets;
   QList<QWidget *> unusedSeparatorWidgets;
   int sep; // separator extent

#ifndef QT_NO_TABWIDGET
   QTabWidget::TabPosition tabPositions[4];
   QTabWidget::TabShape _tabShape;

   QTabWidget::TabShape tabShape() const;
   void setTabShape(QTabWidget::TabShape tabShape);
   QTabWidget::TabPosition tabPosition(Qt::DockWidgetArea area) const;
   void setTabPosition(Qt::DockWidgetAreas areas, QTabWidget::TabPosition tabPosition);
#endif

#endif // QT_NO_TABBAR

   // separators
   QList<int> movingSeparator;
   QPoint movingSeparatorOrigin, movingSeparatorPos;
   QBasicTimer separatorMoveTimer;

   bool startSeparatorMove(const QPoint &pos);
   bool separatorMove(const QPoint &pos);
   bool endSeparatorMove(const QPoint &pos);
   void keepSize(QDockWidget *w);
#endif // QT_NO_DOCKWIDGET

   // save/restore

   enum { // sentinel values used to validate state data
      VersionMarker = 0xff
   };
   void saveState(QDataStream &stream) const;
   bool restoreState(QDataStream &stream);

   // QLayout interface
   void addItem(QLayoutItem *item) override;
   void setGeometry(const QRect &r) override;
   QLayoutItem *itemAt(int index) const override;
   QLayoutItem *takeAt(int index) override;
   int count() const override;

   QSize sizeHint() const override;
   QSize minimumSize() const override;
   void invalidate() override;

   mutable QSize szHint;
   mutable QSize minSize;

   // animations
   QWidgetAnimator widgetAnimator;
   QList<int> currentGapPos;
   QRect currentGapRect;
   QWidget *pluggingWidget;

#ifndef QT_NO_RUBBERBAND
   QRubberBand *gapIndicator;
#endif

   QList<int> hover(QLayoutItem *widgetItem, const QPoint &mousePos);
   bool plug(QLayoutItem *widgetItem);
   QLayoutItem *unplug(QWidget *widget);
   void revert(QLayoutItem *widgetItem);
   void updateGapIndicator();
   void paintDropIndicator(QPainter *p, QWidget *widget, const QRegion &clip);
   void applyState(QMainWindowLayoutState &newState, bool animate = true);
   void restore(bool keepSavedState = false);
   void updateHIToolBarStatus();
   void animationFinished(QWidget *widget);

 private :

#ifndef QT_NO_DOCKWIDGET
#ifndef QT_NO_TABBAR
   GUI_CS_SLOT_1(Private, void tabChanged())
   GUI_CS_SLOT_2(tabChanged)
#endif
#endif

#ifndef QT_NO_TABBAR
   void updateTabBarShapes();
#endif

#ifdef Q_OS_MAC
 public:
   struct ToolBarSaveState {
      ToolBarSaveState() : movable(false) { }
      ToolBarSaveState(bool newMovable, const QSize &newMax)
         : movable(newMovable), maximumSize(newMax) { }
      bool movable;
      QSize maximumSize;
   };

   QList<QToolBar *> qtoolbarsInUnifiedToolbarList;
   QList<void *> toolbarItemsCopy;
   QHash<void *, QToolBar *> unifiedToolbarHash;
   QHash<QToolBar *, ToolBarSaveState> toolbarSaveState;
   QHash<QString, QToolBar *> cocoaItemIDToToolbarHash;
   void insertIntoMacToolbar(QToolBar *before, QToolBar *after);
   void removeFromMacToolbar(QToolBar *toolbar);
   void cleanUpMacToolbarItems();
   void fixSizeInUnifiedToolbar(QToolBar *tb) const;
   bool activateUnifiedToolbarAfterFullScreen;
   void syncUnifiedToolbarVisibility();
   bool blockVisiblityCheck;


   QUnifiedToolbarSurface *unifiedSurface;
   void updateUnifiedToolbarOffset();
#endif

};
QT_END_NAMESPACE

#endif // QT_NO_MAINWINDOW

#endif // QDYNAMICMAINWINDOWLAYOUT_P_H
