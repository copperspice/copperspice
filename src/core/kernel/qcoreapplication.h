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

// do not move include, if qcoreapplication.h is included directly forward declarations are not sufficient 12/30/2013
#include <qobject.h>

#ifndef QCOREAPPLICATION_H
#define QCOREAPPLICATION_H

#include <qcoreevent.h>
#include <qeventloop.h>
#include <QScopedPointer>

#if defined(Q_OS_WIN) && ! defined(tagMSG)
typedef struct tagMSG MSG;
#endif

QT_BEGIN_NAMESPACE

class QCoreApplicationPrivate;
class QTextCodec;
class QTranslator;
class QPostEventList;
class QStringList;

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

   Q_DECLARE_PRIVATE(QCoreApplication)

 public:
   enum { ApplicationFlags = CS_VERSION | 0x01000000 };

   QCoreApplication(int &argc, char **argv, int = ApplicationFlags );
   ~QCoreApplication();

#ifdef QT_DEPRECATED
   QT_DEPRECATED static int argc();
   QT_DEPRECATED static char **argv();
#endif

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

   static QCoreApplication *instance() {
      return self;
   }

   static int exec();
   static void processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
   static void processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime);
   static void exit(int retcode = 0);

   static bool sendEvent(QObject *receiver, QEvent *event);
   static void postEvent(QObject *receiver, QEvent *event);
   static void postEvent(QObject *receiver, QEvent *event, int priority);
   static void sendPostedEvents(QObject *receiver, int event_type);
   static void sendPostedEvents();
   static void removePostedEvents(QObject *receiver);
   static void removePostedEvents(QObject *receiver, int eventType);
   static bool hasPendingEvents();

   virtual bool notify(QObject *, QEvent *);

   static bool startingUp();
   static bool closingDown();

   static QString applicationDirPath();
   static QString applicationFilePath();
   static qint64 applicationPid();

   static void setLibraryPaths(const QStringList &);
   static QStringList libraryPaths();
   static void addLibraryPath(const QString &);
   static void removeLibraryPath(const QString &);

#ifndef QT_NO_TRANSLATION
   static void installTranslator(QTranslator *messageFile);
   static void removeTranslator(QTranslator *messageFile);
#endif

   enum Encoding { CodecForTr, UnicodeUTF8, DefaultCodec = CodecForTr };

   static QString translate(const char *context, const char *key, const char *disambiguation = nullptr,
                  Encoding encoding = CodecForTr, int n = -1);

   static void flush();

#if defined(Q_OS_WIN)
   virtual bool winEventFilter(MSG *message, long *result);
#endif

#if defined(Q_OS_UNIX)
   static void watchUnixSignal(int signal, bool watch);
#endif

   typedef bool (*EventFilter)(void *message, long *result);
   EventFilter setEventFilter(EventFilter filter);
   bool filterEvent(void *message, long *result);

   CORE_CS_SLOT_1(Public, static void quit())
   CORE_CS_SLOT_2(quit)

   CORE_CS_SIGNAL_1(Public, void aboutToQuit())
   CORE_CS_SIGNAL_2(aboutToQuit)

   CORE_CS_SIGNAL_1(Public, void unixSignal(int un_named_arg1))
   CORE_CS_SIGNAL_2(unixSignal, un_named_arg1)

 protected:
   bool event(QEvent *) override;

   virtual bool compressEvent(QEvent *, QObject *receiver, QPostEventList *);

   QCoreApplication(QCoreApplicationPrivate &p);

   QScopedPointer<QCoreApplicationPrivate> d_ptr;

 private:
   static bool sendSpontaneousEvent(QObject *receiver, QEvent *event);
   bool notifyInternal(QObject *receiver, QEvent *event);

   void init();

   static QCoreApplication *self;

   Q_DISABLE_COPY(QCoreApplication)

   friend class QEventDispatcherUNIXPrivate;
   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QETWidget;
   friend class QShortcutMap;
   friend class QWidget;
   friend class QWidgetPrivate;
   friend bool qt_sendSpontaneousEvent(QObject *, QEvent *);
   friend Q_CORE_EXPORT QString qAppName();
   friend class QClassFactory;
};


