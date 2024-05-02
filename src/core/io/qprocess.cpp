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

#include <qprocess.h>

#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qelapsedtimer.h>
#include <qsocketnotifier.h>
#include <qstring.h>
#include <qtimer.h>

#include <qprocess_p.h>

#ifdef Q_OS_WIN
#include <qwineventnotifier.h>
#else
#include <qcore_unix_p.h>
#endif

#include <ctype.h>
#include <errno.h>

#if defined(CS_SHOW_DEBUG_CORE_IO)
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
   if (! data) {
      return "(null)";
   }

   QByteArray out;

   for (int i = 0; i < len && i < maxSize; ++i) {
      char c = data[i];

      if (isprint(c)) {
         out += c;

      } else {
         switch (c) {
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
               char buf[5];
               std::snprintf(buf, sizeof(buf), "\\%3o", c);
               buf[4] = '\0';
               out += QByteArray(buf);
         }
      }
   }

   if (len < maxSize) {
      out += "...";
   }

   return out;
}
#endif

#ifndef QT_NO_PROCESS

QStringList QProcessEnvironmentPrivate::toList() const
{
   QStringList result;

   auto it  = hash.constBegin();
   auto end = hash.constEnd();

   for ( ; it != end; ++it) {
      result << nameToString(it.key()) + '=' + valueToString(it.value());
   }

   return result;
}

QProcessEnvironment QProcessEnvironmentPrivate::fromList(const QStringList &list)
{
   QProcessEnvironment env;

   auto it  = list.constBegin();
   auto end = list.constEnd();

   for ( ; it != end; ++it) {
      int pos = it->indexOf('=', 1);

      if (pos < 1) {
         continue;
      }

      QString value = it->mid(pos + 1);
      QString name  = *it;

      name.truncate(pos);
      env.insert(name, value);
   }

   return env;
}

QStringList QProcessEnvironmentPrivate::keys() const
{
   QStringList result;

   auto it = hash.constBegin();
   auto end = hash.constEnd();

   for ( ; it != end; ++it) {
      result << nameToString(it.key());
   }

   return result;
}

void QProcessEnvironmentPrivate::insert(const QProcessEnvironmentPrivate &other)
{
   auto it = other.hash.constBegin();
   auto end = other.hash.constEnd();

   for ( ; it != end; ++it) {
      hash.insert(it.key(), it.value());
   }

#ifdef Q_OS_UNIX
   auto nit  = other.nameMap.constBegin();
   auto nend = other.nameMap.constEnd();

   for ( ; nit != nend; ++nit) {
      nameMap.insert(nit.key(), nit.value());
   }

#endif
}

QProcessEnvironment::QProcessEnvironment()
   : d(nullptr)
{
}

QProcessEnvironment::~QProcessEnvironment()
{
}

QProcessEnvironment::QProcessEnvironment(const QProcessEnvironment &other)
   : d(other.d)
{
}

QProcessEnvironment &QProcessEnvironment::operator=(const QProcessEnvironment &other)
{
   d = other.d;
   return *this;
}

bool QProcessEnvironment::operator==(const QProcessEnvironment &other) const
{
   if (d == other.d) {
      return true;
   }

   if (d) {
      if (other.d) {
         QProcessEnvironmentPrivate::OrderedMutexLocker locker(d, other.d);
         return d->hash == other.d->hash;
      } else {
         return isEmpty();
      }

   } else {
      return other.isEmpty();
   }
}

bool QProcessEnvironment::isEmpty() const
{
   // Needs no locking since no hash nodes are accessed
   return d ? d->hash.isEmpty() : true;
}

void QProcessEnvironment::clear()
{
   if (d) {
      d->hash.clear();
   }

   // Unix: Don't clear d->nameMap, as the environment is likely to be
   // re-populated with the same keys again.
}

bool QProcessEnvironment::contains(const QString &name) const
{
   if (!d) {
      return false;
   }

   QProcessEnvironmentPrivate::MutexLocker locker(d);
   return d->hash.contains(d->prepareName(name));
}

void QProcessEnvironment::insert(const QString &name, const QString &value)
{
   // our re-impl of detach() detaches from null
   d.detach(); // detach before prepareName()
   d->hash.insert(d->prepareName(name), d->prepareValue(value));
}

void QProcessEnvironment::remove(const QString &name)
{
   if (d) {
      d.detach(); // detach before prepareName()
      d->hash.remove(d->prepareName(name));
   }
}

