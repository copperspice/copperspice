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

#include <qmainwindow.h>
#include <qmainwindowlayout_p.h>

#ifndef QT_NO_MAINWINDOW

#include <qdockwidget.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qevent.h>
#include <qstyle.h>
#include <qdebug.h>
#include <qpainter.h>
#include <qwidget_p.h>
#include <qtoolbar_p.h>
#include <qwidgetanimator_p.h>

// #define QT_EXPERIMENTAL_CLIENT_DECORATIONS

#ifdef Q_OS_DARWIN
#include <qplatform_nativeinterface.h>
#endif

class QMainWindowPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QMainWindow)

 public:
   inline QMainWindowPrivate()
      : layout(nullptr), explicitIconSize(false), toolButtonStyle(Qt::ToolButtonIconOnly)

#ifdef Q_OS_DARWIN
      , useUnifiedToolBar(false)
#endif

#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
      , hasOldCursor(false), cursorAdjusted(false)
#endif
   {
   }

   QMainWindowLayout *layout;
   QSize iconSize;
   bool explicitIconSize;
   Qt::ToolButtonStyle toolButtonStyle;
#ifdef Q_OS_DARWIN
   bool useUnifiedToolBar;
#endif

   void init();
   QList<int> hoverSeparator;
   QPoint hoverPos;

#if ! defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
   QCursor separatorCursor(const QList<int> &path) const;
   void adjustCursor(const QPoint &pos);
   QCursor oldCursor;
   QCursor adjustedCursor;
   uint hasOldCursor   : 1;
   uint cursorAdjusted : 1;
#endif

   static inline QMainWindowLayout *mainWindowLayout(const QMainWindow *mainWindow) {
      return mainWindow ? mainWindow->d_func()->layout : static_cast<QMainWindowLayout *>(nullptr);
   }
};

QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *mainWindow)
{
   return QMainWindowPrivate::mainWindowLayout(mainWindow);
}

#ifdef QT_EXPERIMENTAL_CLIENT_DECORATIONS
Q_GUI_EXPORT void qt_setMainWindowTitleWidget(QMainWindow *mainWindow, Qt::DockWidgetArea area, QWidget *widget)
{
   QGridLayout *topLayout = qobject_cast<QGridLayout *>(mainWindow->layout());
   Q_ASSERT(topLayout);

   int row = 0;
   int column = 0;

   switch (area) {
      case Qt::LeftDockWidgetArea:
         row = 1;
         column = 0;
         break;
      case Qt::TopDockWidgetArea:
         row = 0;
         column = 1;
         break;
      case Qt::BottomDockWidgetArea:
         row = 2;
         column = 1;
         break;
      case Qt::RightDockWidgetArea:
         row = 1;
         column = 2;
         break;
      default:
         Q_ASSERT_X(false, "qt_setMainWindowTitleWidget", "Unknown area");
         return;
   }

   if (QLayoutItem *oldItem = topLayout->itemAtPosition(row, column)) {
      delete oldItem->widget();
   }

   topLayout->addWidget(widget, row, column);
}
#endif

void QMainWindowPrivate::init()
{
   Q_Q(QMainWindow);

#ifdef QT_EXPERIMENTAL_CLIENT_DECORATIONS
   QGridLayout *topLayout = new QGridLayout(q);
   topLayout->setContentsMargins(0, 0, 0, 0);

   layout = new QMainWindowLayout(q, topLayout);

   topLayout->addItem(layout, 1, 1);
#else
   layout = new QMainWindowLayout(q, nullptr);
#endif

   const int metric = q->style()->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, q);
   iconSize = QSize(metric, metric);
   q->setAttribute(Qt::WA_Hover);
}

/*
    The Main Window:

    +----------------------------------------------------------+
    | Menu Bar                                                 |
    +----------------------------------------------------------+
    | Tool Bar Area                                            |
    |   +--------------------------------------------------+   |
    |   | Dock Window Area                                 |   |
    |   |   +------------------------------------------+   |   |
    |   |   |                                          |   |   |
    |   |   | Central Widget                           |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   +------------------------------------------+   |   |
    |   |                                                  |   |
    |   +--------------------------------------------------+   |
    |                                                          |
    +----------------------------------------------------------+
    | Status Bar                                               |
    +----------------------------------------------------------+

*/

QMainWindow::QMainWindow(QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::Window)
{
   d_func()->init();
}