void QCoreApplication::cs_setApplicationName(const QString &application)
{
   QCoreApplication::setApplicationName(application);
};

QString QCoreApplication::cs_applicationName() const
{
   return QCoreApplication::applicationName();
};

void QCoreApplication::cs_setOrganizationName(const QString &orgName)
{
   QCoreApplication::setOrganizationName(orgName);
};

QString QCoreApplication::cs_organizationName() const
{
   return QCoreApplication::organizationName();
};

void QCoreApplication::cs_setApplicationVersion(const QString &version)
{
   QCoreApplication::setApplicationVersion(version);
};

QString QCoreApplication::cs_applicationVersion() const
{
   return QCoreApplication::applicationVersion();
};

void QCoreApplication::cs_setOrganizationDomain(const QString &orgDomain)
{
   QCoreApplication::setOrganizationDomain(orgDomain);
};

QString QCoreApplication::cs_organizationDomain() const
{
   return QCoreApplication::organizationDomain();
};


inline bool QCoreApplication::sendEvent(QObject *receiver, QEvent *event)
{
   if (event) {
      event->spont = false;
   }
   return self ? self->notifyInternal(receiver, event) : false;
}

inline bool QCoreApplication::sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
   if (event) {
      event->spont = true;
   }
   return self ? self->notifyInternal(receiver, event) : false;
}

inline void QCoreApplication::sendPostedEvents()
{
   sendPostedEvents(0, 0);
}


// * *
#ifdef QT_NO_TRANSLATION

// 1
inline QString QCoreApplication::translate(const char *, const char *sourceText, const char *, Encoding encoding)
{
#ifndef QT_NO_TEXTCODEC
   if (encoding == UnicodeUTF8) {
      return QString::fromUtf8(sourceText);
   }
#else
   Q_UNUSED(encoding)

#endif
   return QString::fromLatin1(sourceText);
}

// 2
inline QString QCoreApplication::translate(const char *, const char *sourceText, const char *, Encoding encoding, int)
{
#ifndef QT_NO_TEXTCODEC
   if (encoding == UnicodeUTF8) {
      return QString::fromUtf8(sourceText);
   }
#else
   Q_UNUSED(encoding)
#endif
   return QString::fromLatin1(sourceText);
}
#endif


// ### merge the four functions into two (using "int n = -1")
#define Q_DECLARE_TR_FUNCTIONS(context) \
public: \
    static inline QString tr(const char *sourceText, const char *disambiguation = 0) \
        { return QCoreApplication::translate(#context, sourceText, disambiguation); } \
    static inline QString trUtf8(const char *sourceText, const char *disambiguation = 0) \
        { return QCoreApplication::translate(#context, sourceText, disambiguation, \
                                             QCoreApplication::UnicodeUTF8); } \
    static inline QString tr(const char *sourceText, const char *disambiguation, int n) \
        { return QCoreApplication::translate(#context, sourceText, disambiguation, \
                                             QCoreApplication::CodecForTr, n); } \
    static inline QString trUtf8(const char *sourceText, const char *disambiguation, int n) \
        { return QCoreApplication::translate(#context, sourceText, disambiguation, \
                                             QCoreApplication::UnicodeUTF8, n); } \
private:

//
using QtStartUpFunction = void (*)();
using QtCleanUpFunction = void (*)();

Q_CORE_EXPORT void qAddPreRoutine(QtStartUpFunction);
Q_CORE_EXPORT void qAddPostRoutine(QtCleanUpFunction);
Q_CORE_EXPORT void qRemovePostRoutine(QtCleanUpFunction);
Q_CORE_EXPORT QString qAppName();

#if defined(Q_OS_WIN)
Q_CORE_EXPORT QString decodeMSG(const MSG &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const MSG &);
#endif

QT_END_NAMESPACE

#endif // QCOREAPPLICATION_H
