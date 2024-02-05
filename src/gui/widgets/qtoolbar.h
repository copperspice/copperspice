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

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#include <qaction.h>
#include <qwidget.h>

#ifndef QT_NO_TOOLBAR

class QToolBarPrivate;
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

   QToolBar(const QToolBar &) = delete;
   QToolBar &operator=(const QToolBar &) = delete;

   ~QToolBar();

   void setMovable(bool movable);
   bool isMovable() const;

   void setAllowedAreas(Qt::ToolBarAreas areas);
   Qt::ToolBarAreas allowedAreas() const;

   bool isAreaAllowed(Qt::ToolBarArea area) const {
      return (allowedAreas() & area) == area;
   }

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;

   void clear();

   using QWidget::addAction;

   QAction *addAction(const QString &text);
   QAction *addAction(const QIcon &icon, const QString &text);
   QAction *addAction(const QString &text, const QObject *receiver, const QString &member);
   QAction *addAction(const QIcon &icon, const QString &text, const QObject *receiver, const QString &member);

   // addAction(QString): Connect to a QObject slot / functor or function pointer (with context)
   template <class Obj, typename Func1>
   typename std::enable_if < ! std::is_same<const char *, Func1>::value &&
   std::is_base_of<QObject, Obj>::value, QAction * >::type addAction(const QString &text, const Obj *object, Func1 slot) {
      QAction *result = addAction(text);
      connect(result, &QAction::triggered, object, slot);
      return result;
   }

   // addAction(QString): Connect to a functor or function pointer (without context)
   template <typename Func1>
   QAction *addAction(const QString &text, Func1 slot) {
      QAction *result = addAction(text);
      connect(result, &QAction::triggered, slot);
      return result;
   }

   // addAction(QString): Connect to a QObject slot / functor or function pointer (with context)
   template <class Obj, typename Func1>
   typename std::enable_if < ! std::is_same<const char *, Func1>::value &&
   std::is_base_of<QObject, Obj>::value, QAction * >::type addAction(const QIcon &icon, const QString &text, const Obj *object,
      Func1 slot) {

      QAction *result = addAction(icon, text);
      connect(result, &QAction::triggered, object, slot);
      return result;
   }

   // addAction(QIcon, QString): Connect to a functor or function pointer (without context)
   template <typename Func1>
   QAction *addAction(const QIcon &icon, const QString &text, Func1 slot) {
      QAction *result = addAction(icon, text);
      connect(result, &QAction::triggered, slot);
      return result;
   }

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

   GUI_CS_SLOT_1(Private, void _q_toggleView(bool isToggleView))
   GUI_CS_SLOT_2(_q_toggleView)

   GUI_CS_SLOT_1(Private, void _q_updateIconSize(const QSize &size))
   GUI_CS_SLOT_2(_q_updateIconSize)

   GUI_CS_SLOT_1(Private, void _q_updateToolButtonStyle(Qt::ToolButtonStyle style))
   GUI_CS_SLOT_2(_q_updateToolButtonStyle)

   bool cs_isMainWindow() const;

   friend class QMainWindow;
   friend class QMainWindowLayout;
   friend class QToolBarLayout;
   friend class QToolBarAreaLayout;
};

QAction *QToolBar::actionAt(int x, int y) const
{
   return actionAt(QPoint(x, y));
}

#endif // QT_NO_TOOLBAR

#endif