QString QProcessEnvironment::value(const QString &name, const QString &defaultValue) const
{
   if (!d) {
      return defaultValue;
   }

   QProcessEnvironmentPrivate::MutexLocker locker(d);
   auto it = d->hash.constFind(d->prepareName(name));

   if (it == d->hash.constEnd()) {
      return defaultValue;
   }

   return d->valueToString(it.value());
}

QStringList QProcessEnvironment::toStringList() const
{
   if (! d) {
      return QStringList();
   }

   QProcessEnvironmentPrivate::MutexLocker locker(d);
   return d->toList();
}

QStringList QProcessEnvironment::keys() const
{
   if (! d) {
      return QStringList();
   }

   QProcessEnvironmentPrivate::MutexLocker locker(d);
   return d->keys();
}

void QProcessEnvironment::insert(const QProcessEnvironment &e)
{
   if (! e.d) {
      return;
   }

   // our re-impl of detach() detaches from null
   QProcessEnvironmentPrivate::MutexLocker locker(e.d);
   d->insert(*e.d);
}

void QProcessPrivate::Channel::clear()
{
   switch (type) {
      case PipeSource:
         Q_ASSERT(process);
         process->stdinChannel.type = Normal;
         process->stdinChannel.process = nullptr;
         break;

      case PipeSink:
         Q_ASSERT(process);
         process->stdoutChannel.type = Normal;
         process->stdoutChannel.process = nullptr;
         break;
   }

   type = Normal;
   file.clear();
   process = nullptr;
}

QProcessPrivate::QProcessPrivate()
{
   processChannel = QProcess::StandardOutput;
   processChannelMode = QProcess::SeparateChannels;
   inputChannelMode = QProcess::ManagedInputChannel;
   processError = QProcess::UnknownError;
   processState = QProcess::NotRunning;

#ifdef Q_OS_WIN
   pid = nullptr;
#else
   pid = 0;
#endif

   sequenceNumber = 0;
   exitCode   = 0;
   exitStatus = QProcess::NormalExit;
   startupSocketNotifier = nullptr;
   deathNotifier = nullptr;

   childStartedPipe[0] = INVALID_Q_PIPE;
   childStartedPipe[1] = INVALID_Q_PIPE;

   forkfd   = -1;
   exitCode = 0;
   crashed  = false;
   dying    = false;
   emittedReadyRead    = false;
   emittedBytesWritten = false;

#ifdef Q_OS_WIN
   stdinWriteTrigger = nullptr;
   processFinishedNotifier = nullptr;
#endif

}

QProcessPrivate::~QProcessPrivate()
{
   if (stdinChannel.process) {
      stdinChannel.process->stdoutChannel.clear();
   }

   if (stdoutChannel.process) {
      stdoutChannel.process->stdinChannel.clear();
   }
}

void QProcessPrivate::cleanup()
{
   q_func()->setProcessState(QProcess::NotRunning);

#ifdef Q_OS_WIN

   if (pid) {
      CloseHandle(pid->hThread);
      CloseHandle(pid->hProcess);
      delete pid;
      pid = nullptr;
   }

   if (stdinWriteTrigger) {
      delete stdinWriteTrigger;
      stdinWriteTrigger = nullptr;
   }

   if (processFinishedNotifier) {
      delete processFinishedNotifier;
      processFinishedNotifier = nullptr;
   }

#endif

#ifdef Q_OS_WIN
   pid = nullptr;
#else
   pid = 0;
#endif

   sequenceNumber = 0;
   dying = false;

   if (stdoutChannel.notifier) {
      delete  stdoutChannel.notifier;
      stdoutChannel.notifier = nullptr;
   }

   if (stderrChannel.notifier) {
      delete stderrChannel.notifier;
      stderrChannel.notifier = nullptr;
   }

   if (stdinChannel.notifier) {
      delete stdinChannel.notifier;
      stdinChannel.notifier = nullptr;
   }

   if (startupSocketNotifier) {
      delete startupSocketNotifier;
      startupSocketNotifier = nullptr;
   }

   if (deathNotifier) {
      delete deathNotifier;
      deathNotifier = nullptr;
   }

   closeChannel(&stdoutChannel);
   closeChannel(&stderrChannel);
   closeChannel(&stdinChannel);
   destroyPipe(childStartedPipe);

#ifdef Q_OS_UNIX

   if (forkfd != -1) {
      qt_safe_close(forkfd);
   }

   forkfd = -1;
#endif
}

