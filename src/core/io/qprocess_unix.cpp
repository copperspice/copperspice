/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

//#define QPROCESS_DEBUG
#include <qdebug.h>

#ifndef QT_NO_PROCESS

#if defined QPROCESS_DEBUG
#include <qstring.h>
#include <ctype.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/

QT_BEGIN_NAMESPACE
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
   if (! data) {
      return "(null)";
   }

   QByteArray out;
   for (int i = 0; i < len; ++i) {
      char c = data[i];
      if (isprint(c)) {
         out += c;
      } else switch (c) {
            case '\n':
               out += "\\n";
               break;
            case '\r':
               out += "\\r";
               break;
            case '\t':
               out += "\\t";
               break;
            default:
               QString tmp;
               tmp.sprintf("\\%o", c);
               out += tmp.toLatin1();
         }
   }

   if (len < maxSize) {
      out += "...";
   }

   return out;
}
QT_END_NAMESPACE
#endif

#include <qplatformdefs.h>
#include <qprocess.h>
#include <qprocess_p.h>
#include <qcore_unix_p.h>

#ifdef Q_OS_MAC
#include <qcore_mac_p.h>
#endif

#include <qcoreapplication_p.h>
#include <qthread_p.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qhash.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qsocketnotifier.h>
#include <qthread.h>
#include <qelapsedtimer.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

QT_BEGIN_NAMESPACE

// POSIX requires PIPE_BUF to be 512 or larger
// so we will use 512
static const int errorBufferMax = 512;

static int qt_qprocess_deadChild_pipe[2];
static struct sigaction qt_sa_old_sigchld_handler;
static void qt_sa_sigchld_sigaction(int signum, siginfo_t *info, void *context)
{
   // *Never* use the info or contect variables in this function
   // (except for passing them to the next signal in the chain).
   // We cannot be sure if another library or if the application
   // installed a signal handler for SIGCHLD without SA_SIGINFO
   // and fails to pass the arguments to us. If they do that,
   // these arguments contain garbage and we'd most likely crash.

   qt_safe_write(qt_qprocess_deadChild_pipe[1], "", 1);

#if defined (QPROCESS_DEBUG)
   fprintf(stderr, "*** SIGCHLD\n");
#endif

   // load as volatile
   volatile struct sigaction *vsa = &qt_sa_old_sigchld_handler;

   if (qt_sa_old_sigchld_handler.sa_flags & SA_SIGINFO) {
      void (*oldAction)(int, siginfo_t *, void *) = vsa->sa_sigaction;

      if (oldAction) {
         oldAction(signum, info, context);
      }
   } else {
      void (*oldAction)(int) = vsa->sa_handler;

      if (oldAction && oldAction != SIG_IGN) {
         oldAction(signum);
      }
   }
}

static inline void add_fd(int &nfds, int fd, fd_set *fdset)
{
   FD_SET(fd, fdset);
   if ((fd) > nfds) {
      nfds = fd;
   }
}

struct QProcessInfo {
   QProcess *process;
   int deathPipe;
   int exitResult;
   pid_t pid;
   int serialNumber;
};

class QProcessManager : public QThread
{
   CORE_CS_OBJECT(QProcessManager)

 public:
   QProcessManager();
   ~QProcessManager();

   void run() override;
   void catchDeadChildren();
   void add(pid_t pid, QProcess *process);
   void remove(QProcess *process);
   void lock();
   void unlock();

 private:
   QMutex mutex;
   QHash<int, QProcessInfo *> children;

};


Q_GLOBAL_STATIC(QMutex, processManagerGlobalMutex)

static QProcessManager *processManager()
{
   // The constructor of QProcessManager should be called only once
   // so we cannot use Q_GLOBAL_STATIC directly for QProcessManager
   QMutex *mutex = processManagerGlobalMutex();
   QMutexLocker locker(mutex);
   static QProcessManager processManager;
   return &processManager;
}

QProcessManager::QProcessManager()
{
#if defined (QPROCESS_DEBUG)
   qDebug() << "QProcessManager::QProcessManager()";
#endif
   // initialize the dead child pipe and make it non-blocking. in the
   // extremely unlikely event that the pipe fills up, we do not under any
   // circumstances want to block.
   qt_safe_pipe(qt_qprocess_deadChild_pipe, O_NONBLOCK);

   // set up the SIGCHLD handler, which writes a single byte to the dead
   // child pipe every time a child dies.

   struct sigaction action;
   // use the old handler as template, i.e., preserve the signal mask
   // otherwise the original signal handler might be interrupted although it
   // was marked to never be interrupted
   ::sigaction(SIGCHLD, NULL, &action);
   action.sa_sigaction = qt_sa_sigchld_sigaction;
   // set the SA_SIGINFO flag such that we can use the three argument handler
   // function
   action.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
   ::sigaction(SIGCHLD, &action, &qt_sa_old_sigchld_handler);
}

QProcessManager::~QProcessManager()
{
   // notify the thread that we're shutting down.
   qt_safe_write(qt_qprocess_deadChild_pipe[1], "@", 1);
   qt_safe_close(qt_qprocess_deadChild_pipe[1]);
   wait();

   // on certain unixes, closing the reading end of the pipe will cause
   // select in run() to block forever, rather than return with EBADF.
   qt_safe_close(qt_qprocess_deadChild_pipe[0]);

   qt_qprocess_deadChild_pipe[0] = -1;
   qt_qprocess_deadChild_pipe[1] = -1;

   qDeleteAll(children.values());
   children.clear();

   struct sigaction currentAction;
   ::sigaction(SIGCHLD, 0, &currentAction);
   if (currentAction.sa_sigaction == qt_sa_sigchld_sigaction) {
      ::sigaction(SIGCHLD, &qt_sa_old_sigchld_handler, 0);
   }
}