QMainWindow::~QMainWindow()
{
   // no code
}

void QMainWindow::setDockOptions(DockOptions opt)
{
   Q_D(QMainWindow);
   d->layout->setDockOptions(opt);
}

QMainWindow::DockOptions QMainWindow::dockOptions() const
{
   Q_D(const QMainWindow);
   return d->layout->dockOptions;
}

QSize QMainWindow::iconSize() const
{
   return d_func()->iconSize;
}

void QMainWindow::setIconSize(const QSize &iconSize)
{
   Q_D(QMainWindow);
   QSize sz = iconSize;

   if (!sz.isValid()) {
      const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, this);
      sz = QSize(metric, metric);
   }
   if (d->iconSize != sz) {
      d->iconSize = sz;
      emit iconSizeChanged(d->iconSize);
   }
   d->explicitIconSize = iconSize.isValid();
}

Qt::ToolButtonStyle QMainWindow::toolButtonStyle() const
{
   return d_func()->toolButtonStyle;
}

void QMainWindow::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
   Q_D(QMainWindow);
   if (d->toolButtonStyle == toolButtonStyle) {
      return;
   }
   d->toolButtonStyle = toolButtonStyle;
   emit toolButtonStyleChanged(d->toolButtonStyle);
}

#ifndef QT_NO_MENUBAR

QMenuBar *QMainWindow::menuBar() const
{
   QMenuBar *menuBar = qobject_cast<QMenuBar *>(layout()->menuBar());
   if (!menuBar) {
      QMainWindow *self = const_cast<QMainWindow *>(this);
      menuBar = new QMenuBar(self);
      self->setMenuBar(menuBar);
   }
   return menuBar;
}

void QMainWindow::setMenuBar(QMenuBar *menuBar)
{
   QLayout *topLayout = layout();

   if (topLayout->menuBar() && topLayout->menuBar() != menuBar) {
      // Reparent corner widgets before we delete the old menu bar.
      QMenuBar *oldMenuBar = qobject_cast<QMenuBar *>(topLayout->menuBar());

      if (menuBar) {
         // TopLeftCorner widget.
         QWidget *cornerWidget = oldMenuBar->cornerWidget(Qt::TopLeftCorner);

         if (cornerWidget) {
            menuBar->setCornerWidget(cornerWidget, Qt::TopLeftCorner);
         }

         // TopRightCorner widget.
         cornerWidget = oldMenuBar->cornerWidget(Qt::TopRightCorner);
         if (cornerWidget) {
            menuBar->setCornerWidget(cornerWidget, Qt::TopRightCorner);
         }
      }

      oldMenuBar->hide();
      oldMenuBar->deleteLater();
   }

   topLayout->setMenuBar(menuBar);
}

QWidget *QMainWindow::menuWidget() const
{
   QWidget *menuBar = d_func()->layout->menuBar();
   return menuBar;
}

void QMainWindow::setMenuWidget(QWidget *menuBar)
{
   Q_D(QMainWindow);

   if (d->layout->menuBar() && d->layout->menuBar() != menuBar) {
      d->layout->menuBar()->hide();
      d->layout->menuBar()->deleteLater();
   }

   d->layout->setMenuBar(menuBar);
}
#endif // QT_NO_MENUBAR

#ifndef QT_NO_STATUSBAR
QStatusBar *QMainWindow::statusBar() const
{
   QStatusBar *statusbar = d_func()->layout->statusBar();

   if (! statusbar) {
      QMainWindow *self = const_cast<QMainWindow *>(this);
      statusbar = new QStatusBar(self);
      statusbar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      self->setStatusBar(statusbar);
   }

   return statusbar;
}

void QMainWindow::setStatusBar(QStatusBar *statusbar)
{
   Q_D(QMainWindow);

   if (d->layout->statusBar() && d->layout->statusBar() != statusbar) {
      d->layout->statusBar()->hide();
      d->layout->statusBar()->deleteLater();
   }

   d->layout->setStatusBar(statusbar);
}
#endif

QWidget *QMainWindow::centralWidget() const
{
   return d_func()->layout->centralWidget();
}

void QMainWindow::setCentralWidget(QWidget *widget)
{
   Q_D(QMainWindow);
   if (d->layout->centralWidget() && d->layout->centralWidget() != widget) {
      d->layout->centralWidget()->hide();
      d->layout->centralWidget()->deleteLater();
   }
   d->layout->setCentralWidget(widget);
}

