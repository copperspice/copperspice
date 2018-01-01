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

#include <qkbddriverfactory_qws.h>

#ifndef QT_NO_QWS_KEYBOARD

#include <qapplication.h>
#include <qkbdtty_qws.h>
#include <qkbdlinuxinput_qws.h>
#include <qkbdum_qws.h>
#include <qkbdvfb_qws.h>
#include <qkbdqnx_qws.h>
#include <qkbdintegrity_qws.h>
#include <stdlib.h>
#include <qfactoryloader_p.h>
#include <qkbddriverplugin_qws.h>

QT_BEGIN_NAMESPACE

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QWSKeyboardHandlerFactoryInterface_iid,
         QLatin1String("/kbddrivers"), Qt::CaseInsensitive))

#endif


QWSKeyboardHandler *QKbdDriverFactory::create(const QString &key, const QString &device)
{
   QString driver = key.toLower();

#ifndef QT_NO_QWS_KEYBOARD
# ifndef QT_NO_QWS_KBD_TTY
   if (driver == QLatin1String("tty") || driver.isEmpty()) {
      return new QWSTtyKeyboardHandler(device);
   }
# endif
# ifndef QT_NO_QWS_KBD_LINUXINPUT
   if (driver == QLatin1String("linuxinput") || \
         driver == QLatin1String("usb") || \
         driver == QLatin1String("linuxis")) {
      return new QWSLinuxInputKeyboardHandler(device);
   }
# endif
# ifndef QT_NO_QWS_KBD_UM
   if (driver == QLatin1String("um") || driver == QLatin1String("qvfbkeyboard")) {
      return new QWSUmKeyboardHandler(device);
   }
# endif
# ifndef QT_NO_QWS_KBD_QVFB
   if (driver == QLatin1String("qvfbkbd")
         || driver == QLatin1String("qvfbkeyboard")
         || driver == QLatin1String("qvfb")) {
      return new QVFbKeyboardHandler(device);
   }
# endif
#endif

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
   if (QWSKeyboardHandlerFactoryInterface *factory =
            qobject_cast<QWSKeyboardHandlerFactoryInterface *>(loader()->instance(driver))) {
      return factory->create(driver, device);
   }
#endif

   return 0;
}

/*!
    Returns the list of valid keys, i.e. the available keyboard
    drivers.

    \sa create()
*/
QStringList QKbdDriverFactory::keys()
{
   QStringList list;

#ifndef QT_NO_QWS_KBD_TTY
   list << QLatin1String("TTY");
#endif
#ifndef QT_NO_QWS_KBD_LINUXINPUT
   list << QLatin1String("LinuxInput");
#endif
#ifndef QT_NO_QWS_KBD_UM
   list << QLatin1String("UM");
#endif

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
   QStringList plugins = loader()->keys();

   for (int i = 0; i < plugins.size(); ++i) {
      if (!list.contains(plugins.at(i))) {
         list += plugins.at(i);
      }
   }
#endif

   return list;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_KEYBOARD