void QProcessManager::run()
{
   forever {
      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(qt_qprocess_deadChild_pipe[0], &readset);

#if defined (QPROCESS_DEBUG)
      qDebug() << "QProcessManager::run() waiting for children to die";
#endif

      // block forever, or until activity is detected on the dead child
      // pipe. the only other peers are the SIGCHLD signal handler, and the
      // QProcessManager destructor.
      int nselect = select(qt_qprocess_deadChild_pipe[0] + 1, &readset, 0, 0, 0);
      if (nselect < 0)
      {
         if (errno == EINTR) {
            continue;
         }
         break;
      }

      // empty only one byte from the pipe, even though several SIGCHLD
      // signals may have been delivered in the meantime, to avoid race
      // conditions.
      char c;
      if (qt_safe_read(qt_qprocess_deadChild_pipe[0], &c, 1) < 0 || c == '@')
      {
         break;
      }

      // catch any and all children that we can.
      catchDeadChildren();
   }
}

void QProcessManager::catchDeadChildren()
{
   QMutexLocker locker(&mutex);

   // try to catch all children whose pid we have registered, and whose
   // deathPipe is still valid (i.e, we have not already notified it).
   QHash<int, QProcessInfo *>::Iterator it = children.begin();
   while (it != children.end()) {
      // notify all children that they may have died. they need to run
      // waitpid() in their own thread.
      QProcessInfo *info = it.value();
      qt_safe_write(info->deathPipe, "", 1);

#if defined (QPROCESS_DEBUG)
      qDebug() << "QProcessManager::run() sending death notice to" << info->process;
#endif
      ++it;
   }
}

static QAtomicInt idCounter = QAtomicInt { 1 };

void QProcessManager::add(pid_t pid, QProcess *process)
{
#if defined (QPROCESS_DEBUG)
   qDebug() << "QProcessManager::add() adding pid" << pid << "process" << process;
#endif

   // insert a new info structure for this process
   QProcessInfo *info = new QProcessInfo;
   info->process = process;
   info->deathPipe = process->d_func()->deathPipe[1];
   info->exitResult = 0;
   info->pid = pid;

   int serial = idCounter.fetchAndAddRelaxed(1);
   process->d_func()->serial = serial;
   children.insert(serial, info);
}

void QProcessManager::remove(QProcess *process)
{
   QMutexLocker locker(&mutex);

   int serial = process->d_func()->serial;
   QProcessInfo *info = children.take(serial);
#if defined (QPROCESS_DEBUG)
   if (info) {
      qDebug() << "QProcessManager::remove() removing pid" << info->pid << "process" << info->process;
   }
#endif
   delete info;
}

void QProcessManager::lock()
{
   mutex.lock();
}

void QProcessManager::unlock()
{
   mutex.unlock();
}

static int qt_create_pipe(int *pipe)
{
   if (pipe[0] != -1) {
      qt_safe_close(pipe[0]);
   }
   if (pipe[1] != -1) {
      qt_safe_close(pipe[1]);
   }
   int pipe_ret = qt_safe_pipe(pipe);
   if (pipe_ret != 0) {
      qWarning("QProcessPrivate::createPipe: Cannot create pipe %p: %s",
               pipe, qPrintable(qt_error_string(errno)));
   }
   return pipe_ret;
}

void QProcessPrivate::destroyPipe(int *pipe)
{
   if (pipe[1] != -1) {
      qt_safe_close(pipe[1]);
      pipe[1] = -1;
   }
   if (pipe[0] != -1) {
      qt_safe_close(pipe[0]);
      pipe[0] = -1;
   }
}