QWidget *QMainWindow::takeCentralWidget()
{
   Q_D(QMainWindow);

   QWidget *oldcentralwidget = d->layout->centralWidget();

   if (oldcentralwidget) {
      oldcentralwidget->setParent(nullptr);
      d->layout->setCentralWidget(nullptr);
   }
   return oldcentralwidget;
}

#ifndef QT_NO_DOCKWIDGET

void QMainWindow::setCorner(Qt::Corner corner, Qt::DockWidgetArea area)
{
   bool valid = false;

   switch (corner) {
      case Qt::TopLeftCorner:
         valid = (area == Qt::TopDockWidgetArea || area == Qt::LeftDockWidgetArea);
         break;

      case Qt::TopRightCorner:
         valid = (area == Qt::TopDockWidgetArea || area == Qt::RightDockWidgetArea);
         break;

      case Qt::BottomLeftCorner:
         valid = (area == Qt::BottomDockWidgetArea || area == Qt::LeftDockWidgetArea);
         break;

      case Qt::BottomRightCorner:
         valid = (area == Qt::BottomDockWidgetArea || area == Qt::RightDockWidgetArea);
         break;
   }

   if (! valid) {
      qWarning("QMainWindow::setCorner(): 'area' is not valid for 'corner'");
   } else {
      d_func()->layout->setCorner(corner, area);
   }
}

Qt::DockWidgetArea QMainWindow::corner(Qt::Corner corner) const
{
   return d_func()->layout->corner(corner);
}
#endif

#ifndef QT_NO_TOOLBAR

static bool checkToolBarArea(Qt::ToolBarArea area, const char *where)
{
   switch (area) {
      case Qt::LeftToolBarArea:
      case Qt::RightToolBarArea:
      case Qt::TopToolBarArea:
      case Qt::BottomToolBarArea:
         return true;

      default:
         break;
   }

   qWarning("%s Invalid 'area' argument", where);

   return false;
}

void QMainWindow::addToolBarBreak(Qt::ToolBarArea area)
{
   if (! checkToolBarArea(area, "QMainWindow::addToolBarBreak()")) {
      return;
   }

   d_func()->layout->addToolBarBreak(area);
}

void QMainWindow::insertToolBarBreak(QToolBar *before)
{
   d_func()->layout->insertToolBarBreak(before);
}

void QMainWindow::removeToolBarBreak(QToolBar *before)
{
   Q_D(QMainWindow);
   d->layout->removeToolBarBreak(before);
}

void QMainWindow::addToolBar(Qt::ToolBarArea area, QToolBar *toolbar)
{
   if (! checkToolBarArea(area, "QMainWindow::addToolBar()")) {
      return;
   }

   Q_D(QMainWindow);

   disconnect(this, &QMainWindow::iconSizeChanged,        toolbar, &QToolBar::_q_updateIconSize);
   disconnect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &QToolBar::_q_updateToolButtonStyle);

   if (toolbar->d_func()->state && toolbar->d_func()->state->dragging) {
      //removing a toolbar which is dragging will cause crash

#ifndef QT_NO_DOCKWIDGET
      bool animated = isAnimated();
      setAnimated(false);
#endif

      toolbar->d_func()->endDrag();

#ifndef QT_NO_DOCKWIDGET
      setAnimated(animated);
#endif

   }

   if (! d->layout->usesHIToolBar(toolbar)) {
      d->layout->removeWidget(toolbar);
   } else {
      d->layout->removeToolBar(toolbar);
   }

   toolbar->d_func()->_q_updateIconSize(d->iconSize);
   toolbar->d_func()->_q_updateToolButtonStyle(d->toolButtonStyle);

   connect(this, &QMainWindow::iconSizeChanged,        toolbar, &QToolBar::_q_updateIconSize);
   connect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &QToolBar::_q_updateToolButtonStyle);

   d->layout->addToolBar(area, toolbar);
}

void QMainWindow::addToolBar(QToolBar *toolbar)
{
   addToolBar(Qt::TopToolBarArea, toolbar);
}

QToolBar *QMainWindow::addToolBar(const QString &title)
{
   QToolBar *toolBar = new QToolBar(this);
   toolBar->setWindowTitle(title);
   addToolBar(toolBar);

   return toolBar;
}

