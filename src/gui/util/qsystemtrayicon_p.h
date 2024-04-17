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

#ifndef QSYSTEMTRAYICON_P_H
#define QSYSTEMTRAYICON_P_H

#include <qsystemtrayicon.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <qmenu.h>
#include <qpixmap.h>
#include <qplatform_systemtrayicon.h>
#include <qpointer.h>
#include <qstring.h>

class QLabel;
class QPlatformSystemTrayIcon;
class QSystemTrayIconSys;
class QToolButton;

class QSystemTrayIconPrivate
{
   Q_DECLARE_PUBLIC(QSystemTrayIcon)

 public:
   QSystemTrayIconPrivate();
   virtual ~QSystemTrayIconPrivate();

   void install_sys();
   void remove_sys();
   void updateIcon_sys();
   void updateToolTip_sys();
   void updateMenu_sys();
   QRect geometry_sys() const;
   void showMessage_sys(const QString &title, const QString &msg, QSystemTrayIcon::MessageIcon icon, int secs);

   static bool isSystemTrayAvailable_sys();
   static bool supportsMessages_sys();

   void _q_emitActivated(QPlatformSystemTrayIcon::ActivationReason reason);
   QPointer<QMenu> menu;
   QIcon icon;
   QString toolTip;
   QSystemTrayIconSys *sys;
   QPlatformSystemTrayIcon *qpa_sys;
   bool visible;

 protected:
   QSystemTrayIcon *q_ptr;

 private:
   void install_sys_qpa();
   void remove_sys_qpa();
   void updateIcon_sys_qpa();
   void updateToolTip_sys_qpa();
   void updateMenu_sys_qpa();
   QRect geometry_sys_qpa() const;
   void showMessage_sys_qpa(const QString &title, const QString &msg, QSystemTrayIcon::MessageIcon icon, int secs);
   void addPlatformMenu(QMenu *menu) const;

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
   static void updateBalloonPosition(const QPoint &pos);

 private:
   QBalloonTip(QSystemTrayIcon::MessageIcon icon, const QString &title,
         const QString &msg, QSystemTrayIcon *trayIcon);

   ~QBalloonTip();
   void balloon(const QPoint &, int, bool);

 protected:
   void paintEvent(QPaintEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void timerEvent(QTimerEvent *event) override;

 private:
   QSystemTrayIcon *trayIcon;
   QPixmap pixmap;
   int timerId;

   bool showArrow;
};

#endif // QT_NO_SYSTEMTRAYICON

#endif // QSYSTEMTRAYICON_P_H
