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

#ifndef QMAINWINDOWLAYOUT_P_H
#define QMAINWINDOWLAYOUT_P_H

#include <qmainwindow.h>

#ifndef QT_NO_MAINWINDOW

#include <qlayout.h>
#include <qtabbar.h>
#include <qvector.h>
#include <qset.h>
#include <qbasictimer.h>
#include <qlayoutengine_p.h>
#include <qwidgetanimator_p.h>
#include <qdockarealayout_p.h>
#include <qtoolbararealayout_p.h>

class QToolBar;
class QRubberBand;

#ifndef QT_NO_DOCKWIDGET
class QDockWidgetGroupWindow : public QWidget
{
   GUI_CS_OBJECT(QDockWidgetGroupWindow)

 public:
   explicit QDockWidgetGroupWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag)
      : QWidget(parent, flags)
   {}

   QDockAreaLayoutInfo *layoutInfo() const;
   QDockWidget *topDockWidget() const;
   void destroyOrHideIfEmpty();
   void adjustFlags();

 protected:
   bool event(QEvent *) override;
   void paintEvent(QPaintEvent *) override;
};

// This item will be used in the layout for the gap item. We cannot use QWidgetItem directly
// because QWidgetItem functions return an empty size for widgets that are are floating.
class QDockWidgetGroupWindowItem : public QWidgetItem
{
 public:
   explicit QDockWidgetGroupWindowItem(QDockWidgetGroupWindow *parent) : QWidgetItem(parent) {}
   QSize minimumSize() const override {
      return lay()->minimumSize();
   }
   QSize maximumSize() const override {
      return lay()->maximumSize();
   }
   QSize sizeHint() const override {
      return lay()->sizeHint();
   }

 private:
   QLayout *lay() const {
      return const_cast<QDockWidgetGroupWindowItem *>(this)->widget()->layout();
   }
};
#endif

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
   QLayoutItem *unplug(const QList<int> &path, QMainWindowLayoutState *savedState = nullptr);

   void saveState(QDataStream &stream) const;
   bool checkFormat(QDataStream &stream);
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
   Qt::DockWidgetArea dockWidgetArea(QWidget *widget) const;
   void raise(QDockWidget *widget);
   void setVerticalTabsEnabled(bool enabled);
   bool restoreDockWidget(QDockWidget *dockwidget);
   QDockAreaLayoutInfo *dockInfo(QWidget *w);

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
   QDockWidgetGroupWindow *createTabbedDockWindow();
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
#endif

   // save/restore
   static constexpr const int VersionMarker = 0xff;

   void saveState(QDataStream &stream) const;
   bool restoreState(QDataStream &stream);

   // QLayout interface
   void addItem(QLayoutItem *item) override;
   void setGeometry(const QRect &rect) override;
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
   QPointer<QRubberBand> gapIndicator;
#endif
#ifndef QT_NO_DOCKWIDGET
   QPointer<QWidget> currentHoveredFloat; // set when dragging over a floating dock widget
   void setCurrentHoveredFloat(QWidget *w);
#endif

   void hover(QLayoutItem *widgetItem, const QPoint &mousePos);
   bool plug(QLayoutItem *widgetItem);
   QLayoutItem *unplug(QWidget *widget, bool group = false);
   void revert(QLayoutItem *widgetItem);

   void paintDropIndicator(QPainter *p, QWidget *widget, const QRegion &clip);
   void applyState(QMainWindowLayoutState &newState, bool animate = true);
   void restore(bool keepSavedState = false);
   void updateHIToolBarStatus();
   void animationFinished(QWidget *widget);

 private:
   GUI_CS_SLOT_1(Private, void updateGapIndicator())
   GUI_CS_SLOT_2(updateGapIndicator)

#ifndef QT_NO_DOCKWIDGET
#ifndef QT_NO_TABBAR
   GUI_CS_SLOT_1(Private, void tabChanged())
   GUI_CS_SLOT_2(tabChanged)

   GUI_CS_SLOT_1(Private, void tabMoved(int from, int to))
   GUI_CS_SLOT_2(tabMoved)
#endif
#endif

#ifndef QT_NO_TABBAR
   void updateTabBarShapes();
#endif

};
QDebug operator<<(QDebug debug, const QDockAreaLayout &layout);
QDebug operator<<(QDebug debug, const QMainWindowLayout *layout);

#endif // QT_NO_MAINWINDOW

#endif // QDYNAMICMAINWINDOWLAYOUT_P_H