void QProcessPrivate::setError(QProcess::ProcessError error, const QString &description)
{
   processError = error;

   if (description.isEmpty()) {
      switch (error) {
         case QProcess::FailedToStart:
            errorString = QProcess::tr("Process failed to start");
            break;

         case QProcess::Crashed:
            errorString = QProcess::tr("Process crashed");
            break;

         case QProcess::Timedout:
            errorString = QProcess::tr("Process operation timed out");
            break;

         case QProcess::ReadError:
            errorString = QProcess::tr("Error reading from process");
            break;

         case QProcess::WriteError:
            errorString = QProcess::tr("Error writing to process");
            break;

         case QProcess::UnknownError:
            errorString.clear();
            break;
      }

   } else {
      errorString = description;
   }
}

void QProcessPrivate::setErrorAndEmit(QProcess::ProcessError error, const QString &description)
{
   Q_Q(QProcess);
   Q_ASSERT(error != QProcess::UnknownError);

   setError(error, description);
   emit q->errorOccurred(processError);
}

bool QProcessPrivate::tryReadFromChannel(Channel *channel)
{
   Q_Q(QProcess);

   if (channel->pipe[0] == INVALID_Q_PIPE) {
      return false;
   }

   qint64 available = bytesAvailableInChannel(channel);

   if (available == 0) {
      available = 1;   // always try to read at least one byte
   }

   char *ptr = channel->buffer.reserve(available);
   qint64 readBytes = readFromChannel(channel, ptr, available);

   if (readBytes <= 0) {
      channel->buffer.chop(available);
   }

   if (readBytes == -2) {
      // EWOULDBLOCK
      return false;
   }

   if (readBytes == -1) {
      setErrorAndEmit(QProcess::ReadError);

#if defined(CS_SHOW_DEBUG_CORE_IO)
      qDebug("QProcessPrivate::tryReadFromChannel(%lld), failed to read from the process", channel - &stdinChannel);
#endif

      return false;
   }

   if (readBytes == 0) {

      if (channel->notifier) {
         channel->notifier->setEnabled(false);
      }

      closeChannel(channel);

#if defined(CS_SHOW_DEBUG_CORE_IO)
      qDebug("QProcessPrivate::tryReadFromChannel(%lld), 0 bytes available", channel - &stdinChannel);
#endif
      return false;
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcessPrivate::tryReadFromChannel(%lld), read %d bytes from the process output",
         channel - &stdinChannel, int(readBytes));
#endif

   if (channel->closed) {
      channel->buffer.chop(readBytes);
      return false;
   }

   channel->buffer.chop(available - readBytes);

   bool didRead = false;
   bool isStdout = channel == &stdoutChannel;

   if (readBytes == 0) {
      if (channel->notifier) {
         channel->notifier->setEnabled(false);
      }
   } else if ((processChannel == QProcess::StandardOutput) == isStdout) {
      didRead = true;

      if (!emittedReadyRead) {
         emittedReadyRead = true;
         emit q->readyRead();
         emittedReadyRead = false;
      }
   }

   if (isStdout) {
      emit q->readyReadStandardOutput();
   } else {
      emit q->readyReadStandardError();
   }

   return didRead;
}

bool QProcessPrivate::_q_canReadStandardOutput()
{
   return tryReadFromChannel(&stdoutChannel);
}

bool QProcessPrivate::_q_canReadStandardError()
{
   return tryReadFromChannel(&stderrChannel);
}

bool QProcessPrivate::_q_canWrite()
{
   if (stdinChannel.notifier) {
      stdinChannel.notifier->setEnabled(false);
   }

   if (stdinChannel.buffer.isEmpty()) {

#if defined(CS_SHOW_DEBUG_CORE_IO)
      qDebug("QProcessPrivate::canWrite() Not writing anything (empty write buffer)");
#endif

      return false;
   }

   const bool writeSucceeded = writeToStdin();

   if (stdinChannel.notifier && !stdinChannel.buffer.isEmpty()) {
      stdinChannel.notifier->setEnabled(true);
   }

   if (stdinChannel.buffer.isEmpty() && stdinChannel.closed) {
      closeWriteChannel();
   }

   return writeSucceeded;
}