/*
    Create the pipes to a QProcessPrivate::Channel.

    This function must be called in order: stdin, stdout, stderr
*/
bool QProcessPrivate::createChannel(Channel &channel)
{
   Q_Q(QProcess);

   if (&channel == &stderrChannel && processChannelMode == QProcess::MergedChannels) {
      channel.pipe[0] = -1;
      channel.pipe[1] = -1;
      return true;
   }

   if (channel.type == Channel::Normal) {
      // we're piping this channel to our own process
      if (qt_create_pipe(channel.pipe) != 0) {
         return false;
      }

      // create the socket notifiers
      QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

      if (threadData->eventDispatcher) {
         if (&channel == &stdinChannel) {
            channel.notifier = new QSocketNotifier(channel.pipe[1], QSocketNotifier::Write, q);
            channel.notifier->setEnabled(false);
            QObject::connect(channel.notifier, SIGNAL(activated(int)), q, SLOT(_q_canWrite()));

         } else {
            channel.notifier = new QSocketNotifier(channel.pipe[0], QSocketNotifier::Read, q);
            const char *receiver;
            if (&channel == &stdoutChannel) {
               receiver = SLOT(_q_canReadStandardOutput());
            } else {
               receiver = SLOT(_q_canReadStandardError());
            }

            QObject::connect(channel.notifier, SIGNAL(activated(int)), q, receiver);
         }
      }

      return true;
   } else if (channel.type == Channel::Redirect) {
      // we're redirecting the channel to/from a file
      QByteArray fname = QFile::encodeName(channel.file);

      if (&channel == &stdinChannel) {
         // try to open in read-only mode
         channel.pipe[1] = -1;
         if ( (channel.pipe[0] = qt_safe_open(fname, O_RDONLY)) != -1) {
            return true;   // success
         }

         q->setErrorString(QProcess::tr("Could not open input redirection for reading"));
      } else {
         int mode = O_WRONLY | O_CREAT;
         if (channel.append) {
            mode |= O_APPEND;
         } else {
            mode |= O_TRUNC;
         }

         channel.pipe[0] = -1;
         if ( (channel.pipe[1] = qt_safe_open(fname, mode, 0666)) != -1) {
            return true;   // success
         }

         q->setErrorString(QProcess::tr("Could not open output redirection for writing"));
      }

      // could not open file
      processError = QProcess::FailedToStart;
      emit q->error(processError);
      cleanup();
      return false;
   } else {
      Q_ASSERT_X(channel.process, "QProcess::start", "Internal error");

      Channel *source;
      Channel *sink;

      if (channel.type == Channel::PipeSource) {
         // we are the source
         source = &channel;
         sink = &channel.process->stdinChannel;

         Q_ASSERT(source == &stdoutChannel);
         Q_ASSERT(sink->process == this && sink->type == Channel::PipeSink);
      } else {
         // we are the sink;
         source = &channel.process->stdoutChannel;
         sink = &channel;

         Q_ASSERT(sink == &stdinChannel);
         Q_ASSERT(source->process == this && source->type == Channel::PipeSource);
      }

      if (source->pipe[1] != INVALID_Q_PIPE || sink->pipe[0] != INVALID_Q_PIPE) {
         // already created, do nothing
         return true;
      } else {
         Q_ASSERT(source->pipe[0] == INVALID_Q_PIPE && source->pipe[1] == INVALID_Q_PIPE);
         Q_ASSERT(sink->pipe[0] == INVALID_Q_PIPE && sink->pipe[1] == INVALID_Q_PIPE);

         Q_PIPE pipe[2] = { -1, -1 };
         if (qt_create_pipe(pipe) != 0) {
            return false;
         }
         sink->pipe[0] = pipe[0];
         source->pipe[1] = pipe[1];

         return true;
      }
   }
}

QT_BEGIN_INCLUDE_NAMESPACE
#if defined(Q_OS_MAC) && !defined(QT_NO_CORESERVICES)
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#elif defined(Q_OS_SYMBIAN) || (defined(Q_OS_MAC) && defined(QT_NO_CORESERVICES))
static char *qt_empty_environ[] = { 0 };
#define environ qt_empty_environ
#else
extern char **environ;
#endif
QT_END_INCLUDE_NAMESPACE

QProcessEnvironment QProcessEnvironment::systemEnvironment()
{
   QProcessEnvironment env;
   const char *entry;
   for (int count = 0; (entry = environ[count]); ++count) {
      const char *equal = strchr(entry, '=');
      if (!equal) {
         continue;
      }

      QByteArray name(entry, equal - entry);
      QByteArray value(equal + 1);
      env.d->hash.insert(QProcessEnvironmentPrivate::Key(name),
                         QProcessEnvironmentPrivate::Value(value));
   }
   return env;
}

static char **_q_dupEnvironment(const QProcessEnvironmentPrivate::Hash &environment, int *envc)
{
   *envc = 0;
   if (environment.isEmpty()) {
      return 0;
   }

   // if LD_LIBRARY_PATH exists in the current environment, but
   // not in the environment list passed by the programmer, then
   // copy it over.
#if defined(Q_OS_MAC)
   static const char libraryPath[] = "DYLD_LIBRARY_PATH";
#else
   static const char libraryPath[] = "LD_LIBRARY_PATH";
#endif
   const QByteArray envLibraryPath = qgetenv(libraryPath);
   bool needToAddLibraryPath = !envLibraryPath.isEmpty() &&
                               !environment.contains(QProcessEnvironmentPrivate::Key(QByteArray(libraryPath)));

   char **envp = new char *[environment.count() + 2];
   envp[environment.count()] = 0;
   envp[environment.count() + 1] = 0;

   QProcessEnvironmentPrivate::Hash::ConstIterator it = environment.constBegin();
   const QProcessEnvironmentPrivate::Hash::ConstIterator end = environment.constEnd();
   for ( ; it != end; ++it) {
      QByteArray key = it.key().key;
      QByteArray value = it.value().bytes();
      key.reserve(key.length() + 1 + value.length());
      key.append('=');
      key.append(value);

      envp[(*envc)++] = ::strdup(key.constData());
   }

   if (needToAddLibraryPath)
      envp[(*envc)++] = ::strdup(QByteArray(QByteArray(libraryPath) + '=' +
                                            envLibraryPath).constData());
   return envp;
}

#ifdef Q_OS_MAC
Q_GLOBAL_STATIC(QMutex, cfbundleMutex);
#endif

