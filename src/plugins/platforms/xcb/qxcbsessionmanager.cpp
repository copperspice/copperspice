/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qxcbsessionmanager.h>

#include <qapplication.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qsocketnotifier.h>
#include <qvarlengtharray.h>

#include <X11/SM/SMlib.h>

#include <errno.h>  // ERANGE
#include <cerrno>   // ERANGE

#include <sys/time.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

class QSmSocketReceiver : public QObject
{
   CS_OBJECT(QSmSocketReceiver)

 public:
   QSmSocketReceiver(int socket) {
      QSocketNotifier *sn = new QSocketNotifier(socket, QSocketNotifier::Read, this);
      connect(sn, SIGNAL(activated(int)), this, SLOT(socketActivated(int)));
   }

   CS_SLOT_1(Public, void socketActivated(int))
   CS_SLOT_2(socketActivated)
};

static SmcConn smcConnection = 0;
static bool sm_interactionActive;
static bool sm_smActive;
static int sm_interactStyle;
static int sm_saveType;
static bool sm_cancel;
static bool sm_waitingForInteraction;
static bool sm_isshutdown;
static bool sm_phase2;
static bool sm_in_phase2;

bool qt_sm_blockUserInput = false;

static QSmSocketReceiver *sm_receiver = 0;

static void resetSmState();
static void sm_setProperty(const char *name, const char *type, int num_vals, SmPropValue *vals);
static void sm_saveYourselfCallback(SmcConn smcConn, SmPointer clientData, int saveType, Bool shutdown, int interactStyle, Bool fast);
static void sm_saveYourselfPhase2Callback(SmcConn smcConn, SmPointer clientData);
static void sm_dieCallback(SmcConn smcConn, SmPointer clientData) ;
static void sm_shutdownCancelledCallback(SmcConn smcConn, SmPointer clientData);
static void sm_saveCompleteCallback(SmcConn smcConn, SmPointer clientData);
static void sm_interactCallback(SmcConn smcConn, SmPointer clientData);
static void sm_performSaveYourself(QXcbSessionManager *);

static void resetSmState()
{
   sm_waitingForInteraction = false;
   sm_interactionActive = false;
   sm_interactStyle = SmInteractStyleNone;
   sm_smActive = false;
   qt_sm_blockUserInput = false;
   sm_isshutdown = false;
   sm_phase2 = false;
   sm_in_phase2 = false;
}

// theoretically it is possible to set several properties at once. For
// simplicity, however, we do just one property at a time

static void sm_setProperty(const char *name, const char *type,
   int num_vals, SmPropValue *vals)
{
   if (num_vals) {
      SmProp prop;
      prop.name = const_cast<char *>(name);
      prop.type = const_cast<char *>(type);
      prop.num_vals = num_vals;
      prop.vals = vals;

      SmProp *props[1];
      props[0] = &prop;
      SmcSetProperties(smcConnection, 1, props);
   } else {
      char *names[1];
      names[0] = const_cast<char *>(name);
      SmcDeleteProperties(smcConnection, 1, names);
   }
}

static void sm_setProperty(const QString &name, const QString &value)
{
   QByteArray v = value.toUtf8();
   SmPropValue prop;
   prop.length = v.length();
   prop.value = (SmPointer) const_cast<char *>(v.constData());
   sm_setProperty(name.toLatin1().data(), SmARRAY8, 1, &prop);
}

static void sm_setProperty(const QString &name, const QStringList &value)
{
   SmPropValue *prop = new SmPropValue[value.count()];
   int count = 0;
   QList<QByteArray> vl;

   for (QStringList::const_iterator it = value.begin(); it != value.end(); ++it) {
      prop[count].length = (*it).length();
      vl.append((*it).toUtf8());
      prop[count].value = (char *)vl.last().data();
      ++count;
   }

   sm_setProperty(name.toLatin1().data(), SmLISTofARRAY8, count, prop);
   delete [] prop;
}


// workaround for broken libsm, see below
struct QT_smcConn {
   unsigned int save_yourself_in_progress : 1;
   unsigned int shutdown_in_progress : 1;
};

static void sm_saveYourselfCallback(SmcConn smcConn, SmPointer clientData,
   int saveType, Bool shutdown, int interactStyle, Bool /*fast*/)
{
   if (smcConn != smcConnection) {
      return;
   }

   sm_cancel = false;
   sm_smActive = true;
   sm_isshutdown = shutdown;
   sm_saveType = saveType;
   sm_interactStyle = interactStyle;

   // ugly workaround for broken libSM. libSM should do that _before_
   // actually invoking the callback in sm_process.c
   ((QT_smcConn *)smcConn)->save_yourself_in_progress = true;
   if (sm_isshutdown) {
      ((QT_smcConn *)smcConn)->shutdown_in_progress = true;
   }

   sm_performSaveYourself((QXcbSessionManager *) clientData);
   if (!sm_isshutdown) { // we cannot expect a confirmation message in that case
      resetSmState();
   }
}

