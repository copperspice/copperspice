/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

//#define QT_EXPERIMENTAL_CLIENT_DECORATIONS

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

#ifdef Q_OS_MAC
#include <qt_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
QT_BEGIN_NAMESPACE

extern OSWindowRef qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
QT_END_NAMESPACE
#endif

QT_BEGIN_NAMESPACE

class QMainWindowPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QMainWindow)

 public:
   inline QMainWindowPrivate()
      : layout(0), explicitIconSize(false), toolButtonStyle(Qt::ToolButtonIconOnly)
#ifdef Q_OS_MAC
      , useHIToolBar(false)
#endif
#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
      , hasOldCursor(false) , cursorAdjusted(false)
#endif
   { }

   QMainWindowLayout *layout;
   QSize iconSize;
   bool explicitIconSize;
   Qt::ToolButtonStyle toolButtonStyle;

#ifdef Q_OS_MAC
   bool useHIToolBar;
#endif

   void init();
   QList<int> hoverSeparator;
   QPoint hoverPos;

#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
   QCursor separatorCursor(const QList<int> &path) const;
   void adjustCursor(const QPoint &pos);
   QCursor oldCursor;
   uint hasOldCursor : 1;
   uint cursorAdjusted : 1;
#endif

   static inline QMainWindowLayout *mainWindowLayout(const QMainWindow *mainWindow) {
      return mainWindow ? mainWindow->d_func()->layout : static_cast<QMainWindowLayout *>(0);
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
   layout = new QMainWindowLayout(q, 0);
#endif

   const int metric = q->style()->pixelMetric(QStyle::PM_ToolBarIconSize, 0, q);
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
      const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize, 0, this);
      sz = QSize(metric, metric);
   }
   if (d->iconSize != sz) {
      d->iconSize = sz;
      emit iconSizeChanged(d->iconSize);
   }
   d->explicitIconSize = iconSize.isValid();
}

/*! \property QMainWindow::toolButtonStyle
    \brief style of toolbar buttons in this mainwindow.

    The default is Qt::ToolButtonIconOnly.
*/

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
/*!
    Returns the menu bar for the main window. This function creates
    and returns an empty menu bar if the menu bar does not exist.

    If you want all windows in a Mac application to share one menu
    bar, don't use this function to create it, because the menu bar
    created here will have this QMainWindow as its parent.  Instead,
    you must create a menu bar that does not have a parent, which you
    can then share among all the Mac windows. Create a parent-less
    menu bar this way:

    \snippet doc/src/snippets/code/src_gui_widgets_qmenubar.cpp 1

    \sa setMenuBar()
*/
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

/*!
    Sets the menu bar for the main window to \a menuBar.

    Note: QMainWindow takes ownership of the \a menuBar pointer and
    deletes it at the appropriate time.

    \sa menuBar()
*/
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

/*!
    \since 4.2

    Returns the menu bar for the main window. This function returns
    null if a menu bar hasn't been constructed yet.
*/
QWidget *QMainWindow::menuWidget() const
{
   QWidget *menuBar = d_func()->layout->menuBar();
   return menuBar;
}

/*!
    \since 4.2

    Sets the menu bar for the main window to \a menuBar.

    QMainWindow takes ownership of the \a menuBar pointer and
    deletes it at the appropriate time.
*/
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
/*!
    Returns the status bar for the main window. This function creates
    and returns an empty status bar if the status bar does not exist.

    \sa setStatusBar()
*/
QStatusBar *QMainWindow::statusBar() const
{
   QStatusBar *statusbar = d_func()->layout->statusBar();
   if (!statusbar) {
      QMainWindow *self = const_cast<QMainWindow *>(this);
      statusbar = new QStatusBar(self);
      statusbar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      self->setStatusBar(statusbar);
   }
   return statusbar;
}

/*!
    Sets the status bar for the main window to \a statusbar.

    Setting the status bar to 0 will remove it from the main window.
    Note that QMainWindow takes ownership of the \a statusbar pointer
    and deletes it at the appropriate time.

    \sa statusBar()
*/
void QMainWindow::setStatusBar(QStatusBar *statusbar)
{
   Q_D(QMainWindow);
   if (d->layout->statusBar() && d->layout->statusBar() != statusbar) {
      d->layout->statusBar()->hide();
      d->layout->statusBar()->deleteLater();
   }
   d->layout->setStatusBar(statusbar);
}
#endif // QT_NO_STATUSBAR