bool QProcessPrivate::_q_processDied()
{
   Q_Q(QProcess);

#ifdef Q_OS_UNIX

   if (! waitForDeadChild()) {
      return false;
   }

#endif

#ifdef Q_OS_WIN

   if (processFinishedNotifier) {
      processFinishedNotifier->setEnabled(false);
   }

   drainOutputPipes();
#endif

   // the process may have died before it got a chance to report that it was
   // either running or stopped, so we will call _q_startupNotification() and
   // give it a chance to emit started() or error(FailedToStart).
   if (processState == QProcess::Starting) {
      if (!_q_startupNotification()) {
         return true;
      }
   }

   if (dying) {
      // at this point we know the process is dead. prevent
      // reentering this slot recursively by calling waitForFinished()
      // or opening a dialog inside slots connected to the readyRead
      // signals emitted below.
      return true;
   }

   dying = true;

   // in case there is data in the pipe line and this slot by chance
   // got called before the read notifications, call these two slots
   // so the data is made available before the process dies.
   _q_canReadStandardOutput();
   _q_canReadStandardError();

   findExitCode();

   if (crashed) {
      exitStatus = QProcess::CrashExit;
      setErrorAndEmit(QProcess::Crashed);
   }

   bool wasRunning = (processState == QProcess::Running);

   cleanup();

   if (wasRunning) {
      // we received EOF now:
      emit q->readChannelFinished();
      // in the future:
      //emit q->standardOutputClosed();
      //emit q->standardErrorClosed();

      emit q->finished(exitCode);
      emit q->finished(exitCode, exitStatus);
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcessPrivate::_q_processDied() process is dead");
#endif

   return true;
}

bool QProcessPrivate::_q_startupNotification()
{
   Q_Q(QProcess);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcessPrivate::startupNotification()");
#endif

   if (startupSocketNotifier) {
      startupSocketNotifier->setEnabled(false);
   }

   QString errorMessage;

   if (processStarted(&errorMessage)) {
      q->setProcessState(QProcess::Running);
      emit q->started();
      return true;
   }

   q->setProcessState(QProcess::NotRunning);
   setErrorAndEmit(QProcess::FailedToStart, errorMessage);

#ifdef Q_OS_UNIX
   // make sure the process manager removes this entry
   waitForDeadChild();
   findExitCode();
#endif

   cleanup();

   return false;
}

bool QProcess::_q_canReadStandardOutput()
{
   Q_D(QProcess);
   return d->_q_canReadStandardOutput();
}

bool QProcess::_q_canReadStandardError()
{
   Q_D(QProcess);
   return d->_q_canReadStandardError();
}

bool QProcess::_q_canWrite()
{
   Q_D(QProcess);
   return d->_q_canWrite();
}

bool QProcess::_q_startupNotification()
{
   Q_D(QProcess);
   return d->_q_startupNotification();
}

bool QProcess::_q_processDied()
{
   Q_D(QProcess);
   return d->_q_processDied();
}

void QProcessPrivate::closeWriteChannel()
{
#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcessPrivate::closeWriteChannel()");
#endif

   if (stdinChannel.notifier) {
      delete stdinChannel.notifier;
      stdinChannel.notifier = nullptr;
   }

#ifdef Q_OS_WIN
   // ### Find a better fix, feeding the process little by little instead
   flushPipeWriter();
#endif

   closeChannel(&stdinChannel);
}

QProcess::QProcess(QObject *parent)
   : QIODevice(*new QProcessPrivate, parent)
{
#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcess::QProcess(%p)", parent);
#endif
}

QProcess::~QProcess()
{
   Q_D(QProcess);

   if (d->processState != NotRunning) {
      qWarning("QProcess() Destroyed while a process was running");
      kill();
      waitForFinished();
   }

#ifdef Q_OS_UNIX
   // make sure the process manager removes this entry
   d->findExitCode();
#endif

   d->cleanup();
}

QProcess::ProcessChannelMode QProcess::readChannelMode() const
{
   return processChannelMode();
}

void QProcess::setReadChannelMode(ProcessChannelMode mode)
{
   setProcessChannelMode(mode);
}

QProcess::ProcessChannelMode QProcess::processChannelMode() const
{
   Q_D(const QProcess);
   return d->processChannelMode;
}

void QProcess::setProcessChannelMode(ProcessChannelMode mode)
{
   Q_D(QProcess);
   d->processChannelMode = mode;
}

QProcess::InputChannelMode QProcess::inputChannelMode() const
{
   Q_D(const QProcess);
   return d->inputChannelMode;
}

void QProcess::setInputChannelMode(InputChannelMode mode)
{
   Q_D(QProcess);
   d->inputChannelMode = mode;
}

QProcess::ProcessChannel QProcess::readChannel() const
{
   Q_D(const QProcess);
   return d->processChannel;
}

void QProcess::setReadChannel(ProcessChannel channel)
{
   Q_D(QProcess);

   if (d->processChannel != channel) {
      QByteArray buf = d->buffer.readAll();

      if (d->processChannel == QProcess::StandardOutput) {
         for (int i = buf.size() - 1; i >= 0; --i) {
            d->stdoutChannel.buffer.ungetChar(buf.at(i));
         }

      } else {
         for (int i = buf.size() - 1; i >= 0; --i) {
            d->stderrChannel.buffer.ungetChar(buf.at(i));
         }
      }
   }

   d->processChannel = channel;
}

void QProcess::closeReadChannel(ProcessChannel channel)
{
   Q_D(QProcess);

   if (channel == StandardOutput) {
      d->stdoutChannel.closed = true;
   } else {
      d->stderrChannel.closed = true;
   }
}

void QProcess::closeWriteChannel()
{
   Q_D(QProcess);
   d->stdinChannel.closed = true; // closing

   if (d->stdinChannel.buffer.isEmpty()) {
      d->closeWriteChannel();
   }
}

void QProcess::setStandardInputFile(const QString &fileName)
{
   Q_D(QProcess);
   d->stdinChannel = fileName;
}

void QProcess::setStandardOutputFile(const QString &fileName, OpenMode mode)
{
   Q_ASSERT(mode == Append || mode == Truncate);
   Q_D(QProcess);

   d->stdoutChannel = fileName;
   d->stdoutChannel.append = mode == Append;
}

void QProcess::setStandardErrorFile(const QString &fileName, OpenMode mode)
{
   Q_ASSERT(mode == Append || mode == Truncate);
   Q_D(QProcess);

   d->stderrChannel = fileName;
   d->stderrChannel.append = mode == Append;
}

void QProcess::setStandardOutputProcess(QProcess *destination)
{
   QProcessPrivate *dfrom = d_func();
   QProcessPrivate *dto = destination->d_func();
   dfrom->stdoutChannel.pipeTo(dto);
   dto->stdinChannel.pipeFrom(dfrom);
}

#if defined(Q_OS_WIN)

QString QProcess::nativeArguments() const
{
   Q_D(const QProcess);
   return d->nativeArguments;
}

void QProcess::setNativeArguments(const QString &arguments)
{
   Q_D(QProcess);
   d->nativeArguments = arguments;
}
#endif

QString QProcess::workingDirectory() const
{
   Q_D(const QProcess);
   return d->workingDirectory;
}

void QProcess::setWorkingDirectory(const QString &dir)
{
   Q_D(QProcess);
   d->workingDirectory = dir;
}

Q_PID QProcess::pid() const
{
   Q_D(const QProcess);
   return d->pid;
}

qint64 QProcess::processId() const
{
   Q_D(const QProcess);

#ifdef Q_OS_WIN
   return d->pid ? d->pid->dwProcessId : 0;
#else
   return d->pid;
#endif
}

bool QProcess::canReadLine() const
{
   Q_D(const QProcess);

   const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
         ? &d->stderrChannel.buffer : &d->stdoutChannel.buffer;

   return readBuffer->canReadLine() || QIODevice::canReadLine();
}

void QProcess::close()
{
   emit aboutToClose();

   while (waitForBytesWritten(-1)) {
      ;
   }

   kill();
   waitForFinished(-1);
   QIODevice::close();
}

bool QProcess::atEnd() const
{
   Q_D(const QProcess);

   const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
         ? &d->stderrChannel.buffer : &d->stdoutChannel.buffer;

   return QIODevice::atEnd() && (! isOpen() || readBuffer->isEmpty());
}

bool QProcess::isSequential() const
{
   return true;
}

qint64 QProcess::bytesAvailable() const
{
   Q_D(const QProcess);

   const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
         ? &d->stderrChannel.buffer : &d->stdoutChannel.buffer;

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcess::bytesAvailable() == %i (%s)", readBuffer->size(),
         (d->processChannel == QProcess::StandardError) ? "stderr" : "stdout");
#endif

   return readBuffer->size() + QIODevice::bytesAvailable();
}