void QMainWindow::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
   Q_D(QMainWindow);

   d->layout->removeToolBar(toolbar);

   toolbar->d_func()->_q_updateIconSize(d->iconSize);
   toolbar->d_func()->_q_updateToolButtonStyle(d->toolButtonStyle);

   connect(this, &QMainWindow::iconSizeChanged,        toolbar, &QToolBar::_q_updateIconSize);
   connect(this, &QMainWindow::toolButtonStyleChanged, toolbar, &QToolBar::_q_updateToolButtonStyle);

   d->layout->insertToolBar(before, toolbar);
}

void QMainWindow::removeToolBar(QToolBar *toolbar)
{
   if (toolbar) {
      d_func()->layout->removeToolBar(toolbar);
      toolbar->hide();
   }
}

Qt::ToolBarArea QMainWindow::toolBarArea(QToolBar *toolbar) const
{
   return d_func()->layout->toolBarArea(toolbar);
}

bool QMainWindow::toolBarBreak(QToolBar *toolbar) const
{
   return d_func()->layout->toolBarBreak(toolbar);
}

#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET

bool QMainWindow::isAnimated() const
{
   Q_D(const QMainWindow);
   return d->layout->dockOptions & AnimatedDocks;
}

void QMainWindow::setAnimated(bool enabled)
{
   Q_D(QMainWindow);

   DockOptions opts = d->layout->dockOptions;
   if (enabled) {
      opts |= AnimatedDocks;
   } else {
      opts &= ~AnimatedDocks;
   }

   d->layout->setDockOptions(opts);
}

bool QMainWindow::isDockNestingEnabled() const
{
   Q_D(const QMainWindow);
   return d->layout->dockOptions & AllowNestedDocks;
}

void QMainWindow::setDockNestingEnabled(bool enabled)
{
   Q_D(QMainWindow);

   DockOptions opts = d->layout->dockOptions;

   if (enabled) {
      opts |= AllowNestedDocks;
   } else {
      opts &= ~AllowNestedDocks;
   }

   d->layout->setDockOptions(opts);
}

static bool checkDockWidgetArea(Qt::DockWidgetArea area, const char *where)
{
   switch (area) {
      case Qt::LeftDockWidgetArea:
      case Qt::RightDockWidgetArea:
      case Qt::TopDockWidgetArea:
      case Qt::BottomDockWidgetArea:
         return true;

      default:
         break;
   }

   qWarning("%s: invalid 'area' argument", where);

   return false;
}

#ifndef QT_NO_TABBAR

bool QMainWindow::documentMode() const
{
   return d_func()->layout->documentMode();
}

void QMainWindow::setDocumentMode(bool enabled)
{
   d_func()->layout->setDocumentMode(enabled);
}
#endif

#ifndef QT_NO_TABWIDGET

QTabWidget::TabShape QMainWindow::tabShape() const
{
   return d_func()->layout->tabShape();
}

void QMainWindow::setTabShape(QTabWidget::TabShape tabShape)
{
   d_func()->layout->setTabShape(tabShape);
}

QTabWidget::TabPosition QMainWindow::tabPosition(Qt::DockWidgetArea area) const
{
   if (! checkDockWidgetArea(area, "QMainWindow::tabPosition")) {
      return QTabWidget::South;
   }
   return d_func()->layout->tabPosition(area);
}

void QMainWindow::setTabPosition(Qt::DockWidgetAreas areas, QTabWidget::TabPosition tabPosition)
{
   d_func()->layout->setTabPosition(areas, tabPosition);
}
#endif

void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
   if (! checkDockWidgetArea(area, "QMainWindow::addDockWidget")) {
      return;
   }

   Qt::Orientation orientation = Qt::Vertical;
   switch (area) {
      case Qt::TopDockWidgetArea:
      case Qt::BottomDockWidgetArea:
         orientation = Qt::Horizontal;
         break;

      default:
         break;
   }

   d_func()->layout->removeWidget(dockwidget); // in case it was already in here
   addDockWidget(area, dockwidget, orientation);
}

bool QMainWindow::restoreDockWidget(QDockWidget *dockwidget)
{
   return d_func()->layout->restoreDockWidget(dockwidget);
}

void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget,
   Qt::Orientation orientation)
{
   if (!checkDockWidgetArea(area, "QMainWindow::addDockWidget")) {
      return;
   }

   // add a window to an area, placing done relative to the previous
   d_func()->layout->addDockWidget(area, dockwidget, orientation);
}

void QMainWindow::splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
   Qt::Orientation orientation)
{
   d_func()->layout->splitDockWidget(after, dockwidget, orientation);
}