/*!
    Returns the central widget for the main window. This function
    returns zero if the central widget has not been set.

    \sa setCentralWidget()
*/
QWidget *QMainWindow::centralWidget() const
{
   return d_func()->layout->centralWidget();
}

/*!
    Sets the given \a widget to be the main window's central widget.

    Note: QMainWindow takes ownership of the \a widget pointer and
    deletes it at the appropriate time.

    \sa centralWidget()
*/
void QMainWindow::setCentralWidget(QWidget *widget)
{
   Q_D(QMainWindow);
   if (d->layout->centralWidget() && d->layout->centralWidget() != widget) {
      d->layout->centralWidget()->hide();
      d->layout->centralWidget()->deleteLater();
   }
   d->layout->setCentralWidget(widget);
}

#ifndef QT_NO_DOCKWIDGET
/*!
    Sets the given dock widget \a area to occupy the specified \a
    corner.

    \sa corner()
*/
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
   if (!valid) {
      qWarning("QMainWindow::setCorner(): 'area' is not valid for 'corner'");
   } else {
      d_func()->layout->setCorner(corner, area);
   }
}

/*!
    Returns the dock widget area that occupies the specified \a
    corner.

    \sa setCorner()
*/
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
   qWarning("%s: invalid 'area' argument", where);
   return false;
}

/*!
    Adds a toolbar break to the given \a area after all the other
    objects that are present.
*/
void QMainWindow::addToolBarBreak(Qt::ToolBarArea area)
{
   if (!checkToolBarArea(area, "QMainWindow::addToolBarBreak")) {
      return;
   }
   d_func()->layout->addToolBarBreak(area);
}

/*!
    Inserts a toolbar break before the toolbar specified by \a before.
*/
void QMainWindow::insertToolBarBreak(QToolBar *before)
{
   d_func()->layout->insertToolBarBreak(before);
}

/*!
    Removes a toolbar break previously inserted before the toolbar specified by \a before.
*/

void QMainWindow::removeToolBarBreak(QToolBar *before)
{
   Q_D(QMainWindow);
   d->layout->removeToolBarBreak(before);
}

/*!
    Adds the \a toolbar into the specified \a area in this main
    window. The \a toolbar is placed at the end of the current tool
    bar block (i.e. line). If the main window already manages \a toolbar
    then it will only move the toolbar to \a area.

    \sa insertToolBar() addToolBarBreak() insertToolBarBreak()
*/
void QMainWindow::addToolBar(Qt::ToolBarArea area, QToolBar *toolbar)
{
   if (!checkToolBarArea(area, "QMainWindow::addToolBar")) {
      return;
   }

   Q_D(QMainWindow);

   disconnect(this, SIGNAL(iconSizeChanged(const QSize &)), toolbar, SLOT(_q_updateIconSize(const QSize &)));
   disconnect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)), toolbar,
              SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

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

   if (!d->layout->usesHIToolBar(toolbar)) {
      d->layout->removeWidget(toolbar);
   } else {
      d->layout->removeToolBar(toolbar);
   }

   toolbar->d_func()->_q_updateIconSize(d->iconSize);
   toolbar->d_func()->_q_updateToolButtonStyle(d->toolButtonStyle);
   connect(this, SIGNAL(iconSizeChanged(const QSize &)), toolbar, SLOT(_q_updateIconSize(const QSize &)));

   connect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
           toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

   d->layout->addToolBar(area, toolbar);
}

/*! \overload
    Equivalent of calling addToolBar(Qt::TopToolBarArea, \a toolbar)
*/
void QMainWindow::addToolBar(QToolBar *toolbar)
{
   addToolBar(Qt::TopToolBarArea, toolbar);
}

/*!
    \overload

    Creates a QToolBar object, setting its window title to \a title,
    and inserts it into the top toolbar area.

    \sa setWindowTitle()
*/
QToolBar *QMainWindow::addToolBar(const QString &title)
{
   QToolBar *toolBar = new QToolBar(this);
   toolBar->setWindowTitle(title);
   addToolBar(toolBar);
   return toolBar;
}