void QProcessPrivate::startProcess()
{
   Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::startProcess()");
#endif

   processManager()->start();

   // Initialize pipes
   if (!createChannel(stdinChannel) ||
         !createChannel(stdoutChannel) ||
         !createChannel(stderrChannel) ||
         qt_create_pipe(childStartedPipe) != 0 ||
         qt_create_pipe(deathPipe) != 0) {
      processError = QProcess::FailedToStart;
      q->setErrorString(qt_error_string(errno));
      emit q->error(processError);
      cleanup();
      return;
   }

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

   if (threadData->eventDispatcher) {
      startupSocketNotifier = new QSocketNotifier(childStartedPipe[0], QSocketNotifier::Read, q);

      QObject::connect(startupSocketNotifier, SIGNAL(activated(int)), q, SLOT(_q_startupNotification()));
      deathNotifier = new QSocketNotifier(deathPipe[0], QSocketNotifier::Read, q);

      QObject::connect(deathNotifier, SIGNAL(activated(int)), q, SLOT(_q_processDied()));
   }

   // Start the process (platform dependent)
   q->setProcessState(QProcess::Starting);

   // Create argument list with right number of elements, and set the final
   // one to 0.
   char **argv = new char *[arguments.count() + 2];
   argv[arguments.count() + 1] = 0;

   // Encode the program name.
   QByteArray encodedProgramName = QFile::encodeName(program);

#ifdef Q_OS_MAC
   // allow invoking of .app bundles on the Mac.
   QFileInfo fileInfo(QString::fromUtf8(encodedProgramName.constData()));

   if (encodedProgramName.endsWith(".app") && fileInfo.isDir()) {
      QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(0,
                              QCFString(fileInfo.absoluteFilePath()), kCFURLPOSIXPathStyle, true);
      {
         // CFBundle is not reentrant, since CFBundleCreate might return a reference
         // to a cached bundle object. Protect the bundle calls with a mutex lock.
         QMutexLocker lock(cfbundleMutex());
         QCFType<CFBundleRef> bundle = CFBundleCreate(0, url);
         url = CFBundleCopyExecutableURL(bundle);
      }

      if (url) {
         QCFString str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
         encodedProgramName += "/Contents/MacOS/" + static_cast<QString>(str).toUtf8();
      }
   }
#endif

   // Add the program name to the argument list.
   char *dupProgramName = ::strdup(encodedProgramName.constData());
   argv[0] = dupProgramName;

   // Add every argument to the list
   for (int i = 0; i < arguments.count(); ++i) {
      QString arg = arguments.at(i);

#ifdef Q_OS_MAC
      // Mac OS X uses UTF8 for exec, regardless of the system locale.
      argv[i + 1] = ::strdup(arg.toUtf8().constData());
#else
      argv[i + 1] = ::strdup(arg.toLocal8Bit().constData());
#endif

   }

   // Duplicate the environment.
   int envc = 0;
   char **envp = 0;
   if (environment.d.constData()) {
      QProcessEnvironmentPrivate::MutexLocker locker(environment.d);
      envp = _q_dupEnvironment(environment.d.constData()->hash, &envc);
   }

   // Encode the working directory if it's non-empty, otherwise just pass 0.
   const char *workingDirPtr = 0;
   QByteArray encodedWorkingDirectory;
   if (!workingDirectory.isEmpty()) {
      encodedWorkingDirectory = QFile::encodeName(workingDirectory);
      workingDirPtr = encodedWorkingDirectory.constData();
   }

   // If the program does not specify a path, generate a list of possible
   // locations for the binary using the PATH environment variable.
   char **path = 0;
   int pathc = 0;

   if (!program.contains(QLatin1Char('/'))) {
      const QString pathEnv = QString::fromLocal8Bit(::getenv("PATH"));
      if (!pathEnv.isEmpty()) {
         QStringList pathEntries = pathEnv.split(QLatin1Char(':'), QString::SkipEmptyParts);
         if (!pathEntries.isEmpty()) {
            pathc = pathEntries.size();
            path = new char *[pathc + 1];
            path[pathc] = 0;

            for (int k = 0; k < pathEntries.size(); ++k) {
               QByteArray tmp = QFile::encodeName(pathEntries.at(k));
               if (!tmp.endsWith('/')) {
                  tmp += '/';
               }
               tmp += encodedProgramName;
               path[k] = ::strdup(tmp.constData());
            }
         }
      }
   }

   // Start the process manager, and fork off the child process.
   processManager()->lock();

   pid_t childPid = fork();
   int lastForkErrno = errno;

   if (childPid != 0) {
      // Clean up duplicated memory.
      free(dupProgramName);

      for (int i = 1; i <= arguments.count(); ++i) {
         free(argv[i]);
      }
      for (int i = 0; i < envc; ++i) {
         free(envp[i]);
      }
      for (int i = 0; i < pathc; ++i) {
         free(path[i]);
      }
      delete [] argv;
      delete [] envp;
      delete [] path;
   }

   if (childPid < 0) {
      // Cleanup, report error and return
#if defined (QPROCESS_DEBUG)
      qDebug("fork() failed: %s", qPrintable(qt_error_string(lastForkErrno)));
#endif
      processManager()->unlock();
      q->setProcessState(QProcess::NotRunning);
      processError = QProcess::FailedToStart;
      q->setErrorString(QProcess::tr("Resource error (fork failure): %1").arg(qt_error_string(lastForkErrno)));
      emit q->error(processError);
      cleanup();
      return;
   }

   // Start the child.
   if (childPid == 0) {
      execChild(workingDirPtr, path, argv, envp);
      ::_exit(-1);
   }


   // Register the child. In the mean time, we can get a SIGCHLD, so we need
   // to keep the lock held to avoid a race to catch the child.
   processManager()->add(childPid, q);
   pid = Q_PID(childPid);
   processManager()->unlock();

   // parent
   // close the ends we don't use and make all pipes non-blocking
   ::fcntl(deathPipe[0], F_SETFL, ::fcntl(deathPipe[0], F_GETFL) | O_NONBLOCK);
   qt_safe_close(childStartedPipe[1]);
   childStartedPipe[1] = -1;

   if (stdinChannel.pipe[0] != -1) {
      qt_safe_close(stdinChannel.pipe[0]);
      stdinChannel.pipe[0] = -1;
   }
   if (stdinChannel.pipe[1] != -1) {
      ::fcntl(stdinChannel.pipe[1], F_SETFL, ::fcntl(stdinChannel.pipe[1], F_GETFL) | O_NONBLOCK);
   }

   if (stdoutChannel.pipe[1] != -1) {
      qt_safe_close(stdoutChannel.pipe[1]);
      stdoutChannel.pipe[1] = -1;
   }
   if (stdoutChannel.pipe[0] != -1) {
      ::fcntl(stdoutChannel.pipe[0], F_SETFL, ::fcntl(stdoutChannel.pipe[0], F_GETFL) | O_NONBLOCK);
   }

   if (stderrChannel.pipe[1] != -1) {
      qt_safe_close(stderrChannel.pipe[1]);
      stderrChannel.pipe[1] = -1;
   }
   if (stderrChannel.pipe[0] != -1) {
      ::fcntl(stderrChannel.pipe[0], F_SETFL, ::fcntl(stderrChannel.pipe[0], F_GETFL) | O_NONBLOCK);
   }
}

