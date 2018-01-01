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

#include <qmousedriverfactory_qws.h>
#include <qapplication.h>
#include <qmousepc_qws.h>
#include <qmouselinuxtp_qws.h>
#include <qmouselinuxinput_qws.h>
#include <qmousevfb_qws.h>
#include <qmousetslib_qws.h>
#include <qmouseqnx_qws.h>
#include <qmouseintegrity_qws.h>
#include <stdlib.h>
#include <qfactoryloader_p.h>
#include <qmousedriverplugin_qws.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QWSMouseHandlerFactoryInterface_iid,
                  QLatin1String("/mousedrivers"), Qt::CaseInsensitive))

#endif

/*!
    \class QMouseDriverFactory
    \ingroup qws

    \brief The QMouseDriverFactory class creates mouse drivers in
    Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    QMouseDriverFactory is used to detect and instantiate the
    available mouse drivers, allowing \l{Qt for Embedded Linux} to load the
    preferred driver into the server application at runtime. The
    create() function returns a QWSMouseHandler object representing
    the mouse driver identified by a given key. The valid keys
    (i.e. the supported drivers) can be retrieved using the keys()
    function.

    \l{Qt for Embedded Linux} provides several built-in mouse drivers. In
    addition, custom mouse drivers can be added using Qt's plugin
    mechanism, i.e. by subclassing the QWSMouseHandler class and
    creating a mouse driver plugin (QMouseDriverPlugin). See the
    \l{Qt for Embedded Linux Pointer Handling}{pointer handling}
    documentation for details.

    \sa QWSMouseHandler, QMouseDriverPlugin
*/

/*!
    Creates the mouse driver specified by the given \a key, using the
    display specified by the given \a device.

    Note that the keys are case-insensitive.

    \sa keys()
*/
QWSMouseHandler *QMouseDriverFactory::create(const QString &key, const QString &device)
{
   QString driver = key.toLower();

#ifndef QT_NO_QWS_MOUSE_LINUXTP
   if (driver == QLatin1String("linuxtp") || driver.isEmpty()) {
      return new QWSLinuxTPMouseHandler(key, device);
   }
#endif
#ifndef QT_NO_QWS_MOUSE_PC
   if (driver == QLatin1String("auto")
         || driver == QLatin1String("intellimouse")
         || driver == QLatin1String("microsoft")
         || driver == QLatin1String("mousesystems")
         || driver == QLatin1String("mouseman")
         || driver.isEmpty()) {
      return new QWSPcMouseHandler(key, device);
   }
#endif
#ifndef QT_NO_QWS_MOUSE_TSLIB
   if (driver == QLatin1String("tslib") || driver.isEmpty()) {
      return new QWSTslibMouseHandler(key, device);
   }
#endif
# ifndef QT_NO_QWS_MOUSE_LINUXINPUT
   if (driver == QLatin1String("linuxinput") || \
         driver == QLatin1String("usb") || \
         driver == QLatin1String("linuxis")) {
      return new QWSLinuxInputMouseHandler(device);
   }
# endif
#ifndef QT_NO_QWS_MOUSE_QVFB
   if (driver == QLatin1String("qvfbmouse") || driver == QLatin1String("qvfb")) {
      return new QVFbMouseHandler(key, device);
   }
#endif

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
   if (QWSMouseHandlerFactoryInterface *factory =
            qobject_cast<QWSMouseHandlerFactoryInterface *>(loader()->instance(driver))) {
      return factory->create(driver, device);
   }
#endif

   return 0;
}

/*!
    Returns the list of valid keys, i.e. the available mouse drivers.

    \sa create()
*/
QStringList QMouseDriverFactory::keys()
{
   QStringList list;

#ifndef QT_NO_QWS_MOUSE_LINUXTP
   list << QLatin1String("LinuxTP");
#endif
#ifndef QT_NO_QWS_MOUSE_PC
   list << QLatin1String("Auto")
        << QLatin1String("IntelliMouse")
        << QLatin1String("Microsoft")
        << QLatin1String("MouseSystems")
        << QLatin1String("MouseMan");
#endif
#ifndef QT_NO_QWS_MOUSE_TSLIB
   list << QLatin1String("Tslib");
#endif
#ifndef QT_NO_QWS_MOUSE_LINUXINPUT
   list << QLatin1String("LinuxInput");
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