/*!
    Inserts the \a toolbar into the area occupied by the \a before toolbar
    so that it appears before it. For example, in normal left-to-right
    layout operation, this means that \a toolbar will appear to the left
    of the toolbar specified by \a before in a horizontal toolbar area.

    \sa insertToolBarBreak() addToolBar() addToolBarBreak()
*/
void QMainWindow::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
   Q_D(QMainWindow);

   d->layout->removeToolBar(toolbar);

   toolbar->d_func()->_q_updateIconSize(d->iconSize);
   toolbar->d_func()->_q_updateToolButtonStyle(d->toolButtonStyle);
   connect(this, SIGNAL(iconSizeChanged(const QSize &)), toolbar, SLOT(_q_updateIconSize(const QSize &)));

   connect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
           toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

   d->layout->insertToolBar(before, toolbar);
}

/*!
    Removes the \a toolbar from the main window layout and hides
    it. Note that the \a toolbar is \e not deleted.
*/
void QMainWindow::removeToolBar(QToolBar *toolbar)
{
   if (toolbar) {
      d_func()->layout->removeToolBar(toolbar);
      toolbar->hide();
   }
}

/*!
    Returns the Qt::ToolBarArea for \a toolbar. If \a toolbar has not
    been added to the main window, this function returns \c
    Qt::NoToolBarArea.

    \sa addToolBar() addToolBarBreak() Qt::ToolBarArea
*/
Qt::ToolBarArea QMainWindow::toolBarArea(QToolBar *toolbar) const
{
   return d_func()->layout->toolBarArea(toolbar);
}

/*!

    Returns whether there is a toolbar
    break before the \a toolbar.

    \sa  addToolBarBreak(), insertToolBarBreak()
*/
bool QMainWindow::toolBarBreak(QToolBar *toolbar) const
{
   return d_func()->layout->toolBarBreak(toolbar);
}

#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET

/*! \property QMainWindow::animated
    \brief whether manipulating dock widgets and tool bars is animated
    \since 4.2

    When a dock widget or tool bar is dragged over the
    main window, the main window adjusts its contents
    to indicate where the dock widget or tool bar will
    be docked if it is dropped. Setting this property
    causes QMainWindow to move its contents in a smooth
    animation. Clearing this property causes the contents
    to snap into their new positions.

    By default, this property is set. It may be cleared if
    the main window contains widgets which are slow at resizing
    or repainting themselves.

    Setting this property is identical to setting the AnimatedDocks
    option using setDockOptions().
*/

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

/*! \property QMainWindow::dockNestingEnabled
    \brief whether docks can be nested
    \since 4.2

    If this property is false, dock areas can only contain a single row
    (horizontal or vertical) of dock widgets. If this property is true,
    the area occupied by a dock widget can be split in either direction to contain
    more dock widgets.

    Dock nesting is only necessary in applications that contain a lot of
    dock widgets. It gives the user greater freedom in organizing their
    main window. However, dock nesting leads to more complex
    (and less intuitive) behavior when a dock widget is dragged over the
    main window, since there are more ways in which a dropped dock widget
    may be placed in the dock area.

    Setting this property is identical to setting the AllowNestedDocks option
    using setDockOptions().
*/

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
/*!
    \property QMainWindow::documentMode
    \brief whether the tab bar for tabbed dockwidgets is set to document mode.
    \since 4.5

    The default is false.

    \sa QTabBar::documentMode
*/
bool QMainWindow::documentMode() const
{
   return d_func()->layout->documentMode();
}

void QMainWindow::setDocumentMode(bool enabled)
{
   d_func()->layout->setDocumentMode(enabled);
}
#endif // QT_NO_TABBAR

#ifndef QT_NO_TABWIDGET
/*!
    \property QMainWindow::tabShape
    \brief the tab shape used for tabbed dock widgets.
    \since 4.5

    The default is \l QTabWidget::Rounded.

    \sa setTabPosition()
*/
QTabWidget::TabShape QMainWindow::tabShape() const
{
   return d_func()->layout->tabShape();
}

void QMainWindow::setTabShape(QTabWidget::TabShape tabShape)
{
   d_func()->layout->setTabShape(tabShape);
}

