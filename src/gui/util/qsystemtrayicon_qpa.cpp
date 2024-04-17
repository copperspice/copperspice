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

#include <qsystemtrayicon_p.h>

#include <qapplication.h>
#include <qplatform_systemtrayicon.h>
#include <qplatform_theme.h>
#include <qstyle.h>

#include <qguiapplication_p.h>

#ifndef QT_NO_SYSTEMTRAYICON

QSystemTrayIconPrivate::QSystemTrayIconPrivate()
   : qpa_sys(QGuiApplicationPrivate::platformTheme()->createPlatformSystemTrayIcon()),
     visible(false)
{
}

QSystemTrayIconPrivate::~QSystemTrayIconPrivate()
{
   delete qpa_sys;
}

void QSystemTrayIconPrivate::install_sys()
{
   if (qpa_sys) {
      install_sys_qpa();
   }
}

void QSystemTrayIconPrivate::remove_sys()
{
   if (qpa_sys) {
      remove_sys_qpa();
   }
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
   if (qpa_sys) {
      return geometry_sys_qpa();
   } else {
      return QRect();
   }
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
   if (qpa_sys) {
      updateIcon_sys_qpa();
   }
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
   if (qpa_sys) {
      updateMenu_sys_qpa();
   }
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
   if (qpa_sys) {
      updateToolTip_sys_qpa();
   }
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
   QScopedPointer<QPlatformSystemTrayIcon> sys(QGuiApplicationPrivate::platformTheme()->createPlatformSystemTrayIcon());

   if (sys) {
      return sys->isSystemTrayAvailable();
   } else {
      return false;
   }
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
   QScopedPointer<QPlatformSystemTrayIcon> sys(QGuiApplicationPrivate::platformTheme()->createPlatformSystemTrayIcon());

   if (sys) {
      return sys->supportsMessages();
   } else {
      return false;
   }
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &title, const QString &message,
      QSystemTrayIcon::MessageIcon icon, int msecs)
{
   if (qpa_sys) {
      showMessage_sys_qpa(title, message, icon, msecs);
   }
}

#endif // QT_NO_SYSTEMTRAYICON