void QProcessPrivate::execChild(const char *workingDir, char **path, char **argv, char **envp)
{
   ::signal(SIGPIPE, SIG_DFL);         // reset the signal that we ignored

   Q_Q(QProcess);

   // copy the stdin socket (without closing on exec)
   qt_safe_dup2(stdinChannel.pipe[0], STDIN_FILENO, 0);

   // copy the stdout and stderr if asked to
   if (processChannelMode != QProcess::ForwardedChannels) {
      qt_safe_dup2(stdoutChannel.pipe[1], STDOUT_FILENO, 0);

      // merge stdout and stderr if asked to
      if (processChannelMode == QProcess::MergedChannels) {
         qt_safe_dup2(STDOUT_FILENO, STDERR_FILENO, 0);
      } else {
         qt_safe_dup2(stderrChannel.pipe[1], STDERR_FILENO, 0);
      }
   }

   // make sure this fd is closed if execvp() succeeds
   qt_safe_close(childStartedPipe[0]);

   // enter the working directory
   if (workingDir) {
      QT_CHDIR(workingDir);
   }

   // this is a virtual call, and it base behavior is to do nothing.
   q->setupChildProcess();

   // execute the process
   if (!envp) {
      qt_safe_execvp(argv[0], argv);
   } else {
      if (path) {
         char **arg = path;
         while (*arg) {
            argv[0] = *arg;
#if defined (QPROCESS_DEBUG)
            fprintf(stderr, "QProcessPrivate::execChild() searching / starting %s\n", argv[0]);
#endif
            qt_safe_execve(argv[0], argv, envp);
            ++arg;
         }
      } else {
#if defined (QPROCESS_DEBUG)
         fprintf(stderr, "QProcessPrivate::execChild() starting %s\n", argv[0]);
#endif
         qt_safe_execve(argv[0], argv, envp);
      }
   }

   // notify failure
   QString error = qt_error_string(errno);
#if defined (QPROCESS_DEBUG)
   fprintf(stderr, "QProcessPrivate::execChild() failed (%s), notifying parent process\n", qPrintable(error));
#endif
   qt_safe_write(childStartedPipe[1], error.data(), error.length() * sizeof(QChar));
   qt_safe_close(childStartedPipe[1]);
   childStartedPipe[1] = -1;
}


bool QProcessPrivate::processStarted()
{
   ushort buf[errorBufferMax];
   int i = qt_safe_read(childStartedPipe[0], &buf, sizeof buf);
   if (startupSocketNotifier) {
      startupSocketNotifier->setEnabled(false);
      startupSocketNotifier->deleteLater();
      startupSocketNotifier = 0;
   }
   qt_safe_close(childStartedPipe[0]);
   childStartedPipe[0] = -1;

#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::processStarted() == %s", i <= 0 ? "true" : "false");
#endif

   // did we read an error message?
   if (i > 0) {
      q_func()->setErrorString(QString((const QChar *)buf, i / sizeof(QChar)));
   }

   return i <= 0;
}

qint64 QProcessPrivate::bytesAvailableFromStdout() const
{
   int nbytes = 0;
   qint64 available = 0;
   if (::ioctl(stdoutChannel.pipe[0], FIONREAD, (char *) &nbytes) >= 0) {
      available = (qint64) nbytes;
   }

#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::bytesAvailableFromStdout() == %lld", available);
#endif

   return available;
}

qint64 QProcessPrivate::bytesAvailableFromStderr() const
{
   int nbytes = 0;
   qint64 available = 0;
   if (::ioctl(stderrChannel.pipe[0], FIONREAD, (char *) &nbytes) >= 0) {
      available = (qint64) nbytes;
   }

#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::bytesAvailableFromStderr() == %lld", available);
#endif

   return available;
}