/*!
    \since 4.5

    Returns the tab position for \a area.

    \note The \l VerticalTabs dock option overrides the tab positions returned
    by this function.

    \sa setTabPosition(), tabShape()
*/
QTabWidget::TabPosition QMainWindow::tabPosition(Qt::DockWidgetArea area) const
{
   if (!checkDockWidgetArea(area, "QMainWindow::tabPosition")) {
      return QTabWidget::South;
   }
   return d_func()->layout->tabPosition(area);
}

/*!
    \since 4.5

    Sets the tab position for the given dock widget \a areas to the specified
    \a tabPosition. By default, all dock areas show their tabs at the bottom.

    \note The \l VerticalTabs dock option overrides the tab positions set by
    this method.

    \sa tabPosition(), setTabShape()
*/
void QMainWindow::setTabPosition(Qt::DockWidgetAreas areas, QTabWidget::TabPosition tabPosition)
{
   d_func()->layout->setTabPosition(areas, tabPosition);
}
#endif // QT_NO_TABWIDGET

/*!
    Adds the given \a dockwidget to the specified \a area.
*/
void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
   if (!checkDockWidgetArea(area, "QMainWindow::addDockWidget")) {
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

#ifdef Q_OS_MAC     //drawer support
   QMacCocoaAutoReleasePool pool;
   extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
   if (qt_mac_is_macdrawer(dockwidget)) {
      extern bool qt_mac_set_drawer_preferred_edge(QWidget *, Qt::DockWidgetArea); //qwidget_mac.cpp
      window()->createWinId();
      dockwidget->window()->createWinId();
      qt_mac_set_drawer_preferred_edge(dockwidget, area);
      if (dockwidget->isVisible()) {
         dockwidget->hide();
         dockwidget->show();
      }
   }
#endif
}

/*!
    Restores the state of \a dockwidget if it is created after the call
    to restoreState(). Returns true if the state was restored; otherwise
    returns false.

    \sa restoreState(), saveState()
*/

bool QMainWindow::restoreDockWidget(QDockWidget *dockwidget)
{
   return d_func()->layout->restoreDockWidget(dockwidget);
}

/*!
    Adds \a dockwidget into the given \a area in the direction
    specified by the \a orientation.
*/
void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget,
                                Qt::Orientation orientation)
{
   if (!checkDockWidgetArea(area, "QMainWindow::addDockWidget")) {
      return;
   }

   // add a window to an area, placing done relative to the previous
   d_func()->layout->addDockWidget(area, dockwidget, orientation);
}

/*!
    \fn void QMainWindow::splitDockWidget(QDockWidget *first, QDockWidget *second, Qt::Orientation orientation)

    Splits the space covered by the \a first dock widget into two parts,
    moves the \a first dock widget into the first part, and moves the
    \a second dock widget into the second part.

    The \a orientation specifies how the space is divided: A Qt::Horizontal
    split places the second dock widget to the right of the first; a
    Qt::Vertical split places the second dock widget below the first.

    \e Note: if \a first is currently in a tabbed docked area, \a second will
    be added as a new tab, not as a neighbor of \a first. This is because a
    single tab can contain only one dock widget.

    \e Note: The Qt::LayoutDirection influences the order of the dock widgets
    in the two parts of the divided area. When right-to-left layout direction
    is enabled, the placing of the dock widgets will be reversed.

    \sa tabifyDockWidget(), addDockWidget(), removeDockWidget()
*/
void QMainWindow::splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
                                  Qt::Orientation orientation)
{
   d_func()->layout->splitDockWidget(after, dockwidget, orientation);
}

/*!
    \fn void QMainWindow::tabifyDockWidget(QDockWidget *first, QDockWidget *second)

    Moves \a second dock widget on top of \a first dock widget, creating a tabbed
    docked area in the main window.

    \sa tabifiedDockWidgets()
*/
void QMainWindow::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
   d_func()->layout->tabifyDockWidget(first, second);
}


/*!
    \fn QList<QDockWidget*> QMainWindow::tabifiedDockWidgets(QDockWidget *dockwidget) const

    Returns the dock widgets that are tabified together with \a dockwidget.

    \since 4.5
    \sa tabifyDockWidget()
*/

