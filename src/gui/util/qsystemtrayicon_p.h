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

#ifndef QSYSTEMTRAYICON_P_H
#define QSYSTEMTRAYICON_P_H

#include <qsystemtrayicon.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QtGui/qmenu.h>
#include <QtGui/qpixmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QSystemTrayIconSys;
class QToolButton;
class QLabel;

class QSystemTrayIconPrivate
{
   Q_DECLARE_PUBLIC(QSystemTrayIcon)

 public:
   QSystemTrayIconPrivate() : sys(0), visible(false) { }
   virtual ~QSystemTrayIconPrivate() {}

   void install_sys();
   void remove_sys();
   void updateIcon_sys();
   void updateToolTip_sys();
   void updateMenu_sys();
   QRect geometry_sys() const;
   void showMessage_sys(const QString &msg, const QString &title, QSystemTrayIcon::MessageIcon icon, int secs);

   static bool isSystemTrayAvailable_sys();
   static bool supportsMessages_sys();

   QPointer<QMenu> menu;
   QIcon icon;
   QString toolTip;
   QSystemTrayIconSys *sys;
   bool visible;

 protected:
   QSystemTrayIcon *q_ptr;
};

class QBalloonTip : public QWidget
{
   GUI_CS_OBJECT(QBalloonTip)

 public:
   static void showBalloon(QSystemTrayIcon::MessageIcon icon, const QString &title,
                           const QString &msg, QSystemTrayIcon *trayIcon,
                           const QPoint &pos, int timeout, bool showArrow = true);

   static void hideBalloon();
   static bool isBalloonVisible();

 private:
   QBalloonTip(QSystemTrayIcon::MessageIcon icon, const QString &title,
               const QString &msg, QSystemTrayIcon *trayIcon);

   ~QBalloonTip();
   void balloon(const QPoint &, int, bool);

 protected:
   void paintEvent(QPaintEvent *) override;
   void resizeEvent(QResizeEvent *) override;
   void mousePressEvent(QMouseEvent *e) override;
   void timerEvent(QTimerEvent *e) override;

 private:
   QSystemTrayIcon *trayIcon;
   QPixmap pixmap;
   int timerId;
};

#if defined(Q_WS_X11)
QT_BEGIN_INCLUDE_NAMESPACE
#include <QtCore/qcoreapplication.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
QT_END_INCLUDE_NAMESPACE

class QSystemTrayIconSys : public QWidget
{
   friend class QSystemTrayIconPrivate;

 public:
   QSystemTrayIconSys(QSystemTrayIcon *q);
   ~QSystemTrayIconSys();
   enum {
      SYSTEM_TRAY_REQUEST_DOCK = 0,
      SYSTEM_TRAY_BEGIN_MESSAGE = 1,
      SYSTEM_TRAY_CANCEL_MESSAGE = 2
   };

   void addToTray();
   void updateIcon();
   XVisualInfo *getSysTrayVisualInfo();

   // QObject::event is public but QWidget's ::event() re-implementation
   // is protected ;(
   inline bool deliverToolTipEvent(QEvent *e) {
      return QWidget::event(e);
   }

   static Window sysTrayWindow;
   static QList<QSystemTrayIconSys *> trayIcons;
   static QCoreApplication::EventFilter oldEventFilter;
   static bool sysTrayTracker(void *message, long *result);
   static Window locateSystemTray();
   static Atom sysTraySelection;
   static XVisualInfo sysTrayVisual;

 protected:
   void paintEvent(QPaintEvent *pe) override;
   void resizeEvent(QResizeEvent *re) override;
   bool x11Event(XEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   bool event(QEvent *e) override;

 private:
   QPixmap background;
   QSystemTrayIcon *q;
   Colormap colormap;
};
#endif // Q_WS_X11

QT_END_NAMESPACE

#endif // QT_NO_SYSTEMTRAYICON

#endif // QSYSTEMTRAYICON_P_H

