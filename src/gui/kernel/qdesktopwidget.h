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

#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

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
   ~QDesktopWidget();

   bool isVirtualDesktop() const;

   int numScreens() const;
   inline int screenCount() const;
   int primaryScreen() const;

   int screenNumber(const QWidget *widget = 0) const;
   int screenNumber(const QPoint &) const;

   QWidget *screen(int screen = -1);

   const QRect screenGeometry(int screen = -1) const;
   const QRect screenGeometry(const QWidget *widget) const;
   inline const QRect screenGeometry(const QPoint &point) const {
      return screenGeometry(screenNumber(point));
   }

   const QRect availableGeometry(int screen = -1) const;
   const QRect availableGeometry(const QWidget *widget) const;
   inline const QRect availableGeometry(const QPoint &point) const {
      return availableGeometry(screenNumber(point));
   }

   GUI_CS_SIGNAL_1(Public, void resized(int un_named_arg1))
   GUI_CS_SIGNAL_2(resized, un_named_arg1)
   GUI_CS_SIGNAL_1(Public, void workAreaResized(int un_named_arg1))
   GUI_CS_SIGNAL_2(workAreaResized, un_named_arg1)
   GUI_CS_SIGNAL_1(Public, void screenCountChanged(int un_named_arg1))
   GUI_CS_SIGNAL_2(screenCountChanged, un_named_arg1)

 protected:
   void resizeEvent(QResizeEvent *e) override;

 private:
   Q_DISABLE_COPY(QDesktopWidget)
   Q_DECLARE_PRIVATE(QDesktopWidget)

   friend class QApplication;
   friend class QApplicationPrivate;
};

int QDesktopWidget::screenCount() const
{
   return numScreens();
}

QT_END_NAMESPACE

#endif // QDESKTOPWIDGET_H
