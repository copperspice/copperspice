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

#include <qsystemtrayicon_p.h>

#ifndef QT_NO_SYSTEMTRAYICON

QT_BEGIN_NAMESPACE

void QSystemTrayIconPrivate::install_sys()
{
}

void QSystemTrayIconPrivate::remove_sys()
{
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
   return QRect();
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
   return false;
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
   return false;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &message,
      const QString &title,
      QSystemTrayIcon::MessageIcon icon,
      int msecs)
{
   Q_UNUSED(message);
   Q_UNUSED(title);
   Q_UNUSED(icon);
   Q_UNUSED(msecs);
}

QT_END_NAMESPACE

#endif // QT_NO_SYSTEMTRAYICON