QList<QDockWidget *> QMainWindow::tabifiedDockWidgets(QDockWidget *dockwidget) const
{
   QList<QDockWidget *> ret;
#if defined(QT_NO_TABBAR)
   Q_UNUSED(dockwidget);
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


/*!
    Removes the \a dockwidget from the main window layout and hides
    it. Note that the \a dockwidget is \e not deleted.
*/
void QMainWindow::removeDockWidget(QDockWidget *dockwidget)
{
   if (dockwidget) {
      d_func()->layout->removeWidget(dockwidget);
      dockwidget->hide();
   }
}

/*!
    Returns the Qt::DockWidgetArea for \a dockwidget. If \a dockwidget
    has not been added to the main window, this function returns \c
    Qt::NoDockWidgetArea.

    \sa addDockWidget() splitDockWidget() Qt::DockWidgetArea
*/
Qt::DockWidgetArea QMainWindow::dockWidgetArea(QDockWidget *dockwidget) const
{
   return d_func()->layout->dockWidgetArea(dockwidget);
}

#endif // QT_NO_DOCKWIDGET

/*!
    Saves the current state of this mainwindow's toolbars and
    dockwidgets. The \a version number is stored as part of the data.

    The \link QObject::objectName objectName\endlink property is used
    to identify each QToolBar and QDockWidget.  You should make sure
    that this property is unique for each QToolBar and QDockWidget you
    add to the QMainWindow

    To restore the saved state, pass the return value and \a version
    number to restoreState().

    To save the geometry when the window closes, you can
    implement a close event like this:

    \snippet doc/src/snippets/code/src_gui_widgets_qmainwindow.cpp 0

    \sa restoreState(), QWidget::saveGeometry(), QWidget::restoreGeometry()
*/
QByteArray QMainWindow::saveState(int version) const
{
   QByteArray data;
   QDataStream stream(&data, QIODevice::WriteOnly);
   stream << QMainWindowLayout::VersionMarker;
   stream << version;
   d_func()->layout->saveState(stream);
   return data;
}

/*!
    Restores the \a state of this mainwindow's toolbars and
    dockwidgets. The \a version number is compared with that stored
    in \a state. If they do not match, the mainwindow's state is left
    unchanged, and this function returns \c false; otherwise, the state
    is restored, and this function returns \c true.

    To restore geometry saved using QSettings, you can use code like
    this:

    \snippet doc/src/snippets/code/src_gui_widgets_qmainwindow.cpp 1

    \sa saveState(), QWidget::saveGeometry(),
    QWidget::restoreGeometry(), restoreDockWidget()
*/
bool QMainWindow::restoreState(const QByteArray &state, int version)
{
   if (state.isEmpty()) {
      return false;
   }
   QByteArray sd = state;
   QDataStream stream(&sd, QIODevice::ReadOnly);
   int marker, v;
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
   Q_ASSERT(info != 0);
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
   } else {
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
            QCursor cursor = separatorCursor(hoverSeparator);
            cursorAdjusted = false; //to not reset the oldCursor in event(CursorChange)
            q->setCursor(cursor);
            cursorAdjusted = true;
         }
      }
   }
}
#endif

/*! \reimp */
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

      // We don't want QWidget to call update() on the entire QMainWindow
      // on HoverEnter and HoverLeave, hence accept the event (return true).
      case QEvent::HoverEnter:
         return true;
      case QEvent::HoverLeave:
         d->adjustCursor(QPoint(0, 0));
         return true;
      case QEvent::ShortcutOverride: // when a menu pops up
         d->adjustCursor(QPoint(0, 0));
         break;
#endif // QT_NO_CURSOR

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

#ifdef Q_OS_MAC
      case QEvent::Show:
         d->layout->blockVisiblityCheck = false;
         if (unifiedTitleAndToolBarOnMac()) {
            d->layout->syncUnifiedToolbarVisibility();
         }
         break;
      case QEvent::WindowStateChange: {
         if (isHidden()) {
            // We are coming out of a minimize, leave things as is.
            d->layout->blockVisiblityCheck = true;
         }

         // We need to update the HIToolbar status when we go out of or into fullscreen.
         QWindowStateChangeEvent *wce = static_cast<QWindowStateChangeEvent *>(event);
         if ((windowState() & Qt::WindowFullScreen) || (wce->oldState() & Qt::WindowFullScreen)) {
            d->layout->updateHIToolBarStatus();
         }
      }

      break;
