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

#ifndef QCOREAPPLICATION_P_H
#define QCOREAPPLICATION_P_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qtranslator.h>
#include <QtCore/qsettings.h>

QT_BEGIN_NAMESPACE

typedef QList<QTranslator *> QTranslatorList;

class QAbstractEventDispatcher;

class Q_CORE_EXPORT QCoreApplicationPrivate
{
   Q_DECLARE_PUBLIC(QCoreApplication)

 public:
   QCoreApplicationPrivate(int &aargc,  char **aargv, uint flags);
   virtual ~QCoreApplicationPrivate();

   bool sendThroughApplicationEventFilters(QObject *, QEvent *);
   bool sendThroughObjectEventFilters(QObject *, QEvent *);
   bool notify_helper(QObject *, QEvent *);

   virtual QString appName() const;
   mutable QString applicationName;

   virtual void createEventDispatcher();
   static void removePostedEvent(QEvent *);

#ifdef Q_OS_WIN
   static void removePostedTimerEvent(QObject *object, int timerId);
#endif

#ifdef Q_OS_MAC
   static QString macMenuBarName();
#endif

   static QThread *theMainThread;
   static QThread *mainThread();
   static bool checkInstance(const char *method);
   static void sendPostedEvents(QObject *receiver, int event_type, QThreadData *data);

#if !defined (QT_NO_DEBUG) || defined (QT_MAC_FRAMEWORK_BUILD)
   void checkReceiverThread(QObject *receiver);
#endif

   int &argc;
   char **argv;
   void appendApplicationPathToLibraryPaths(void);
   void processCommandLineArguments();

   static QString qmljsDebugArguments(); // access arguments from other libraries

#ifndef QT_NO_TRANSLATION
   QTranslatorList translators;
#endif

   uint application_type;

   QCoreApplication::EventFilter eventFilter;

   bool in_exec;
   bool aboutToQuitEmitted;
   QString cachedApplicationDirPath;
   QString cachedApplicationFilePath;

   static bool isTranslatorInstalled(QTranslator *translator);

   static QAbstractEventDispatcher *eventDispatcher;
   static bool is_app_running;
   static bool is_app_closing;

   static uint attribs;
   static inline bool testAttribute(uint flag) {
      return attribs & (1 << flag);
   }
   static int app_compile_version;
   static QSettings *copperspiceConf();

 protected:
   QCoreApplication *q_ptr;

};

QT_END_NAMESPACE

#endif // QCOREAPPLICATION_P_H
