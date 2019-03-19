/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qdecorationfactory_qws.h>
#include <qdecorationplugin_qws.h>
#include <qfactoryloader_p.h>
#include <qmutex.h>
#include <qapplication.h>
#include <qdecorationdefault_qws.h>
#include <qdecorationwindows_qws.h>
#include <qdecorationstyled_qws.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QDecorationFactoryInterface_iid, QLatin1String("/decorations"), Qt::CaseInsensitive))

/*!
    \class QDecorationFactory
    \ingroup qws
    \ingroup appearance

    \brief The QDecorationFactory class creates window decorations in
    Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    QDecorationFactory is used to detect and instantiate the available
    decorations, allowing \l{Qt for Embedded Linux} to load the preferred
    decoration into the application at runtime. The create() function
    returns a QDecoration object representing the decoration
    identified by a given key. The valid keys (i.e. the supported
    decorations) can be retrieved using the keys() function.

    \l{Qt for Embedded Linux} provides three built-in decorations: \c Default,
    \c Styled and \c Windows. In addition, custom decorations can be
    added using Qt's \l {How to Create Qt Plugins}{plugin mechanism},
    i.e. by subclassing the QDecoration class and creating a mouse
    driver plugin (QDecorationPlugin).

    \sa QDecoration, QDecorationPlugin
*/

/*!
    Creates the decoration specified by the given \a key. Note that
    the keys are case-insensitive.

    \sa keys()
*/

QDecoration *QDecorationFactory::create(const QString &key)
{
   QDecoration *ret = 0;
   QString decoration = key.toLower();
#ifndef QT_NO_QWS_DECORATION_DEFAULT
   if (decoration == QLatin1String("default")) {
      ret = new QDecorationDefault;
   } else
#endif
#ifndef QT_NO_QWS_DECORATION_WINDOWS
      if (decoration == QLatin1String("windows")) {
         ret = new QDecorationWindows;
      } else
#endif
#ifndef QT_NO_QWS_DECORATION_STYLED
         if (decoration == QLatin1String("styled")) {
            ret = new QDecorationStyled;
         } else
#endif
         { } // Keep these here - they make the #ifdefery above work

   if (!ret) {
      if (QDecorationFactoryInterface *factory = qobject_cast<QDecorationFactoryInterface *>(loader()->instance(
               decoration))) {
         ret = factory->create(decoration);
      }
   }

   return ret;
}

/*!
    Returns the list of valid keys, i.e., the available decorations.

    \sa create()
*/
QStringList QDecorationFactory::keys()
{
   QStringList list;

#ifndef QT_NO_QWS_DECORATION_STYLED
   list << QLatin1String("Styled");
#endif

#ifndef QT_NO_QWS_DECORATION_DEFAULT
   list << QLatin1String("Default");
#endif

#ifndef QT_NO_QWS_DECORATION_WINDOWS
   list << QLatin1String("Windows");
#endif

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
   QStringList plugins = loader()->keys();

   for (int i = 0; i < plugins.size(); ++i) {
      if (! list.contains(plugins.at(i))) {
         list += plugins.at(i);
      }
   }
#endif

   return list;
}

QT_END_NAMESPACE
