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

#ifndef QCOREAPPLICATION_P_H
#define QCOREAPPLICATION_P_H

#include <qcoreapplication.h>
#include <qtranslator.h>
#include <qsettings.h>

using QTranslatorList = QList<QTranslator *>;

class QAbstractEventDispatcher;

class Q_CORE_EXPORT QCoreApplicationPrivate
{
   Q_DECLARE_PUBLIC(QCoreApplication)

 public:
   enum Type {
      Tty,
      Gui
   };

   QCoreApplicationPrivate(int &aargc,  char **aargv, uint flags);
   virtual ~QCoreApplicationPrivate();

   void init();
   static void initLocale();
   bool sendThroughApplicationEventFilters(QObject *, QEvent *);
   bool sendThroughObjectEventFilters(QObject *, QEvent *);
   bool notify_helper(QObject *, QEvent *);

   static void setEventSpontaneous(QEvent *e, bool spontaneous) {
      e->spont = spontaneous;
   }

   virtual QString appName() const;
   mutable QString applicationName;

   virtual void createEventDispatcher();
   virtual void eventDispatcherReady();
   static void removePostedEvent(QEvent *);

#ifdef Q_OS_WIN
   static void removePostedTimerEvent(QObject *object, int timerId);
#endif

#ifdef Q_OS_DARWIN
   static QString macMenuBarName();
#endif

   QAtomicInt quitLockRef;

   virtual bool shouldQuit() {
      return true;
   }

   void maybeQuit();

   static QThread *mainThread();

   static bool checkInstance(const char *method);
   static void sendPostedEvents(QObject *receiver, int event_type, QThreadData *data);

   void checkReceiverThread(QObject *receiver);

   QThreadData *getThreadData() {
      return CSInternalThreadData::get_m_ThreadData(q_ptr);
   }

   int &argc;
   char **argv;
   void appendApplicationPathToLibraryPaths(void);
   void processCommandLineArguments();

   static QString qmljsDebugArguments();          // access arguments from other libraries

   QTranslatorList translators;
   static bool isTranslatorInstalled(QTranslator *translator);

   QCoreApplicationPrivate::Type application_type;

   bool in_exec;
   bool aboutToQuitEmitted;

   QString cachedApplicationDirPath;
   QString cachedApplicationFilePath;

   static QThread *theMainThread;
   static QAbstractEventDispatcher *eventDispatcher;  // points to the platform dispatcher
   static bool is_app_running;
   static bool is_app_closing;

   static bool setuidAllowed;
   static uint attribs;

   static bool testAttribute(uint flag) {
      return attribs & (1 << flag);
   }

   static QSettings *copperspiceConf();

 protected:
   QCoreApplication *q_ptr;

};

#endif