void QMainWindow::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
   d_func()->layout->tabifyDockWidget(first, second);
}

QList<QDockWidget *> QMainWindow::tabifiedDockWidgets(QDockWidget *dockwidget) const
{
   QList<QDockWidget *> ret;

#if defined(QT_NO_TABBAR)
   (void) dockwidget;

#else
   const QDockAreaLayoutInfo *info = d_func()->layout->layoutState.dockAreaLayout.info(dockwidget);

   if (info && info->tabbed && info->tabBar) {
      for (int i = 0; i < info->item_list.count(); ++i) {
         const QDockAreaLayoutItem &item = info->item_list.at(i);

         if (item.widgetItem) {
            if (QDockWidget *dock = qobject_cast<QDockWidget *>(item.widgetItem->widget())) {
               if (dock != dockwidget) {
                  ret += dock;
               }
            }
         }
      }
   }
#endif

   return ret;
}

void QMainWindow::removeDockWidget(QDockWidget *dockwidget)
{
   if (dockwidget) {
      d_func()->layout->removeWidget(dockwidget);
      dockwidget->hide();
   }
}

Qt::DockWidgetArea QMainWindow::dockWidgetArea(QDockWidget *dockwidget) const
{
   return d_func()->layout->dockWidgetArea(dockwidget);
}

void QMainWindow::resizeDocks(const QList<QDockWidget *> &docks,
   const QList<int> &sizes, Qt::Orientation orientation)
{
   d_func()->layout->layoutState.dockAreaLayout.resizeDocks(docks, sizes, orientation);
   d_func()->layout->invalidate();
}

#endif // QT_NO_DOCKWIDGET

QByteArray QMainWindow::saveState(int version) const
{
   QByteArray data;
   QDataStream stream(&data, QIODevice::WriteOnly);

   stream << QMainWindowLayout::VersionMarker;
   stream << version;

   d_func()->layout->saveState(stream);

   return data;
}

bool QMainWindow::restoreState(const QByteArray &state, int version)
{
   if (state.isEmpty()) {
      return false;
   }

   QDataStream stream(state);

   int marker;
   int v;

   stream >> marker;
   stream >> v;

   if (stream.status() != QDataStream::Ok || marker != QMainWindowLayout::VersionMarker || v != version) {
      return false;
   }

   bool restored = d_func()->layout->restoreState(stream);

   return restored;
}

#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
QCursor QMainWindowPrivate::separatorCursor(const QList<int> &path) const
{
   QDockAreaLayoutInfo *info = layout->layoutState.dockAreaLayout.info(path);
   Q_ASSERT(info != nullptr);

   if (path.size() == 1) { // is this the "top-level" separator which separates a dock area
      // from the central widget?
      switch (path.first()) {
         case QInternal::LeftDock:
         case QInternal::RightDock:
            return Qt::SplitHCursor;

         case QInternal::TopDock:
         case QInternal::BottomDock:
            return Qt::SplitVCursor;

         default:
            break;
      }
   }

   // no, it's a splitter inside a dock area, separating two dock widgets

   return info->o == Qt::Horizontal
      ? Qt::SplitHCursor : Qt::SplitVCursor;
}

void QMainWindowPrivate::adjustCursor(const QPoint &pos)
{
   Q_Q(QMainWindow);

   hoverPos = pos;

   if (pos == QPoint(0, 0)) {
      if (!hoverSeparator.isEmpty()) {
         q->update(layout->layoutState.dockAreaLayout.separatorRect(hoverSeparator));
      }
      hoverSeparator.clear();

      if (cursorAdjusted) {
         cursorAdjusted = false;
         if (hasOldCursor) {
            q->setCursor(oldCursor);
         } else {
            q->unsetCursor();
         }
      }

   } else if (layout->movingSeparator.isEmpty()) {
      // Don't change cursor when moving separator
      QList<int> pathToSeparator
         = layout->layoutState.dockAreaLayout.findSeparator(pos);

      if (pathToSeparator != hoverSeparator) {
         if (!hoverSeparator.isEmpty()) {
            q->update(layout->layoutState.dockAreaLayout.separatorRect(hoverSeparator));
         }

         hoverSeparator = pathToSeparator;

         if (hoverSeparator.isEmpty()) {
            if (cursorAdjusted) {
               cursorAdjusted = false;
               if (hasOldCursor) {
                  q->setCursor(oldCursor);
               } else {
                  q->unsetCursor();
               }
            }

         } else {
            q->update(layout->layoutState.dockAreaLayout.separatorRect(hoverSeparator));
            if (!cursorAdjusted) {
               oldCursor = q->cursor();
               hasOldCursor = q->testAttribute(Qt::WA_SetCursor);
            }
            adjustedCursor = separatorCursor(hoverSeparator);
            q->setCursor(adjustedCursor);
            cursorAdjusted = true;
         }
      }
   }
}
#endif

