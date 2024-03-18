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

// do not move include
#include <qobject.h>

#ifndef QCOREAPPLICATION_H
#define QCOREAPPLICATION_H

#include <qcoreevent.h>
#include <qeventloop.h>
#include <qscopedpointer.h>

#if defined(Q_OS_WIN) && ! defined(tagMSG)
using MSG = struct tagMSG;
#endif

class QCoreApplicationPrivate;
class QTextCodec;
class QTranslator;
class QPostEventList;
class QStringList;
class QAbstractEventDispatcher;
class QAbstractNativeEventFilter;

#define qApp QCoreApplication::instance()

class Q_CORE_EXPORT QCoreApplication : public QObject
{
   CORE_CS_OBJECT(QCoreApplication)

   CORE_CS_PROPERTY_READ(applicationName, cs_applicationName)
   CORE_CS_PROPERTY_WRITE(applicationName, cs_setApplicationName)

   CORE_CS_PROPERTY_READ(applicationVersion, cs_applicationVersion)
   CORE_CS_PROPERTY_WRITE(applicationVersion, cs_setApplicationVersion)

   CORE_CS_PROPERTY_READ(organizationName, cs_organizationName)
   CORE_CS_PROPERTY_WRITE(organizationName, cs_setOrganizationName)

   CORE_CS_PROPERTY_READ(organizationDomain, cs_organizationDomain)
   CORE_CS_PROPERTY_WRITE(organizationDomain, cs_setOrganizationDomain)

   CORE_CS_PROPERTY_READ(quitLockEnabled, cs_isQuitLockEnabled)
   CORE_CS_PROPERTY_WRITE(quitLockEnabled, cs_setQuitLockEnabled)

   Q_DECLARE_PRIVATE(QCoreApplication)

 public:
   static constexpr const int ApplicationFlags = CS_VERSION | 0x01000000;

   QCoreApplication(int &argc, char **argv, int = ApplicationFlags);

   QCoreApplication(const QCoreApplication &) = delete;
   QCoreApplication &operator=(const QCoreApplication &) = delete;

   ~QCoreApplication();

   static QStringList arguments();

   static void setAttribute(Qt::ApplicationAttribute attribute, bool on = true);
   static bool testAttribute(Qt::ApplicationAttribute attribute);

   static void setOrganizationDomain(const QString &orgDomain);
   static QString organizationDomain();

   // wrapper for static method
   inline void cs_setOrganizationDomain(const QString &orgDomain);
   inline QString cs_organizationDomain() const;

   static void setOrganizationName(const QString &orgName);
   static QString organizationName();
   inline void cs_setOrganizationName(const QString &orgName);
   inline QString cs_organizationName() const;

   static void setApplicationName(const QString &application);
   static QString applicationName();
   inline void cs_setApplicationName(const QString &application);
   inline QString cs_applicationName() const;

   static void setApplicationVersion(const QString &version);
   static QString applicationVersion();
   inline void cs_setApplicationVersion(const QString &version);
   inline QString cs_applicationVersion() const;

   static void setSetuidAllowed(bool allow);
   static bool isSetuidAllowed();

   static QCoreApplication *instance() {
      return m_self;
   }

