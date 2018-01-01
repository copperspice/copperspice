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

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TOOLBAR

class QToolBarPrivate;
class QAction;
class QIcon;
class QMainWindow;
class QStyleOptionToolBar;

class Q_GUI_EXPORT QToolBar : public QWidget
{
   GUI_CS_OBJECT(QToolBar)

   GUI_CS_PROPERTY_READ(movable, isMovable)
   GUI_CS_PROPERTY_WRITE(movable, setMovable)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(movable, cs_isMainWindow())
   GUI_CS_PROPERTY_NOTIFY(movable, movableChanged)

   GUI_CS_PROPERTY_READ(allowedAreas, allowedAreas)
   GUI_CS_PROPERTY_WRITE(allowedAreas, setAllowedAreas)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(allowedAreas, cs_isMainWindow())
   GUI_CS_PROPERTY_NOTIFY(allowedAreas, allowedAreasChanged)

   GUI_CS_PROPERTY_READ(orientation, orientation)
   GUI_CS_PROPERTY_WRITE(orientation, setOrientation)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(orientation, cs_isMainWindow())
   GUI_CS_PROPERTY_NOTIFY(orientation, orientationChanged)

   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)
   GUI_CS_PROPERTY_NOTIFY(iconSize, iconSizeChanged)

   GUI_CS_PROPERTY_READ(toolButtonStyle, toolButtonStyle)
   GUI_CS_PROPERTY_WRITE(toolButtonStyle, setToolButtonStyle)
   GUI_CS_PROPERTY_NOTIFY(toolButtonStyle, toolButtonStyleChanged)
   GUI_CS_PROPERTY_READ(floating, isFloating)
   GUI_CS_PROPERTY_READ(floatable, isFloatable)
   GUI_CS_PROPERTY_WRITE(floatable, setFloatable)

 public:
   explicit QToolBar(const QString &title, QWidget *parent = nullptr);
   explicit QToolBar(QWidget *parent = nullptr);
   ~QToolBar();

   void setMovable(bool movable);
   bool isMovable() const;

   void setAllowedAreas(Qt::ToolBarAreas areas);
   Qt::ToolBarAreas allowedAreas() const;

   inline bool isAreaAllowed(Qt::ToolBarArea area) const {
      return (allowedAreas() & area) == area;
   }

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;

   void clear();

   using QWidget::addAction;

   QAction *addAction(const QString &text);
   QAction *addAction(const QIcon &icon, const QString &text);
   QAction *addAction(const QString &text, const QObject *receiver, const char *member);
   QAction *addAction(const QIcon &icon, const QString &text, const QObject *receiver, const char *member);

   QAction *addSeparator();
   QAction *insertSeparator(QAction *before);

   QAction *addWidget(QWidget *widget);
   QAction *insertWidget(QAction *before, QWidget *widget);

   QRect actionGeometry(QAction *action) const;
   QAction *actionAt(const QPoint &p) const;
   inline QAction *actionAt(int x, int y) const;

   QAction *toggleViewAction() const;

   QSize iconSize() const;
   Qt::ToolButtonStyle toolButtonStyle() const;

   QWidget *widgetForAction(QAction *action) const;

   bool isFloatable() const;
   void setFloatable(bool floatable);
   bool isFloating() const;

   GUI_CS_SLOT_1(Public, void setIconSize(const QSize &iconSize))
   GUI_CS_SLOT_2(setIconSize)
   GUI_CS_SLOT_1(Public, void setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle))
   GUI_CS_SLOT_2(setToolButtonStyle)

   GUI_CS_SIGNAL_1(Public, void actionTriggered(QAction *action))
   GUI_CS_SIGNAL_2(actionTriggered, action)
   GUI_CS_SIGNAL_1(Public, void movableChanged(bool movable))
   GUI_CS_SIGNAL_2(movableChanged, movable)
   GUI_CS_SIGNAL_1(Public, void allowedAreasChanged(Qt::ToolBarAreas allowedAreas))
   GUI_CS_SIGNAL_2(allowedAreasChanged, allowedAreas)
   GUI_CS_SIGNAL_1(Public, void orientationChanged(Qt::Orientation orientation))
   GUI_CS_SIGNAL_2(orientationChanged, orientation)
   GUI_CS_SIGNAL_1(Public, void iconSizeChanged(const QSize &iconSize))
   GUI_CS_SIGNAL_2(iconSizeChanged, iconSize)
   GUI_CS_SIGNAL_1(Public, void toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle))
   GUI_CS_SIGNAL_2(toolButtonStyleChanged, toolButtonStyle)
   GUI_CS_SIGNAL_1(Public, void topLevelChanged(bool topLevel))
   GUI_CS_SIGNAL_2(topLevelChanged, topLevel)
   GUI_CS_SIGNAL_1(Public, void visibilityChanged(bool visible))
   GUI_CS_SIGNAL_2(visibilityChanged, visible)

 protected:
   void actionEvent(QActionEvent *event) override;
   void changeEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   bool event(QEvent *event) override;
   void initStyleOption(QStyleOptionToolBar *option) const;

 private:
   Q_DECLARE_PRIVATE(QToolBar)
   Q_DISABLE_COPY(QToolBar)

   GUI_CS_SLOT_1(Private, void _q_toggleView(bool un_named_arg1))
   GUI_CS_SLOT_2(_q_toggleView)

   GUI_CS_SLOT_1(Private, void _q_updateIconSize(const QSize &un_named_arg1))
   GUI_CS_SLOT_2(_q_updateIconSize)

   GUI_CS_SLOT_1(Private, void _q_updateToolButtonStyle(Qt::ToolButtonStyle un_named_arg1))
   GUI_CS_SLOT_2(_q_updateToolButtonStyle)

   bool cs_isMainWindow() const;

   friend class QMainWindow;
   friend class QMainWindowLayout;
   friend class QToolBarLayout;
   friend class QToolBarAreaLayout;
};

QAction *QToolBar::actionAt(int ax, int ay) const
{
   return actionAt(QPoint(ax, ay));
}

#endif // QT_NO_TOOLBAR

QT_END_NAMESPACE

#endif // QDYNAMICTOOLBAR_H