#endif
#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
      case QEvent::CursorChange:
         if (d->cursorAdjusted) {
            d->oldCursor = cursor();
            d->hasOldCursor = testAttribute(Qt::WA_SetCursor);
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
#ifdef Q_OS_MAC
   Q_D(QMainWindow);
   if (!isWindow() || d->useHIToolBar == set || QSysInfo::MacintoshVersion < QSysInfo::MV_10_3) {
      return;
   }

   d->useHIToolBar = set;
   createWinId(); // We need the hiview for down below.


   // Activate the unified toolbar with the raster engine.
   if (windowSurface() && set) {
      d->layout->unifiedSurface = new QUnifiedToolbarSurface(this);
   }

   d->layout->updateHIToolBarStatus();

   // Deactivate the unified toolbar with the raster engine.
   if (windowSurface() && !set) {
      if (d->layout->unifiedSurface) {
         delete d->layout->unifiedSurface;
         d->layout->unifiedSurface = 0;
      }
   }

   // Enabling the unified toolbar clears the opaque size grip setting, update it.
   d->macUpdateOpaqueSizeGrip();
#else
   Q_UNUSED(set)

#endif
}

bool QMainWindow::unifiedTitleAndToolBarOnMac() const
{
#ifdef Q_OS_MAC
   return d_func()->useHIToolBar && !testAttribute(Qt::WA_MacBrushedMetal) && !(windowFlags() & Qt::FramelessWindowHint);
#endif
   return false;
}

#endif

/*!
    \internal
*/
bool QMainWindow::isSeparator(const QPoint &pos) const
{
#ifndef QT_NO_DOCKWIDGET
   Q_D(const QMainWindow);
   return !d->layout->layoutState.dockAreaLayout.findSeparator(pos).isEmpty();
#else
   Q_UNUSED(pos);
   return false;
#endif
}

#ifndef QT_NO_CONTEXTMENU
/*!
    \reimp
*/
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
#endif // QT_NO_DOCKWIDGET
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
/*!
    Returns a popup menu containing checkable entries for the toolbars and
    dock widgets present in the main window. If  there are no toolbars and
    dock widgets present, this function returns a null pointer.

    By default, this function is called by the main window when the user
    activates a context menu, typically by right-clicking on a toolbar or a dock
    widget.

    If you want to create a custom popup menu, reimplement this function and
    return a newly-created popup menu. Ownership of the popup menu is transferred
    to the caller.

    \sa addDockWidget(), addToolBar(), menuBar()
*/
QMenu *QMainWindow::createPopupMenu()
{
   Q_D(QMainWindow);
   QMenu *menu = 0;
#ifndef QT_NO_DOCKWIDGET
   QList<QDockWidget *> dockwidgets = findChildren<QDockWidget *>();
   if (dockwidgets.size()) {
      menu = new QMenu(this);
      for (int i = 0; i < dockwidgets.size(); ++i) {
         QDockWidget *dockWidget = dockwidgets.at(i);
         if (dockWidget->parentWidget() == this
               && !d->layout->layoutState.dockAreaLayout.indexOf(dockWidget).isEmpty()) {
            menu->addAction(dockwidgets.at(i)->toggleViewAction());
         }
      }
      menu->addSeparator();
   }
#endif // QT_NO_DOCKWIDGET
#ifndef QT_NO_TOOLBAR
   QList<QToolBar *> toolbars = findChildren<QToolBar *>();
   if (toolbars.size()) {
      if (!menu) {
         menu = new QMenu(this);
      }
      for (int i = 0; i < toolbars.size(); ++i) {
         QToolBar *toolBar = toolbars.at(i);
         if (toolBar->parentWidget() == this
               && (!d->layout->layoutState.toolBarAreaLayout.indexOf(toolBar).isEmpty()
                   || (unifiedTitleAndToolBarOnMac()
                       && toolBarArea(toolBar) == Qt::TopToolBarArea))) {
            menu->addAction(toolbars.at(i)->toggleViewAction());
         }
      }
   }
#endif
   Q_UNUSED(d);
   return menu;
}
#endif // QT_NO_MENU

QT_END_NAMESPACE

#endif // QT_NO_MAINWINDOW