qint64 QProcess::bytesToWrite() const
{
   Q_D(const QProcess);
   qint64 size = d->stdinChannel.buffer.size();

#ifdef Q_OS_WIN
   size += d->pipeWriterBytesToWrite();
#endif

   return size;
}

QProcess::ProcessError QProcess::error() const
{
   Q_D(const QProcess);
   return d->processError;
}

QProcess::ProcessState QProcess::state() const
{
   Q_D(const QProcess);
   return d->processState;
}

void QProcess::setEnvironment(const QStringList &environment)
{
   setProcessEnvironment(QProcessEnvironmentPrivate::fromList(environment));
}

QStringList QProcess::environment() const
{
   Q_D(const QProcess);
   return d->environment.toStringList();
}

void QProcess::setProcessEnvironment(const QProcessEnvironment &environment)
{
   Q_D(QProcess);
   d->environment = environment;
}

QProcessEnvironment QProcess::processEnvironment() const
{
   Q_D(const QProcess);
   return d->environment;
}

bool QProcess::waitForStarted(int msecs)
{
   Q_D(QProcess);

   if (d->processState == QProcess::Starting) {
      return d->waitForStarted(msecs);
   }

   return d->processState == QProcess::Running;
}

bool QProcess::waitForReadyRead(int msecs)
{
   Q_D(QProcess);

   if (d->processState == QProcess::NotRunning) {
      return false;
   }

   if (d->processChannel == QProcess::StandardOutput && d->stdoutChannel.closed) {
      return false;
   }

   if (d->processChannel == QProcess::StandardError && d->stderrChannel.closed) {
      return false;
   }

   return d->waitForReadyRead(msecs);
}