   static int exec();
   static void processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
   static void processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime);
   static void exit(int returnCode = 0);

   static bool sendEvent(QObject *receiver, QEvent *event);
   static void postEvent(QObject *receiver, QEvent *event, int priority = Qt::NormalEventPriority);

   static void sendPostedEvents(QObject *receiver = nullptr, int event_type = 0);
   static void removePostedEvents(QObject *receiver, int eventType = 0);
   static QAbstractEventDispatcher *eventDispatcher();
   static void setEventDispatcher(QAbstractEventDispatcher *eventDispatcher);

   virtual bool notify(QObject *receiver, QEvent *event);

   static bool startingUp();
   static bool closingDown();

   static QString applicationDirPath();
   static QString applicationFilePath();
   static qint64 applicationPid();

   static void setLibraryPaths(const QStringList &paths);
   static QStringList libraryPaths();
   static void addLibraryPath(const QString &path);
   static void removeLibraryPath(const QString &path);

   static void installTranslator(QTranslator *translationFile);
   static void removeTranslator(QTranslator *translationFile);

   static QString translate(const char *context, const char *text, const char *comment = nullptr,
         std::optional<int> numArg = std::optional<int>());

   static QString translate(const QString &context, const QString &text, const QString &comment = QString(),
         std::optional<int> numArg = std::optional<int>());

   static void flush();
   void installNativeEventFilter(QAbstractNativeEventFilter *filterObj);
   void removeNativeEventFilter(QAbstractNativeEventFilter *filterObj);

   void cs_internal_maybeQuit();
   bool cs_isRealGuiApp();

   static bool isQuitLockEnabled();
   static void setQuitLockEnabled(bool enabled);

   // wrapper for static method
   inline bool cs_isQuitLockEnabled() const;
   inline void cs_setQuitLockEnabled(bool enabled);

   CORE_CS_SLOT_1(Public, static void quit())
   CORE_CS_SLOT_2(quit)

   CORE_CS_SIGNAL_1(Public, void aboutToQuit())
   CORE_CS_SIGNAL_2(aboutToQuit)

   CORE_CS_SIGNAL_1(Public, void unixSignal(int signalId))
   CORE_CS_SIGNAL_2(unixSignal, signalId)

 protected:
   bool event(QEvent *) override;
   virtual bool compressEvent(QEvent *, QObject *receiver, QPostEventList *);
   QCoreApplication(QCoreApplicationPrivate &p);

   QScopedPointer<QCoreApplicationPrivate> d_ptr;

 private:
   static bool sendSpontaneousEvent(QObject *receiver, QEvent *event);
   bool notifyInternal(QObject *receiver, QEvent *event);

   void init();

   static QCoreApplication *m_self;

   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QClassFactory;
   friend class QCocoaEventDispatcherPrivate;
   friend class QEventDispatcherUNIXPrivate;
   friend class QWidget;
   friend class QWidgetWindow;
   friend class QWidgetPrivate;

   friend bool qt_sendSpontaneousEvent(QObject *, QEvent *);
   friend Q_CORE_EXPORT QString qAppName();
};

// wrappers for static method
void QCoreApplication::cs_setApplicationName(const QString &application)
{
   QCoreApplication::setApplicationName(application);
}

QString QCoreApplication::cs_applicationName() const
{
   return QCoreApplication::applicationName();
}

void QCoreApplication::cs_setOrganizationName(const QString &orgName)
{
   QCoreApplication::setOrganizationName(orgName);
}

QString QCoreApplication::cs_organizationName() const
{
   return QCoreApplication::organizationName();
}

void QCoreApplication::cs_setApplicationVersion(const QString &version)
{
   QCoreApplication::setApplicationVersion(version);
}

QString QCoreApplication::cs_applicationVersion() const
{
   return QCoreApplication::applicationVersion();
}

void QCoreApplication::cs_setOrganizationDomain(const QString &orgDomain)
{
   QCoreApplication::setOrganizationDomain(orgDomain);
}

QString QCoreApplication::cs_organizationDomain() const
{
   return QCoreApplication::organizationDomain();
}

bool QCoreApplication::cs_isQuitLockEnabled() const
{
   return QCoreApplication::isQuitLockEnabled();
}

void QCoreApplication::cs_setQuitLockEnabled(bool enabled)
{
   return QCoreApplication::setQuitLockEnabled(enabled);
}

inline bool QCoreApplication::sendEvent(QObject *receiver, QEvent *event)
{
   if (event) {
      event->spont = false;
   }

   return m_self ? m_self->notifyInternal(receiver, event) : false;
}

inline bool QCoreApplication::sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
   if (event) {
      event->spont = true;
   }

   return m_self ? m_self->notifyInternal(receiver, event) : false;
}

#define Q_DECLARE_TR_FUNCTIONS(context) \
   public: \
   static QString tr(const char *text, const char *comment = nullptr,       \
         std::optional<int> numArg = std::optional<int>())                  \
   { return QCoreApplication::translate(#context, text, comment, numArg); } \
   private:

using FP_Void = void (*)();

Q_CORE_EXPORT void qAddPreRoutine(FP_Void);
Q_CORE_EXPORT void qAddPostRoutine(FP_Void);
Q_CORE_EXPORT void qRemovePostRoutine(FP_Void);
Q_CORE_EXPORT QString qAppName();

#if defined(Q_OS_WIN)
Q_CORE_EXPORT QString decodeMSG(const MSG &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const MSG &);
#endif

#endif
