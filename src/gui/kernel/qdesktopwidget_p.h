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

#ifndef QDESKTOPWIDGET_P_H
#define QDESKTOPWIDGET_P_H

#include <QDesktopWidget>
#include <qwidget_p.h>

#include <qalgorithms.h>

class QDesktopScreenWidget : public QWidget
{
   GUI_CS_OBJECT(QDesktopScreenWidget)

 public:
   explicit QDesktopScreenWidget(QScreen *screen, const QRect &geometry);

   int screenNumber() const;
   void setScreenGeometry(const QRect &geometry);

   QScreen *screen() const {
      return m_screen.data();
   }
   QRect screenGeometry() const {
      return m_geometry;
   }

 private:
   // The widget updates its screen and geometry automatically. We need to save them separately
   // to detect changes, and trigger the appropriate signals.
   const QPointer<QScreen> m_screen;
   QRect m_geometry;
};

class QDesktopWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QDesktopWidget)

 public:
   ~QDesktopWidgetPrivate() {
      qDeleteAll(screens);
   }

   void _q_updateScreens();
   void _q_availableGeometryChanged();
   QDesktopScreenWidget *widgetForScreen(QScreen *qScreen) const;

   QList<QDesktopScreenWidget *> screens;
};

#endif
