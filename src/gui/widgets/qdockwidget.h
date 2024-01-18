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

#ifndef QDOCKWIDGET_H
#define QDOCKWIDGET_H

#include <qwidget.h>

#ifndef QT_NO_DOCKWIDGET

class QDockAreaLayout;
class QDockWidgetPrivate;
class QMainWindow;
class QStyleOptionDockWidget;

class Q_GUI_EXPORT QDockWidget : public QWidget
{
   GUI_CS_OBJECT(QDockWidget)

   GUI_CS_ENUM(DockWidgetFeature)
   GUI_CS_FLAG(DockWidgetFeature, DockWidgetFeatures)

   GUI_CS_PROPERTY_READ(floating, isFloating)
   GUI_CS_PROPERTY_WRITE(floating, setFloating)

   GUI_CS_PROPERTY_READ(features, features)
   GUI_CS_PROPERTY_WRITE(features, setFeatures)
   GUI_CS_PROPERTY_NOTIFY(features, featuresChanged)

   GUI_CS_PROPERTY_READ(allowedAreas, allowedAreas)
   GUI_CS_PROPERTY_WRITE(allowedAreas, setAllowedAreas)
   GUI_CS_PROPERTY_NOTIFY(allowedAreas, allowedAreasChanged)

   GUI_CS_PROPERTY_READ(windowTitle, windowTitle)
   GUI_CS_PROPERTY_WRITE(windowTitle, setWindowTitle)
   GUI_CS_PROPERTY_DESIGNABLE(windowTitle, true)

 public:
   GUI_CS_REGISTER_ENUM(
      enum DockWidgetFeature {
         DockWidgetClosable         = 0x01,
         DockWidgetMovable          = 0x02,
         DockWidgetFloatable        = 0x04,
         DockWidgetVerticalTitleBar = 0x08,
         DockWidgetFeatureMask      = 0x0f,
         AllDockWidgetFeatures      = DockWidgetClosable | DockWidgetMovable | DockWidgetFloatable,
         NoDockWidgetFeatures       = 0x00,
         Reserved                   = 0xff
      };
   )
   using DockWidgetFeatures = QFlags<DockWidgetFeature>;

   explicit QDockWidget(const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   explicit QDockWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QDockWidget(const QDockWidget &) = delete;
   QDockWidget &operator=(const QDockWidget &) = delete;

   ~QDockWidget();

   QWidget *widget() const;
   void setWidget(QWidget *widget);

   void setFeatures(DockWidgetFeatures features);
   DockWidgetFeatures features() const;

   void setFloating(bool floating);
   bool isFloating() const {
      return isWindow();
   }

   void setAllowedAreas(Qt::DockWidgetAreas areas);
   Qt::DockWidgetAreas allowedAreas() const;

   void setTitleBarWidget(QWidget *widget);
   QWidget *titleBarWidget() const;

   bool isAreaAllowed(Qt::DockWidgetArea area) const {
      return (allowedAreas() & area) == area;
   }

#ifndef QT_NO_ACTION
   QAction *toggleViewAction() const;
#endif

   GUI_CS_SIGNAL_1(Public, void featuresChanged(QDockWidget::DockWidgetFeatures features))
   GUI_CS_SIGNAL_2(featuresChanged, features)

   GUI_CS_SIGNAL_1(Public, void topLevelChanged(bool topLevel))
   GUI_CS_SIGNAL_2(topLevelChanged, topLevel)

   GUI_CS_SIGNAL_1(Public, void allowedAreasChanged(Qt::DockWidgetAreas allowedAreas))
   GUI_CS_SIGNAL_2(allowedAreasChanged, allowedAreas)

   GUI_CS_SIGNAL_1(Public, void visibilityChanged(bool visible))
   GUI_CS_SIGNAL_2(visibilityChanged, visible)

   GUI_CS_SIGNAL_1(Public, void dockLocationChanged(Qt::DockWidgetArea area))
   GUI_CS_SIGNAL_2(dockLocationChanged, area)

 protected:
   void changeEvent(QEvent *event) override;
   void closeEvent(QCloseEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   bool event(QEvent *event) override;
   void initStyleOption(QStyleOptionDockWidget *option) const;

 private:
   Q_DECLARE_PRIVATE(QDockWidget)

   GUI_CS_SLOT_1(Private, void _q_toggleView(bool isToggled))
   GUI_CS_SLOT_2(_q_toggleView)

   GUI_CS_SLOT_1(Private, void _q_toggleTopLevel())
   GUI_CS_SLOT_2(_q_toggleTopLevel)

   friend class QDockAreaLayout;
   friend class QDockWidgetItem;
   friend class QMainWindowLayout;
   friend class QDockWidgetLayout;
   friend class QDockAreaLayoutInfo;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDockWidget::DockWidgetFeatures)

#endif // QT_NO_DOCKWIDGET

#endif // QDYNAMICDOCKWIDGET_H
