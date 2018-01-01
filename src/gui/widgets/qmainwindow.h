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

#ifndef QMainWindow_H
#define QMainWindow_H

#include <QtGui/qwidget.h>
#include <QtGui/qtabwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_MAINWINDOW

class QDockWidget;
class QMainWindowPrivate;
class QMenuBar;
class QStatusBar;
class QToolBar;
class QMenu;

class Q_GUI_EXPORT QMainWindow : public QWidget
{
   GUI_CS_OBJECT(QMainWindow)

   GUI_CS_ENUM(DockOption)
   GUI_CS_FLAG(DockOption, DockOptions)

   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)
   GUI_CS_PROPERTY_READ(toolButtonStyle, toolButtonStyle)
   GUI_CS_PROPERTY_WRITE(toolButtonStyle, setToolButtonStyle)

#ifndef QT_NO_DOCKWIDGET
   GUI_CS_PROPERTY_READ(animated, isAnimated)
   GUI_CS_PROPERTY_WRITE(animated, setAnimated)

#ifndef QT_NO_TABBAR
   GUI_CS_PROPERTY_READ(documentMode, documentMode)
   GUI_CS_PROPERTY_WRITE(documentMode, setDocumentMode)
#endif

#ifndef QT_NO_TABWIDGET
   GUI_CS_PROPERTY_READ(tabShape, tabShape)
   GUI_CS_PROPERTY_WRITE(tabShape, setTabShape)
#endif

   GUI_CS_PROPERTY_READ(dockNestingEnabled, isDockNestingEnabled)
   GUI_CS_PROPERTY_WRITE(dockNestingEnabled, setDockNestingEnabled)
#endif // QT_NO_DOCKWIDGET

   GUI_CS_PROPERTY_READ(dockOptions, dockOptions)
   GUI_CS_PROPERTY_WRITE(dockOptions, setDockOptions)

#ifndef QT_NO_TOOLBAR
   GUI_CS_PROPERTY_READ(unifiedTitleAndToolBarOnMac, unifiedTitleAndToolBarOnMac)
   GUI_CS_PROPERTY_WRITE(unifiedTitleAndToolBarOnMac, setUnifiedTitleAndToolBarOnMac)
#endif

 public:
   enum DockOption {
      AnimatedDocks = 0x01,
      AllowNestedDocks = 0x02,
      AllowTabbedDocks = 0x04,
      ForceTabbedDocks = 0x08,  // implies AllowTabbedDocks, !AllowNestedDocks
      VerticalTabs = 0x10       // implies AllowTabbedDocks
   };
   using DockOptions = QFlags<DockOption>;

   explicit QMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
   ~QMainWindow();

   QSize iconSize() const;
   void setIconSize(const QSize &iconSize);

   Qt::ToolButtonStyle toolButtonStyle() const;
   void setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle);

   void setDockOptions(DockOptions options);
   DockOptions dockOptions() const;

   bool isSeparator(const QPoint &pos) const;

   QWidget *centralWidget() const;
   void setCentralWidget(QWidget *widget);

#ifndef QT_NO_DOCKWIDGET
   bool isAnimated() const;
   bool isDockNestingEnabled() const;

   void setCorner(Qt::Corner corner, Qt::DockWidgetArea area);
   Qt::DockWidgetArea corner(Qt::Corner corner) const;

   void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget);
   void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget, Qt::Orientation orientation);
   void splitDockWidget(QDockWidget *after, QDockWidget *dockwidget, Qt::Orientation orientation);
   void tabifyDockWidget(QDockWidget *first, QDockWidget *second);

   QList<QDockWidget *> tabifiedDockWidgets(QDockWidget *dockwidget) const;
   void removeDockWidget(QDockWidget *dockwidget);
   bool restoreDockWidget(QDockWidget *dockwidget);

   Qt::DockWidgetArea dockWidgetArea(QDockWidget *dockwidget) const;

   GUI_CS_SLOT_1(Public, void setAnimated(bool enabled))
   GUI_CS_SLOT_2(setAnimated)
   GUI_CS_SLOT_1(Public, void setDockNestingEnabled(bool enabled))
   GUI_CS_SLOT_2(setDockNestingEnabled)
#endif

#ifndef QT_NO_TABBAR
   bool documentMode() const;
   void setDocumentMode(bool enabled);
#endif

#ifndef QT_NO_TABWIDGET
   QTabWidget::TabShape tabShape() const;
   void setTabShape(QTabWidget::TabShape tabShape);
   QTabWidget::TabPosition tabPosition(Qt::DockWidgetArea area) const;
   void setTabPosition(Qt::DockWidgetAreas areas, QTabWidget::TabPosition tabPosition);
#endif

#ifndef QT_NO_MENU
   virtual QMenu *createPopupMenu();
#endif

#ifndef QT_NO_MENUBAR
   QMenuBar *menuBar() const;
   void setMenuBar(QMenuBar *menubar);

   QWidget  *menuWidget() const;
   void setMenuWidget(QWidget *menubar);
#endif

#ifndef QT_NO_STATUSBAR
   QStatusBar *statusBar() const;
   void setStatusBar(QStatusBar *statusbar);
#endif

#ifndef QT_NO_TOOLBAR
   void addToolBarBreak(Qt::ToolBarArea area = Qt::TopToolBarArea);
   void insertToolBarBreak(QToolBar *before);

   void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar);
   void addToolBar(QToolBar *toolbar);
   QToolBar *addToolBar(const QString &title);
   void insertToolBar(QToolBar *before, QToolBar *toolbar);
   void removeToolBar(QToolBar *toolbar);
   void removeToolBarBreak(QToolBar *before);

   void setUnifiedTitleAndToolBarOnMac(bool set);
   bool unifiedTitleAndToolBarOnMac() const;

   Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;
   bool toolBarBreak(QToolBar *toolbar) const;
#endif

   QByteArray saveState(int version = 0) const;
   bool restoreState(const QByteArray &state, int version = 0);

   GUI_CS_SIGNAL_1(Public, void iconSizeChanged(const QSize &iconSize))
   GUI_CS_SIGNAL_2(iconSizeChanged, iconSize)
   GUI_CS_SIGNAL_1(Public, void toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle))
   GUI_CS_SIGNAL_2(toolButtonStyleChanged, toolButtonStyle)

 protected:
#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *event) override;
#endif

   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QMainWindow)
   Q_DISABLE_COPY(QMainWindow)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMainWindow::DockOptions)

#endif // QT_NO_MAINWINDOW

QT_END_NAMESPACE

#endif // QDYNAMICMAINWINDOW_H