bool QMainWindow::event(QEvent *event)
{
   Q_D(QMainWindow);
   switch (event->type()) {

#ifndef QT_NO_DOCKWIDGET
      case QEvent::Paint: {
         QPainter p(this);
         QRegion r = static_cast<QPaintEvent *>(event)->region();
         d->layout->layoutState.dockAreaLayout.paintSeparators(&p, this, r, d->hoverPos);
         break;
      }

#ifndef QT_NO_CURSOR
      case QEvent::HoverMove:  {
         d->adjustCursor(static_cast<QHoverEvent *>(event)->pos());
         break;
      }

      // do not want QWidget to call update() on the entire QMainWindow
      // on HoverEnter and HoverLeave, hence accept the event (return true).
      case QEvent::HoverEnter:
         return true;

      case QEvent::HoverLeave:
         d->adjustCursor(QPoint(0, 0));
         return true;

      case QEvent::ShortcutOverride: // when a menu pops up
         d->adjustCursor(QPoint(0, 0));
         break;
#endif

      case QEvent::MouseButtonPress: {
         QMouseEvent *e = static_cast<QMouseEvent *>(event);
         if (e->button() == Qt::LeftButton && d->layout->startSeparatorMove(e->pos())) {
            // The click was on a separator, eat this event
            e->accept();
            return true;
         }

         break;
      }

      case QEvent::MouseMove: {
         QMouseEvent *e = static_cast<QMouseEvent *>(event);

#ifndef QT_NO_CURSOR
         d->adjustCursor(e->pos());
#endif
         if (e->buttons() & Qt::LeftButton) {
            if (d->layout->separatorMove(e->pos())) {
               // We're moving a separator, eat this event
               e->accept();
               return true;
            }
         }

         break;
      }

      case QEvent::MouseButtonRelease: {
         QMouseEvent *e = static_cast<QMouseEvent *>(event);
         if (d->layout->endSeparatorMove(e->pos())) {
            // We've released a separator, eat this event
            e->accept();
            return true;
         }
         break;
      }

#endif

#ifndef QT_NO_TOOLBAR
      case QEvent::ToolBarChange: {
         d->layout->toggleToolBarsVisible();
         return true;
      }
#endif

#ifndef QT_NO_STATUSTIP
      case QEvent::StatusTip:

#ifndef QT_NO_STATUSBAR
         if (QStatusBar *sb = d->layout->statusBar()) {
            sb->showMessage(static_cast<QStatusTipEvent *>(event)->tip());
         } else
#endif
            static_cast<QStatusTipEvent *>(event)->ignore();
         return true;
#endif

      case QEvent::StyleChange:

#ifndef QT_NO_DOCKWIDGET
         d->layout->layoutState.dockAreaLayout.styleChangedEvent();
#endif
         if (!d->explicitIconSize) {
            setIconSize(QSize());
         }
         break;

#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
      case QEvent::CursorChange:
         // CursorChange events are triggered as mouse moves to new widgets even
         // if the cursor doesn't actually change, so do not change oldCursor if
         // the "changed" cursor has same shape as adjusted cursor.
         if (d->cursorAdjusted && d->adjustedCursor.shape() != cursor().shape()) {
            d->oldCursor = cursor();
            d->hasOldCursor = testAttribute(Qt::WA_SetCursor);

            // Ensure our adjusted cursor stays visible
            setCursor(d->adjustedCursor);
         }
         break;
#endif
      default:
         break;
   }

   return QWidget::event(event);
}

#ifndef QT_NO_TOOLBAR