static void sm_performSaveYourself(QXcbSessionManager *sm)
{
   if (sm_isshutdown) {
      qt_sm_blockUserInput = true;
   }

   // generate a new session key
   timeval tv;
   gettimeofday(&tv, 0);
   sm->setSessionKey(QString::number(quint64(tv.tv_sec)) + '_' + QString::number(quint64(tv.tv_usec)));

   QStringList arguments = QCoreApplication::arguments();
   QString argument0 = arguments.isEmpty() ? QCoreApplication::applicationFilePath() : arguments.at(0);

   // tell the session manager about our program in best POSIX style
   sm_setProperty(QString::fromLatin1(SmProgram), argument0);

   // tell the session manager about our user as well.
   struct passwd *entryPtr = nullptr;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && (_POSIX_THREAD_SAFE_FUNCTIONS - 0 > 0)
   QVarLengthArray<char, 1024> buf(qMax<long>(sysconf(_SC_GETPW_R_SIZE_MAX), 1024L));
   struct passwd entry;

   while (getpwuid_r(geteuid(), &entry, buf.data(), buf.size(), &entryPtr) == ERANGE) {
      if (buf.size() >= 32768) {
         // too big already, fail
         static char badusername[] = "";
         entryPtr = &entry;
         entry.pw_name = badusername;
         break;
      }

      // retry with a bigger buffer
      buf.resize(buf.size() * 2);
   }

#else
   entryPtr = getpwuid(geteuid());
#endif

   if (entryPtr) {
      sm_setProperty(QString::fromLatin1(SmUserID), QString::fromUtf8(entryPtr->pw_name));
   }

   // generate a restart and discard command that makes sense
   QStringList restart;
   restart  << argument0 << "-session"
      << sm->sessionId() + '_' + sm->sessionKey();

   QFileInfo fi = QCoreApplication::applicationFilePath();
   if (qAppName().compare(fi.fileName(), Qt::CaseInsensitive) != 0) {
      restart << QLatin1String("-name") << qAppName();
   }

   sm->setRestartCommand(restart);
   QStringList discard;
   sm->setDiscardCommand(discard);

   switch (sm_saveType) {
      case SmSaveBoth:
         sm->appCommitData();
         if (sm_isshutdown && sm_cancel) {
            break;   // we cancelled the shutdown, no need to save state
         }
      // fall through
      case SmSaveLocal:
         sm->appSaveState();
         break;
      case SmSaveGlobal:
         sm->appCommitData();
         break;
      default:
         break;
   }

   if (sm_phase2 && !sm_in_phase2) {
      SmcRequestSaveYourselfPhase2(smcConnection, sm_saveYourselfPhase2Callback, (SmPointer *) sm);
      qt_sm_blockUserInput = false;
   } else {
      // close eventual interaction monitors and cancel the
      // shutdown, if required. Note that we can only cancel when
      // performing a shutdown, it does not work for checkpoints
      if (sm_interactionActive) {
         SmcInteractDone(smcConnection, sm_isshutdown && sm_cancel);
         sm_interactionActive = false;
      } else if (sm_cancel && sm_isshutdown) {
         if (sm->allowsErrorInteraction()) {
            SmcInteractDone(smcConnection, True);
            sm_interactionActive = false;
         }
      }

      // set restart and discard command in session manager
      sm_setProperty(QString::fromLatin1(SmRestartCommand), sm->restartCommand());
      sm_setProperty(QString::fromLatin1(SmDiscardCommand), sm->discardCommand());

      // set the restart hint
      SmPropValue prop;
      prop.length = sizeof(int);
      int value = sm->restartHint();
      prop.value = (SmPointer) &value;
      sm_setProperty(SmRestartStyleHint, SmCARD8, 1, &prop);

      // we are done
      SmcSaveYourselfDone(smcConnection, !sm_cancel);
   }
}

static void sm_dieCallback(SmcConn smcConn, SmPointer /* clientData */)
{
   if (smcConn != smcConnection) {
      return;
   }
   resetSmState();
   QEvent quitEvent(QEvent::Quit);
   QApplication::sendEvent(qApp, &quitEvent);
}

static void sm_shutdownCancelledCallback(SmcConn smcConn, SmPointer clientData)
{
   if (smcConn != smcConnection) {
      return;
   }
   if (sm_waitingForInteraction) {
      ((QXcbSessionManager *) clientData)->exitEventLoop();
   }
   resetSmState();
}

static void sm_saveCompleteCallback(SmcConn smcConn, SmPointer /*clientData */)
{
   if (smcConn != smcConnection) {
      return;
   }
   resetSmState();
}

static void sm_interactCallback(SmcConn smcConn, SmPointer clientData)
{
   if (smcConn != smcConnection) {
      return;
   }
   if (sm_waitingForInteraction) {
      ((QXcbSessionManager *) clientData)->exitEventLoop();
   }
}

