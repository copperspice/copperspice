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

#include <qinputcontextfactory.h>

#ifndef QT_NO_IM
#include <qcoreapplication.h>
#include <qinputcontext.h>
#include <qinputcontextplugin.h>

#ifdef Q_WS_X11
#include <qt_x11_p.h>
#include <qximinputcontext_p.h>
#endif

#ifdef Q_OS_WIN
#include <qwininputcontext_p.h>
#endif

#ifdef Q_OS_MAC
#include <qmacinputcontext_p.h>
#endif

#include <qfactoryloader_p.h>
#include <qmutex.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QInputContextFactoryInterface_iid, QLatin1String("/inputmethods")))

/*!
    Creates and returns a QInputContext object for the input context
    specified by \a key with the given \a parent. Keys are case
    sensitive.

    \sa keys()
*/
QInputContext *QInputContextFactory::create( const QString &key, QObject *parent )
{
   QInputContext *result = 0;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
   if (key == QLatin1String("xim")) {
      result = new QXIMInputContext;
   }
#endif
#if defined(Q_OS_WIN)
   if (key == QLatin1String("win")) {
      result = new QWinInputContext;
   }
#endif
#if defined(Q_OS_MAC)
   if (key == QLatin1String("mac")) {
      result = new QMacInputContext;
   }
#endif

   if (QInputContextFactoryInterface *factory =
            qobject_cast<QInputContextFactoryInterface *>(loader()->instance(key))) {
      result = factory->create(key);
   }

   if (result) {
      result->setParent(parent);
   }
   return result;
}


/*!
    Returns the list of keys this factory can create input contexts
    for.

    The keys are the names used, for example, to identify and specify
    input methods for the input method switching mechanism.  The names
    have to be consistent with QInputContext::identifierName(), and
    may only contain ASCII characters.

    \sa create(), displayName(), QInputContext::identifierName()
*/
QStringList QInputContextFactory::keys()
{
   QStringList result;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
   result << QLatin1String("xim");
#endif
#if defined(Q_OS_WIN) && !defined(QT_NO_XIM)
   result << QLatin1String("win");
#endif
#if defined(Q_OS_MAC)
   result << QLatin1String("mac");
#endif

   result += loader()->keys();
   return result;
}


/*!
    Returns the languages supported by the QInputContext object
    specified by \a key.

    The languages are expressed as language code (e.g. "zh_CN",
    "zh_TW", "zh_HK", "ja", "ko", ...). An input context that supports
    multiple languages can return all supported languages as a
    QStringList. The name has to be consistent with
    QInputContext::language().

    This information may be used to optimize a user interface.

    \sa keys(), QInputContext::language(), QLocale
*/
QStringList QInputContextFactory::languages( const QString &key )
{
   QStringList result;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
   if (key == QLatin1String("xim")) {
      return QStringList(QString());
   }
#endif
#if defined(Q_OS_WIN)
   if (key == QLatin1String("win")) {
      return QStringList(QString());
   }
#endif
#if defined(Q_OS_MAC)
   if (key == QLatin1String("mac")) {
      return QStringList(QString());
   }
#endif

#if defined(QT_NO_SETTINGS)
   Q_UNUSED(key);
#else
   if (QInputContextFactoryInterface *factory =
            qobject_cast<QInputContextFactoryInterface *>(loader()->instance(key))) {
      result = factory->languages(key);
   }
#endif
   return result;
}

/*!
    Returns a user friendly internationalized name of the
    QInputContext object specified by \a key. You can, for example,
    use this name in a menu.

    \sa keys(), QInputContext::identifierName()
*/
QString QInputContextFactory::displayName( const QString &key )
{
   QString result;
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
   if (key == QLatin1String("xim")) {
      return QInputContext::tr( "XIM" );
   }
#endif

#if defined(QT_NO_SETTINGS)
   Q_UNUSED(key);
#else
   if (QInputContextFactoryInterface *factory =
            qobject_cast<QInputContextFactoryInterface *>(loader()->instance(key))) {
      return factory->displayName(key);
   }
#endif
   return QString();
}

/*!
    Returns an internationalized brief description of the QInputContext
    object specified by \a key. You can, for example, use this
    description in a user interface.

    \sa keys(), displayName()
*/
QString QInputContextFactory::description( const QString &key )
{
#if defined(Q_WS_X11) && !defined(QT_NO_XIM)
   if (key == QLatin1String("xim")) {
      return QInputContext::tr( "XIM input method" );
   }
#endif
#if defined(Q_OS_WIN) && !defined(QT_NO_XIM)
   if (key == QLatin1String("win")) {
      return QInputContext::tr( "Windows input method" );
   }
#endif
#if defined(Q_OS_MAC)
   if (key == QLatin1String("mac")) {
      return QInputContext::tr( "Mac OS X input method" );
   }
#endif

#if defined(QT_NO_SETTINGS)
   Q_UNUSED(key);
#else
   if (QInputContextFactoryInterface *factory =
            qobject_cast<QInputContextFactoryInterface *>(loader()->instance(key))) {
      return factory->description(key);
   }
#endif
   return QString();
}

QT_END_NAMESPACE

#endif // QT_NO_IM