bool QProcess::waitForBytesWritten(int msecs)
{
   Q_D(QProcess);

   if (d->processState == QProcess::NotRunning) {
      return false;
   }

   if (d->processState == QProcess::Starting) {
      QElapsedTimer stopWatch;
      stopWatch.start();

      bool started = waitForStarted(msecs);

      if (! started) {
         return false;
      }

      msecs = qt_subtract_from_timeout(msecs, stopWatch.elapsed());
   }

   return d->waitForBytesWritten(msecs);
}

bool QProcess::waitForFinished(int msecs)
{
   Q_D(QProcess);

   if (d->processState == QProcess::NotRunning) {
      return false;
   }

   if (d->processState == QProcess::Starting) {
      QElapsedTimer stopWatch;
      stopWatch.start();

      bool started = waitForStarted(msecs);

      if (!started) {
         return false;
      }

      msecs = qt_subtract_from_timeout(msecs, stopWatch.elapsed());
   }

   return d->waitForFinished(msecs);
}

void QProcess::setProcessState(ProcessState state)
{
   Q_D(QProcess);

   if (d->processState == state) {
      return;
   }

   d->processState = state;
   emit stateChanged(state);
}

void QProcess::setupChildProcess()
{
}

qint64 QProcess::readData(char *data, qint64 maxlen)
{
   Q_D(QProcess);

   if (! maxlen) {
      return 0;
   }

   QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
         ? &d->stderrChannel.buffer : &d->stdoutChannel.buffer;

   if (maxlen == 1 && !readBuffer->isEmpty()) {
      int c = readBuffer->getChar();

      if (c == -1) {

#if defined(CS_SHOW_DEBUG_CORE_IO)
         qDebug("QProcess::readData(%p \"%s\", %d) == -1",
               data, qt_prettyDebug(data, 1, maxlen).constData(), 1);
#endif

         return -1;
      }

      *data = (char) c;

#if defined(CS_SHOW_DEBUG_CORE_IO)
      qDebug("QProcess::readData(%p \"%s\", %d) == 1",
            data, qt_prettyDebug(data, 1, maxlen).constData(), 1);
#endif

      return 1;
   }

   qint64 bytesToRead = qint64(qMin(readBuffer->size(), maxlen));
   qint64 readSoFar = 0;

   while (readSoFar < bytesToRead) {
      const char *ptr = readBuffer->readPointer();
      qint64 bytesToReadFromThisBlock = qMin(bytesToRead - readSoFar, readBuffer->nextDataBlockSize());

      memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
      readSoFar += bytesToReadFromThisBlock;
      readBuffer->free(bytesToReadFromThisBlock);
   }

   if (! readSoFar && d->processState == QProcess::NotRunning) {
      return -1;   // EOF
   }

   return readSoFar;
}