qint64 QProcessPrivate::readFromStdout(char *data, qint64 maxlen)
{
   qint64 bytesRead = qt_safe_read(stdoutChannel.pipe[0], data, maxlen);
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::readFromStdout(%p \"%s\", %lld) == %lld",
          data, qt_prettyDebug(data, bytesRead, 16).constData(), maxlen, bytesRead);
#endif
   return bytesRead;
}

qint64 QProcessPrivate::readFromStderr(char *data, qint64 maxlen)
{
   qint64 bytesRead = qt_safe_read(stderrChannel.pipe[0], data, maxlen);
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::readFromStderr(%p \"%s\", %lld) == %lld",
          data, qt_prettyDebug(data, bytesRead, 16).constData(), maxlen, bytesRead);
#endif
   return bytesRead;
}

qint64 QProcessPrivate::writeToStdin(const char *data, qint64 maxlen)
{
   qint64 written = qt_safe_write_nosignal(stdinChannel.pipe[1], data, maxlen);
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::writeToStdin(%p \"%s\", %lld) == %lld",
          data, qt_prettyDebug(data, maxlen, 16).constData(), maxlen, written);
   if (written == -1) {
      qDebug("QProcessPrivate::writeToStdin(), failed to write (%s)", qPrintable(qt_error_string(errno)));
   }
#endif
   // If the O_NONBLOCK flag is set and If some data can be written without blocking
   // the process, write() will transfer what it can and return the number of bytes written.
   // Otherwise, it will return -1 and set errno to EAGAIN
   if (written == -1 && errno == EAGAIN) {
      written = 0;
   }
   return written;
}

void QProcessPrivate::terminateProcess()
{
#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::killProcess()");
#endif
   if (pid) {
      ::kill(pid_t(pid), SIGTERM);
   }
}

void QProcessPrivate::killProcess()
{
#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::killProcess()");
#endif
   if (pid) {
      ::kill(pid_t(pid), SIGKILL);
   }
}

static int select_msecs(int nfds, fd_set *fdread, fd_set *fdwrite, int timeout)
{
   if (timeout < 0) {
      return qt_safe_select(nfds, fdread, fdwrite, 0, 0);
   }

   struct timeval tv;
   tv.tv_sec = timeout / 1000;
   tv.tv_usec = (timeout % 1000) * 1000;
   return qt_safe_select(nfds, fdread, fdwrite, 0, &tv);
}

/*
   Returns the difference between msecs and elapsed. If msecs is -1,
   however, -1 is returned.
*/
static int qt_timeout_value(int msecs, int elapsed)
{
   if (msecs == -1) {
      return -1;
   }

   int timeout = msecs - elapsed;
   return timeout < 0 ? 0 : timeout;
}

bool QProcessPrivate::waitForStarted(int msecs)
{
   Q_Q(QProcess);

#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::waitForStarted(%d) waiting for child to start (fd = %d)", msecs,
          childStartedPipe[0]);
#endif

   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(childStartedPipe[0], &fds);
   if (select_msecs(childStartedPipe[0] + 1, &fds, 0, msecs) == 0) {
      processError = QProcess::Timedout;
      q->setErrorString(QProcess::tr("Process operation timed out"));
#if defined (QPROCESS_DEBUG)
      qDebug("QProcessPrivate::waitForStarted(%d) == false (timed out)", msecs);
#endif
      return false;
   }

   bool startedEmitted = _q_startupNotification();
#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::waitForStarted() == %s", startedEmitted ? "true" : "false");
#endif
   return startedEmitted;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
   Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::waitForReadyRead(%d)", msecs);
#endif

   QElapsedTimer stopWatch;
   stopWatch.start();

   forever {
      fd_set fdread;
      fd_set fdwrite;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);

      int nfds = deathPipe[0];
      FD_SET(deathPipe[0], &fdread);

      if (processState == QProcess::Starting)
      {
         add_fd(nfds, childStartedPipe[0], &fdread);
      }

      if (stdoutChannel.pipe[0] != -1)
      {
         add_fd(nfds, stdoutChannel.pipe[0], &fdread);
      }
      if (stderrChannel.pipe[0] != -1)
      {
         add_fd(nfds, stderrChannel.pipe[0], &fdread);
      }

      if (!writeBuffer.isEmpty() && stdinChannel.pipe[1] != -1)
      {
         add_fd(nfds, stdinChannel.pipe[1], &fdwrite);
      }

      int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
      int ret = select_msecs(nfds + 1, &fdread, &fdwrite, timeout);
      if (ret < 0)
      {
         break;
      }
      if (ret == 0)
      {
         processError = QProcess::Timedout;
         q->setErrorString(QProcess::tr("Process operation timed out"));
         return false;
      }

      if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread))
      {
         if (!_q_startupNotification()) {
            return false;
         }
      }

      bool readyReadEmitted = false;
      if (stdoutChannel.pipe[0] != -1 && FD_ISSET(stdoutChannel.pipe[0], &fdread))
      {
         bool canRead = _q_canReadStandardOutput();
         if (processChannel == QProcess::StandardOutput && canRead) {
            readyReadEmitted = true;
         }
      }
      if (stderrChannel.pipe[0] != -1 && FD_ISSET(stderrChannel.pipe[0], &fdread))
      {
         bool canRead = _q_canReadStandardError();
         if (processChannel == QProcess::StandardError && canRead) {
            readyReadEmitted = true;
         }
      }
      if (readyReadEmitted)
      {
         return true;
      }

      if (stdinChannel.pipe[1] != -1 && FD_ISSET(stdinChannel.pipe[1], &fdwrite))
      {
         _q_canWrite();
      }

      if (deathPipe[0] == -1 || FD_ISSET(deathPipe[0], &fdread))
      {
         if (_q_processDied()) {
            return false;
         }
      }
   }
   return false;
}