void QMainWindow::setUnifiedTitleAndToolBarOnMac(bool set)
{
#ifdef Q_OS_DARWIN
   Q_D(QMainWindow);

   if (isWindow()) {
      d->useUnifiedToolBar = set;
      createWinId();

      QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();

      QPlatformNativeInterface::FP_Integration function =
         nativeInterface->nativeResourceFunctionForIntegration("setContentBorderEnabled");

      if (! function) {
         return;   // Not Cocoa platform plugin.
      }

      typedef void (*SetContentBorderEnabledFunction)(QWindow * window, bool enable);
      (reinterpret_cast<SetContentBorderEnabledFunction>(function))(window()->windowHandle(), set);
      update();
   }
#else
   (void) set;
#endif
}

bool QMainWindow::unifiedTitleAndToolBarOnMac() const
{
#ifdef Q_OS_DARWIN
   return d_func()->useUnifiedToolBar;
#endif

   return false;
}

#endif

bool QMainWindow::isSeparator(const QPoint &pos) const
{
#ifndef QT_NO_DOCKWIDGET
   Q_D(const QMainWindow);
   return !d->layout->layoutState.dockAreaLayout.findSeparator(pos).isEmpty();
#else
   (void) pos;
   return false;
#endif
}

#ifndef QT_NO_CONTEXTMENU

void QMainWindow::contextMenuEvent(QContextMenuEvent *event)
{
   event->ignore();
   // only show the context menu for direct QDockWidget and QToolBar
   // children and for the menu bar as well
   QWidget *child = childAt(event->pos());

   while (child && child != this) {
#ifndef QT_NO_MENUBAR
      if (QMenuBar *mb = qobject_cast<QMenuBar *>(child)) {
         if (mb->parentWidget() != this) {
            return;
         }
         break;
      }
#endif

#ifndef QT_NO_DOCKWIDGET
      if (QDockWidget *dw = qobject_cast<QDockWidget *>(child)) {
         if (dw->parentWidget() != this) {
            return;
         }
         if (dw->widget()
            && dw->widget()->geometry().contains(child->mapFrom(this, event->pos()))) {
            // ignore the event if the mouse is over the QDockWidget contents
            return;
         }
         break;
      }
#endif

#ifndef QT_NO_TOOLBAR
      if (QToolBar *tb = qobject_cast<QToolBar *>(child)) {
         if (tb->parentWidget() != this) {
            return;
         }
         break;
      }
#endif
      child = child->parentWidget();
   }

   if (child == this) {
      return;
   }

#ifndef QT_NO_MENU
   QMenu *popup = createPopupMenu();

   if (popup) {
      if (!popup->isEmpty()) {
         popup->setAttribute(Qt::WA_DeleteOnClose);
         popup->popup(event->globalPos());
         event->accept();
      } else {
         delete popup;
      }
   }
#endif
}
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_MENU

QMenu *QMainWindow::createPopupMenu()
{
   Q_D(QMainWindow);

   QMenu *menu = nullptr;

#ifndef QT_NO_DOCKWIDGET
   QList<QDockWidget *> dockwidgets = findChildren<QDockWidget *>();
   if (dockwidgets.size()) {
      menu = new QMenu(this);

      for (int i = 0; i < dockwidgets.size(); ++i) {
         QDockWidget *dockWidget = dockwidgets.at(i);
         // filter to find out if we own this QDockWidget

         if (dockWidget->parentWidget() == this) {
            if (d->layout->layoutState.dockAreaLayout.indexOf(dockWidget).isEmpty()) {
               continue;
            }

         } else if (QDockWidgetGroupWindow *dwgw =
               qobject_cast<QDockWidgetGroupWindow *>(dockWidget->parentWidget())) {
            if (dwgw->parentWidget() != this) {
               continue;
            }

            if (dwgw->layoutInfo()->indexOf(dockWidget).isEmpty()) {
               continue;
            }

         } else {
            continue;
         }

         menu->addAction(dockwidgets.at(i)->toggleViewAction());
      }

      menu->addSeparator();
   }
#endif

#ifndef QT_NO_TOOLBAR
   QList<QToolBar *> toolbars = findChildren<QToolBar *>();

   if (toolbars.size()) {
      if (!menu) {
         menu = new QMenu(this);
      }

      for (int i = 0; i < toolbars.size(); ++i) {
         QToolBar *toolBar = toolbars.at(i);

         if (toolBar->parentWidget() == this
            && (!d->layout->layoutState.toolBarAreaLayout.indexOf(toolBar).isEmpty())) {
            menu->addAction(toolbars.at(i)->toggleViewAction());
         }
      }
   }
#endif

   return menu;
}
#endif // QT_NO_MENU

#endif // QT_NO_MAINWINDOW