static void sm_saveYourselfPhase2Callback(SmcConn smcConn, SmPointer clientData)
{
   if (smcConn != smcConnection) {
      return;
   }
   sm_in_phase2 = true;
   sm_performSaveYourself((QXcbSessionManager *) clientData);
}


void QSmSocketReceiver::socketActivated(int)
{
   IceProcessMessages(SmcGetIceConnection(smcConnection), 0, 0);
}


// QXcbSessionManager starts here

QXcbSessionManager::QXcbSessionManager(const QString &id, const QString &key)
   : QPlatformSessionManager(id, key)
   , m_eventLoop(0)
{
   resetSmState();
   char cerror[256];
   char *myId = 0;
   QByteArray b_id = id.toLatin1();
   char *prevId = b_id.data();

   SmcCallbacks cb;
   cb.save_yourself.callback = sm_saveYourselfCallback;
   cb.save_yourself.client_data = (SmPointer) this;
   cb.die.callback = sm_dieCallback;
   cb.die.client_data = (SmPointer) this;
   cb.save_complete.callback = sm_saveCompleteCallback;
   cb.save_complete.client_data = (SmPointer) this;
   cb.shutdown_cancelled.callback = sm_shutdownCancelledCallback;
   cb.shutdown_cancelled.client_data = (SmPointer) this;

   // avoid showing a warning message below
   if (! qgetenv("SESSION_MANAGER").isEmpty()) {
      return;
   }

   smcConnection = SmcOpenConnection(0, 0, 1, 0,
         SmcSaveYourselfProcMask | SmcDieProcMask | SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask,
         &cb, prevId, &myId, 256, cerror);

   setSessionId(QString::fromLatin1(myId));
   ::free(myId); // it was allocated by C

   QString error = QString::fromUtf8(cerror);
   if (! smcConnection) {
      qWarning("Session management error: %s", qPrintable(error));
   } else {
      sm_receiver = new QSmSocketReceiver(IceConnectionNumber(SmcGetIceConnection(smcConnection)));
   }
}

QXcbSessionManager::~QXcbSessionManager()
{
   if (smcConnection) {
      SmcCloseConnection(smcConnection, 0, 0);
   }
   smcConnection = 0;
   delete sm_receiver;
}


void *QXcbSessionManager::handle() const
{
   return (void *) smcConnection;
}

bool QXcbSessionManager::allowsInteraction()
{
   if (sm_interactionActive) {
      return true;
   }

   if (sm_waitingForInteraction) {
      return false;
   }

   if (sm_interactStyle == SmInteractStyleAny) {
      sm_waitingForInteraction = SmcInteractRequest(smcConnection,
            SmDialogNormal,
            sm_interactCallback,
            (SmPointer *) this);
   }
   if (sm_waitingForInteraction) {
      QEventLoop eventLoop;
      m_eventLoop = &eventLoop;
      eventLoop.exec();
      m_eventLoop = 0;

      sm_waitingForInteraction = false;
      if (sm_smActive) { // not cancelled
         sm_interactionActive = true;
         qt_sm_blockUserInput = false;
         return true;
      }
   }
   return false;
}

bool QXcbSessionManager::allowsErrorInteraction()
{
   if (sm_interactionActive) {
      return true;
   }

   if (sm_waitingForInteraction) {
      return false;
   }

   if (sm_interactStyle == SmInteractStyleAny || sm_interactStyle == SmInteractStyleErrors) {
      sm_waitingForInteraction = SmcInteractRequest(smcConnection,
            SmDialogError,
            sm_interactCallback,
            (SmPointer *) this);
   }
   if (sm_waitingForInteraction) {
      QEventLoop eventLoop;
      m_eventLoop = &eventLoop;
      eventLoop.exec();
      m_eventLoop = 0;

      sm_waitingForInteraction = false;
      if (sm_smActive) { // not cancelled
         sm_interactionActive = true;
         qt_sm_blockUserInput = false;
         return true;
      }
   }
   return false;
}

void QXcbSessionManager::release()
{
   if (sm_interactionActive) {
      SmcInteractDone(smcConnection, False);
      sm_interactionActive = false;
      if (sm_smActive && sm_isshutdown) {
         qt_sm_blockUserInput = true;
      }
   }
}

void QXcbSessionManager::cancel()
{
   sm_cancel = true;
}

void QXcbSessionManager::setManagerProperty(const QString &name, const QString &value)
{
   sm_setProperty(name, value);
}

void QXcbSessionManager::setManagerProperty(const QString &name, const QStringList &value)
{
   sm_setProperty(name, value);
}

bool QXcbSessionManager::isPhase2() const
{
   return sm_in_phase2;
}

void QXcbSessionManager::requestPhase2()
{
   sm_phase2 = true;
}

void QXcbSessionManager::exitEventLoop()
{
   m_eventLoop->exit();
}