bool QProcessPrivate::waitForBytesWritten(int msecs)
{
   Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::waitForBytesWritten(%d)", msecs);
#endif

   QElapsedTimer stopWatch;
   stopWatch.start();

   while (!writeBuffer.isEmpty()) {
      fd_set fdread;
      fd_set fdwrite;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);

      int nfds = deathPipe[0];
      FD_SET(deathPipe[0], &fdread);

      if (processState == QProcess::Starting) {
         add_fd(nfds, childStartedPipe[0], &fdread);
      }

      if (stdoutChannel.pipe[0] != -1) {
         add_fd(nfds, stdoutChannel.pipe[0], &fdread);
      }
      if (stderrChannel.pipe[0] != -1) {
         add_fd(nfds, stderrChannel.pipe[0], &fdread);
      }


      if (!writeBuffer.isEmpty() && stdinChannel.pipe[1] != -1) {
         add_fd(nfds, stdinChannel.pipe[1], &fdwrite);
      }

      int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
      int ret = select_msecs(nfds + 1, &fdread, &fdwrite, timeout);
      if (ret < 0) {
         break;
      }

      if (ret == 0) {
         processError = QProcess::Timedout;
         q->setErrorString(QProcess::tr("Process operation timed out"));
         return false;
      }

      if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread)) {
         if (!_q_startupNotification()) {
            return false;
         }
      }

      if (stdinChannel.pipe[1] != -1 && FD_ISSET(stdinChannel.pipe[1], &fdwrite)) {
         return _q_canWrite();
      }

      if (stdoutChannel.pipe[0] != -1 && FD_ISSET(stdoutChannel.pipe[0], &fdread)) {
         _q_canReadStandardOutput();
      }

      if (stderrChannel.pipe[0] != -1 && FD_ISSET(stderrChannel.pipe[0], &fdread)) {
         _q_canReadStandardError();
      }

      if (deathPipe[0] == -1 || FD_ISSET(deathPipe[0], &fdread)) {
         if (_q_processDied()) {
            return false;
         }
      }
   }

   return false;
}

bool QProcessPrivate::waitForFinished(int msecs)
{
   Q_Q(QProcess);
#if defined (QPROCESS_DEBUG)
   qDebug("QProcessPrivate::waitForFinished(%d)", msecs);
#endif

   QElapsedTimer stopWatch;
   stopWatch.start();

   forever {
      fd_set fdread;
      fd_set fdwrite;
      int nfds = -1;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);

      if (processState == QProcess::Starting)
      {
         add_fd(nfds, childStartedPipe[0], &fdread);
      }

      if (stdoutChannel.pipe[0] != -1)
      {
         add_fd(nfds, stdoutChannel.pipe[0], &fdread);
      }
      if (stderrChannel.pipe[0] != -1)
      {
         add_fd(nfds, stderrChannel.pipe[0], &fdread);
      }

      if (processState == QProcess::Running)
      {
         add_fd(nfds, deathPipe[0], &fdread);
      }

      if (!writeBuffer.isEmpty() && stdinChannel.pipe[1] != -1)
      {
         add_fd(nfds, stdinChannel.pipe[1], &fdwrite);
      }

      int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
      int ret = select_msecs(nfds + 1, &fdread, &fdwrite, timeout);
      if (ret < 0)
      {
         break;
      }
      if (ret == 0)
      {
         processError = QProcess::Timedout;
         q->setErrorString(QProcess::tr("Process operation timed out"));
         return false;
      }

      if (childStartedPipe[0] != -1 && FD_ISSET(childStartedPipe[0], &fdread))
      {
         if (!_q_startupNotification()) {
            return false;
         }
      }
      if (stdinChannel.pipe[1] != -1 && FD_ISSET(stdinChannel.pipe[1], &fdwrite))
      {
         _q_canWrite();
      }

      if (stdoutChannel.pipe[0] != -1 && FD_ISSET(stdoutChannel.pipe[0], &fdread))
      {
         _q_canReadStandardOutput();
      }

      if (stderrChannel.pipe[0] != -1 && FD_ISSET(stderrChannel.pipe[0], &fdread))
      {
         _q_canReadStandardError();
      }

      if (deathPipe[0] == -1 || FD_ISSET(deathPipe[0], &fdread))
      {
         if (_q_processDied()) {
            return true;
         }
      }
   }
   return false;
}

bool QProcessPrivate::waitForWrite(int msecs)
{
   fd_set fdwrite;
   FD_ZERO(&fdwrite);
   FD_SET(stdinChannel.pipe[1], &fdwrite);
   return select_msecs(stdinChannel.pipe[1] + 1, 0, &fdwrite, msecs < 0 ? 0 : msecs) == 1;
}

void QProcessPrivate::findExitCode()
{
   Q_Q(QProcess);
   processManager()->remove(q);
}

