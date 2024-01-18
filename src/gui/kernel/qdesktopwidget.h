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

#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#include <qwidget.h>

class QApplication;
class QDesktopWidgetPrivate;

class Q_GUI_EXPORT QDesktopWidget : public QWidget
{
   GUI_CS_OBJECT(QDesktopWidget)

   GUI_CS_PROPERTY_READ(virtualDesktop, isVirtualDesktop)

   GUI_CS_PROPERTY_READ(screenCount, screenCount)
   GUI_CS_PROPERTY_NOTIFY(screenCount, screenCountChanged)

   GUI_CS_PROPERTY_READ(primaryScreen, primaryScreen)

 public:
   QDesktopWidget();

   QDesktopWidget(const QDesktopWidget &) = delete;
   QDesktopWidget &operator=(const QDesktopWidget &) = delete;

   ~QDesktopWidget();

   bool isVirtualDesktop() const;

   int numScreens() const;
   inline int screenCount() const;
   int primaryScreen() const;

   int screenNumber(const QWidget *widget = nullptr) const;
   int screenNumber(const QPoint &point) const;

   QWidget *screen(int screen = -1);

   const QRect screenGeometry(int screen = -1) const;
   const QRect screenGeometry(const QWidget *widget) const;

   const QRect screenGeometry(const QPoint &point) const {
      return screenGeometry(screenNumber(point));
   }

   const QRect availableGeometry(int screen = -1) const;
   const QRect availableGeometry(const QWidget *widget) const;

   const QRect availableGeometry(const QPoint &point) const {
      return availableGeometry(screenNumber(point));
   }

   GUI_CS_SIGNAL_1(Public, void resized(int screen))
   GUI_CS_SIGNAL_2(resized, screen)

   GUI_CS_SIGNAL_1(Public, void workAreaResized(int screen))
   GUI_CS_SIGNAL_2(workAreaResized, screen)

   GUI_CS_SIGNAL_1(Public, void screenCountChanged(int newCount))
   GUI_CS_SIGNAL_2(screenCountChanged, newCount)

   GUI_CS_SIGNAL_1(Public, void primaryScreenChanged())
   GUI_CS_SIGNAL_2(primaryScreenChanged)

 protected:
   void resizeEvent(QResizeEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QDesktopWidget)

   GUI_CS_SLOT_1(Private, void _q_updateScreens())
   GUI_CS_SLOT_2(_q_updateScreens)

   GUI_CS_SLOT_1(Private, void _q_availableGeometryChanged())
   GUI_CS_SLOT_2(_q_availableGeometryChanged)

   friend class QApplication;
   friend class QApplicationPrivate;
};

int QDesktopWidget::screenCount() const
{
   return numScreens();
}

#endif