qint64 QProcess::writeData(const char *data, qint64 len)
{
   Q_D(QProcess);

   if (d->stdinChannel.closed) {
      return 0;
   }

#if defined(Q_OS_WIN)

   if (! d->stdinWriteTrigger) {
      d->stdinWriteTrigger = new QTimer;
      d->stdinWriteTrigger->setSingleShot(true);

      connect(d->stdinWriteTrigger, &QTimer::timeout, this,
         [d]() {
            d->_q_canWrite();
         } );
   }

#endif

   if (len == 1) {
      d->stdinChannel.buffer.putChar(*data);

#ifdef Q_OS_WIN

      if (! d->stdinWriteTrigger->isActive()) {
         d->stdinWriteTrigger->start();
      }

#else

      if (d->stdinChannel.notifier) {
         d->stdinChannel.notifier->setEnabled(true);
      }

#endif

      return 1;
   }

   char *dest = d->stdinChannel.buffer.reserve(len);
   memcpy(dest, data, len);

#ifdef Q_OS_WIN

   if (! d->stdinWriteTrigger->isActive()) {
      d->stdinWriteTrigger->start();
   }

#else

   if (d->stdinChannel.notifier) {
      d->stdinChannel.notifier->setEnabled(true);
   }

#endif

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcess::writeData(%p \"%s\", %lld) == %lld (written to buffer)",
         data, qt_prettyDebug(data, len, 16).constData(), len, len);
#endif

   return len;
}

QByteArray QProcess::readAllStandardOutput()
{
   ProcessChannel tmp = readChannel();
   setReadChannel(StandardOutput);

   QByteArray data = readAll();
   setReadChannel(tmp);

   return data;
}

QByteArray QProcess::readAllStandardError()
{
   ProcessChannel tmp = readChannel();
   setReadChannel(StandardError);
   QByteArray data = readAll();
   setReadChannel(tmp);

   return data;
}

void QProcess::start(const QString &program, const QStringList &arguments, OpenMode mode)
{
   Q_D(QProcess);

   if (d->processState != NotRunning) {
      qWarning("QProcess::start() Process is already running");
      return;
   }

   if (program.isEmpty()) {
      d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
      return;
   }

   d->program = program;
   d->arguments = arguments;
   d->start(mode);
}

void QProcess::start(OpenMode mode)
{
   Q_D(QProcess);

   if (d->processState != NotRunning) {
      qWarning("QProcess::start() Process is already running");
      return;
   }

   if (d->program.isEmpty()) {
      d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
      return;
   }

   d->start(mode);
}

bool QProcess::open(OpenMode mode)
{
   Q_D(QProcess);

   if (d->processState != NotRunning) {
      qWarning("QProcess::start() Process is already running");
      return false;
   }

   if (d->program.isEmpty()) {
      qWarning("QProcess::start() No program was specified");
      return false;
   }

   d->start(mode);
   return true;
}

void QProcessPrivate::start(QIODevice::OpenMode mode)
{
   Q_Q(QProcess);

   stdinChannel.buffer.clear();
   stdoutChannel.buffer.clear();
   stderrChannel.buffer.clear();

   if (stdinChannel.type != QProcessPrivate::Channel::Normal) {
      mode &= ~QIODevice::WriteOnly;   // not open for writing
   }

   if (stdoutChannel.type != QProcessPrivate::Channel::Normal &&
         (stderrChannel.type != QProcessPrivate::Channel::Normal ||
               processChannelMode == QProcess::MergedChannels)) {
      mode &= ~QIODevice::ReadOnly;   // not open for reading
   }

   if (mode == 0) {
      mode = QIODevice::Unbuffered;
   }

   if ((mode & QIODevice::ReadOnly) == 0) {
      if (stdoutChannel.type == QProcessPrivate::Channel::Normal) {
         q->setStandardOutputFile(q->nullDevice());
      }

      if (stderrChannel.type == QProcessPrivate::Channel::Normal
            && processChannelMode != QProcess::MergedChannels) {
         q->setStandardErrorFile(q->nullDevice());
      }
   }

   q->QIODevice::open(mode);

   stdinChannel.closed = false;
   stdoutChannel.closed = false;
   stderrChannel.closed = false;

   exitCode = 0;
   exitStatus = QProcess::NormalExit;
   processError = QProcess::UnknownError;
   errorString.clear();
   startProcess();
}