bool QProcessPrivate::waitForDeadChild()
{
   Q_Q(QProcess);

   // read a byte from the death pipe
   char c;
   qt_safe_read(deathPipe[0], &c, 1);

   // check if our process is dead
   int exitStatus;
   if (qt_safe_waitpid(pid_t(pid), &exitStatus, WNOHANG) > 0) {
      processManager()->remove(q);
      crashed = !WIFEXITED(exitStatus);
      exitCode = WEXITSTATUS(exitStatus);
#if defined QPROCESS_DEBUG
      qDebug() << "QProcessPrivate::waitForDeadChild() dead with exitCode"
               << exitCode << ", crashed?" << crashed;
#endif
      return true;
   }
#if defined QPROCESS_DEBUG
   qDebug() << "QProcessPrivate::waitForDeadChild() not dead!";
#endif
   return false;
}

void QProcessPrivate::_q_notified()
{
}


bool QProcessPrivate::startDetached(const QString &program, const QStringList &arguments,
                                    const QString &workingDirectory, qint64 *pid)
{
   processManager()->start();

   QByteArray encodedWorkingDirectory = QFile::encodeName(workingDirectory);

   // To catch the startup of the child
   int startedPipe[2];
   if (qt_safe_pipe(startedPipe) != 0) {
      return false;
   }
   // To communicate the pid of the child
   int pidPipe[2];
   if (qt_safe_pipe(pidPipe) != 0) {
      qt_safe_close(startedPipe[0]);
      qt_safe_close(startedPipe[1]);
      return false;
   }

   pid_t childPid = fork();
   if (childPid == 0) {
      struct sigaction noaction;
      memset(&noaction, 0, sizeof(noaction));
      noaction.sa_handler = SIG_IGN;
      ::sigaction(SIGPIPE, &noaction, 0);

      ::setsid();

      qt_safe_close(startedPipe[0]);
      qt_safe_close(pidPipe[0]);

      pid_t doubleForkPid = fork();
      if (doubleForkPid == 0) {
         qt_safe_close(pidPipe[1]);

         if (!encodedWorkingDirectory.isEmpty()) {
            QT_CHDIR(encodedWorkingDirectory.constData());
         }

         char **argv = new char *[arguments.size() + 2];
         for (int i = 0; i < arguments.size(); ++i) {
#ifdef Q_OS_MAC
            argv[i + 1] = ::strdup(arguments.at(i).toUtf8().constData());
#else
            argv[i + 1] = ::strdup(arguments.at(i).toLocal8Bit().constData());
#endif
         }
         argv[arguments.size() + 1] = 0;

         if (!program.contains(QLatin1Char('/'))) {
            const QString path = QString::fromLocal8Bit(::getenv("PATH"));
            if (!path.isEmpty()) {
               QStringList pathEntries = path.split(QLatin1Char(':'));
               for (int k = 0; k < pathEntries.size(); ++k) {
                  QByteArray tmp = QFile::encodeName(pathEntries.at(k));
                  if (!tmp.endsWith('/')) {
                     tmp += '/';
                  }
                  tmp += QFile::encodeName(program);
                  argv[0] = tmp.data();
                  qt_safe_execv(argv[0], argv);
               }
            }
         } else {
            QByteArray tmp = QFile::encodeName(program);
            argv[0] = tmp.data();
            qt_safe_execv(argv[0], argv);
         }

         struct sigaction noaction;
         memset(&noaction, 0, sizeof(noaction));
         noaction.sa_handler = SIG_IGN;
         ::sigaction(SIGPIPE, &noaction, 0);

         // '\1' means execv failed
         char c = '\1';
         qt_safe_write(startedPipe[1], &c, 1);
         qt_safe_close(startedPipe[1]);
         ::_exit(1);
      } else if (doubleForkPid == -1) {
         struct sigaction noaction;
         memset(&noaction, 0, sizeof(noaction));
         noaction.sa_handler = SIG_IGN;
         ::sigaction(SIGPIPE, &noaction, 0);

         // '\2' means internal error
         char c = '\2';
         qt_safe_write(startedPipe[1], &c, 1);
      }

      qt_safe_close(startedPipe[1]);
      qt_safe_write(pidPipe[1], (const char *)&doubleForkPid, sizeof(pid_t));
      QT_CHDIR("/");
      ::_exit(1);
   }

   qt_safe_close(startedPipe[1]);
   qt_safe_close(pidPipe[1]);

   if (childPid == -1) {
      qt_safe_close(startedPipe[0]);
      qt_safe_close(pidPipe[0]);
      return false;
   }

   char reply = '\0';
   int startResult = qt_safe_read(startedPipe[0], &reply, 1);
   int result;
   qt_safe_close(startedPipe[0]);
   qt_safe_waitpid(childPid, &result, 0);
   bool success = (startResult != -1 && reply == '\0');
   if (success && pid) {
      pid_t actualPid = 0;
      if (qt_safe_read(pidPipe[0], (char *)&actualPid, sizeof(pid_t)) == sizeof(pid_t)) {
         *pid = actualPid;
      } else {
         *pid = 0;
      }
   }
   qt_safe_close(pidPipe[0]);
   return success;
}

void QProcessPrivate::initializeProcessManager()
{
   (void) processManager();
}

QT_END_NAMESPACE

#endif // QT_NO_PROCESS