static QStringList parseCombinedArgString(const QString &program)
{
   QStringList args;
   QString tmp;
   int quoteCount = 0;
   bool inQuote = false;

   // handle quoting. tokens can be surrounded by double quotes
   // "hello world". three consecutive double quotes represent
   // the quote character itself.

   for (int i = 0; i < program.size(); ++i) {
      if (program.at(i) == '"') {
         ++quoteCount;

         if (quoteCount == 3) {
            // third consecutive quote
            quoteCount = 0;
            tmp += program.at(i);
         }

         continue;
      }

      if (quoteCount) {
         if (quoteCount == 1) {
            inQuote = !inQuote;
         }

         quoteCount = 0;
      }

      if (!inQuote && program.at(i).isSpace()) {
         if (!tmp.isEmpty()) {
            args += tmp;
            tmp.clear();
         }

      } else {
         tmp += program.at(i);
      }
   }

   if (!tmp.isEmpty()) {
      args += tmp;
   }

   return args;
}

void QProcess::start(const QString &program, OpenMode mode)
{
   QStringList args = parseCombinedArgString(program);

   if (args.isEmpty()) {
      Q_D(QProcess);

      d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
      return;
   }

   QString prog = args.first();
   args.removeFirst();

   start(prog, args, mode);
}

QString QProcess::program() const
{
   Q_D(const QProcess);
   return d->program;
}

void QProcess::setProgram(const QString &program)
{
   Q_D(QProcess);

   if (d->processState != NotRunning) {
      qWarning("QProcess::setProgram() Process is already running");
      return;
   }

   d->program = program;
}

QStringList QProcess::arguments() const
{
   Q_D(const QProcess);
   return d->arguments;
}

void QProcess::setArguments(const QStringList &arguments)
{
   Q_D(QProcess);

   if (d->processState != NotRunning) {
      qWarning("QProcess::setProgram() Process is already running");
      return;
   }

   d->arguments = arguments;
}

void QProcess::terminate()
{
   Q_D(QProcess);
   d->terminateProcess();
}

void QProcess::kill()
{
   Q_D(QProcess);
   d->killProcess();
}

int QProcess::exitCode() const
{
   Q_D(const QProcess);
   return d->exitCode;
}

QProcess::ExitStatus QProcess::exitStatus() const
{
   Q_D(const QProcess);
   return d->exitStatus;
}

int QProcess::execute(const QString &program, const QStringList &arguments)
{
   QProcess process;
   process.setReadChannelMode(ForwardedChannels);
   process.start(program, arguments);

   if (! process.waitForFinished(-1) || process.error() == FailedToStart) {
      return -2;
   }

   return process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
}

int QProcess::execute(const QString &program)
{
   QProcess process;
   process.setReadChannelMode(ForwardedChannels);
   process.start(program);

   if (! process.waitForFinished(-1) || process.error() == FailedToStart) {
      return -2;
   }

   return process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
}

bool QProcess::startDetached(const QString &program, const QStringList &arguments,
      const QString &workingDirectory, qint64 *pid)
{
   return QProcessPrivate::startDetached(program, arguments, workingDirectory, pid);
}

bool QProcess::startDetached(const QString &program)
{
   QStringList args = parseCombinedArgString(program);

   if (args.isEmpty()) {
      return false;
   }

   QString prog = args.first();
   args.removeFirst();

   return QProcessPrivate::startDetached(prog, args);
}

#ifdef Q_OS_DARWIN
#include <crt_externs.h>
#define environ (*_NSGetEnviron())

#elif ! defined(Q_OS_WIN)
extern char **environ;

#endif

QStringList QProcess::systemEnvironment()
{
   QStringList tmp;
   const char *entry = nullptr;

   if (environ != nullptr) {

      for (int count = 0; (entry = environ[count]); ++count) {
         tmp << QString::fromUtf8(entry);
      }
   }

   return tmp;
}

QString QProcess::nullDevice()
{
#ifdef Q_OS_WIN
   return QString("\\\\.\\NUL");
#else
   return QString("/dev/null");
#endif
}

#endif // QT_NO_PROCESS
