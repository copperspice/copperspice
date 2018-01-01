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

//#define QPROCESS_DEBUG

#if defined QPROCESS_DEBUG
#include <qdebug.h>
#include <qstring.h>
#include <ctype.h>
#include <errno.h>

static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
   if (!data) {
      return "(null)";
   }
   QByteArray out;
   for (int i = 0; i < len && i < maxSize; ++i) {
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
               char buf[5];
               qsnprintf(buf, sizeof(buf), "\\%3o", c);
               buf[4] = '\0';
               out += QByteArray(buf);
         }
   }

   if (len < maxSize) {
      out += "...";
   }

   return out;
}

QT_END_NAMESPACE

#endif

#include <qprocess.h>
#include <qprocess_p.h>

#include <qbytearray.h>
#include <qelapsedtimer.h>
#include <qcoreapplication.h>
#include <qsocketnotifier.h>
#include <qtimer.h>

#ifdef Q_OS_WIN
#include <qwineventnotifier_p.h>
#endif

#ifndef QT_NO_PROCESS

QStringList QProcessEnvironmentPrivate::toList() const
{
   QStringList result;
   result.reserve(hash.size());
   Hash::ConstIterator it = hash.constBegin(), end = hash.constEnd();

   for ( ; it != end; ++it) {
      QString data = nameToString(it.key());
      QString value = valueToString(it.value());
      data.reserve(data.length() + value.length() + 1);
      data.append(QLatin1Char('='));
      data.append(value);
      result << data;
   }
   return result;
}

QProcessEnvironment QProcessEnvironmentPrivate::fromList(const QStringList &list)
{
   QProcessEnvironment env;
   QStringList::ConstIterator it = list.constBegin(),
                              end = list.constEnd();
   for ( ; it != end; ++it) {
      int pos = it->indexOf(QLatin1Char('='));
      if (pos < 1) {
         continue;
      }

      QString value = it->mid(pos + 1);
      QString name = *it;
      name.truncate(pos);
      env.insert(name, value);
   }
   return env;
}

QStringList QProcessEnvironmentPrivate::keys() const
{
   QStringList result;
   result.reserve(hash.size());
   Hash::ConstIterator it = hash.constBegin(),
                       end = hash.constEnd();
   for ( ; it != end; ++it) {
      result << nameToString(it.key());
   }
   return result;
}

void QProcessEnvironmentPrivate::insert(const QProcessEnvironmentPrivate &other)
{
   Hash::ConstIterator it = other.hash.constBegin(),
                       end = other.hash.constEnd();
   for ( ; it != end; ++it) {
      hash.insert(it.key(), it.value());
   }

#ifdef Q_OS_UNIX
   QHash<QString, Key>::ConstIterator nit = other.nameMap.constBegin(),
                                      nend = other.nameMap.constEnd();
   for ( ; nit != nend; ++nit) {
      nameMap.insert(nit.key(), nit.value());
   }
#endif
}

/*!
    Creates a new QProcessEnvironment object. This constructor creates an
    empty environment. If set on a QProcess, this will cause the current
    environment variables to be removed.
*/
QProcessEnvironment::QProcessEnvironment()
   : d(0)
{
}

/*!
    Frees the resources associated with this QProcessEnvironment object.
*/
QProcessEnvironment::~QProcessEnvironment()
{
}

/*!
    Creates a QProcessEnvironment object that is a copy of \a other.
*/
QProcessEnvironment::QProcessEnvironment(const QProcessEnvironment &other)
   : d(other.d)
{
}

/*!
    Copies the contents of the \a other QProcessEnvironment object into this
    one.
*/
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
   if (d && other.d) {
      QProcessEnvironmentPrivate::OrderedMutexLocker locker(d, other.d);
      return d->hash == other.d->hash;
   }
   return false;
}

/*!
    Returns true if this QProcessEnvironment object is empty: that is
    there are no key=value pairs set.

    \sa clear(), systemEnvironment(), insert()
*/
bool QProcessEnvironment::isEmpty() const
{
   // Needs no locking, as no hash nodes are accessed
   return d ? d->hash.isEmpty() : true;
}

/*!
    Removes all key=value pairs from this QProcessEnvironment object, making
    it empty.

    \sa isEmpty(), systemEnvironment()
*/
void QProcessEnvironment::clear()
{
   if (d) {
      d->hash.clear();
   }
   // Unix: Don't clear d->nameMap, as the environment is likely to be
   // re-populated with the same keys again.
}

/*!
    Returns true if the environment variable of name \a name is found in
    this QProcessEnvironment object.

    On Windows, variable names are case-insensitive, so the key is converted
    to uppercase before searching. On other systems, names are case-sensitive
    so no trasformation is applied.

    \sa insert(), value()
*/
bool QProcessEnvironment::contains(const QString &name) const
{
   if (!d) {
      return false;
   }
   QProcessEnvironmentPrivate::MutexLocker locker(d);
   return d->hash.contains(d->prepareName(name));
}

/*!
    Inserts the environment variable of name \a name and contents \a value
    into this QProcessEnvironment object. If that variable already existed,
    it is replaced by the new value.

    On Windows, variable names are case-insensitive, so this function always
    uppercases the variable name before inserting. On other systems, names
    are case-sensitive, so no transformation is applied.

    On most systems, inserting a variable with no contents will have the
    same effect for applications as if the variable had not been set at all.
    However, to guarantee that there are no incompatibilities, to remove a
    variable, please use the remove() function.

    \sa contains(), remove(), value()
*/
void QProcessEnvironment::insert(const QString &name, const QString &value)
{
   // our re-impl of detach() detaches from null
   d.detach(); // detach before prepareName()
   d->hash.insert(d->prepareName(name), d->prepareValue(value));
}

/*!
    Removes the environment variable identified by \a name from this
    QProcessEnvironment object. If that variable did not exist before,
    nothing happens.

    On Windows, variable names are case-insensitive, so the key is converted
    to uppercase before searching. On other systems, names are case-sensitive
    so no trasformation is applied.

    \sa contains(), insert(), value()
*/
void QProcessEnvironment::remove(const QString &name)
{
   if (d) {
      d.detach(); // detach before prepareName()
      d->hash.remove(d->prepareName(name));
   }
}

/*!
    Searches this QProcessEnvironment object for a variable identified by
    \a name and returns its value. If the variable is not found in this object,
    then \a defaultValue is returned instead.

    On Windows, variable names are case-insensitive, so the key is converted
    to uppercase before searching. On other systems, names are case-sensitive
    so no trasformation is applied.

    \sa contains(), insert(), remove()
*/
QString QProcessEnvironment::value(const QString &name, const QString &defaultValue) const
{
   if (!d) {
      return defaultValue;
   }

   QProcessEnvironmentPrivate::MutexLocker locker(d);
   QProcessEnvironmentPrivate::Hash::ConstIterator it = d->hash.constFind(d->prepareName(name));
   if (it == d->hash.constEnd()) {
      return defaultValue;
   }

   return d->valueToString(it.value());
}

/*!
    Converts this QProcessEnvironment object into a list of strings, one for
    each environment variable that is set. The environment variable's name
    and its value are separated by an equal character ('=').

    The QStringList contents returned by this function are suitable for use
    with the QProcess::setEnvironment function. However, it is recommended
    to use QProcess::setProcessEnvironment instead since that will avoid
    unnecessary copying of the data.

    \sa systemEnvironment(), QProcess::systemEnvironment(), QProcess::environment(),
        QProcess::setEnvironment()
*/
QStringList QProcessEnvironment::toStringList() const
{
   if (!d) {
      return QStringList();
   }
   QProcessEnvironmentPrivate::MutexLocker locker(d);
   return d->toList();
}

/*!
    \since 4.8

    Returns a list containing all the variable names in this QProcessEnvironment
    object.
*/
QStringList QProcessEnvironment::keys() const
{
   if (!d) {
      return QStringList();
   }
   QProcessEnvironmentPrivate::MutexLocker locker(d);
   return d->keys();
}

/*!
    \overload
    \since 4.8

    Inserts the contents of \a e in this QProcessEnvironment object. Variables in
    this object that also exist in \a e will be overwritten.
*/
void QProcessEnvironment::insert(const QProcessEnvironment &e)
{
   if (!e.d) {
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
         process->stdinChannel.process = 0;
         break;
      case PipeSink:
         Q_ASSERT(process);
         process->stdoutChannel.type = Normal;
         process->stdoutChannel.process = 0;
         break;
   }

   type = Normal;
   file.clear();
   process = 0;
}

/*! \internal
*/
QProcessPrivate::QProcessPrivate()
{
   processChannel = QProcess::StandardOutput;
   processChannelMode = QProcess::SeparateChannels;
   processError = QProcess::UnknownError;
   processState = QProcess::NotRunning;
   pid = 0;
   sequenceNumber = 0;
   exitCode = 0;
   exitStatus = QProcess::NormalExit;
   startupSocketNotifier = 0;
   deathNotifier = 0;
   notifier = 0;
   pipeWriter = 0;
   childStartedPipe[0] = INVALID_Q_PIPE;
   childStartedPipe[1] = INVALID_Q_PIPE;
   deathPipe[0] = INVALID_Q_PIPE;
   deathPipe[1] = INVALID_Q_PIPE;
   exitCode = 0;
   crashed = false;
   dying = false;
   emittedReadyRead = false;
   emittedBytesWritten = false;

#ifdef Q_OS_WIN
   pipeWriter = 0;
   processFinishedNotifier = 0;
#endif

#ifdef Q_OS_UNIX
   serial = 0;
#endif

}

/*! \internal
*/
QProcessPrivate::~QProcessPrivate()
{
   if (stdinChannel.process) {
      stdinChannel.process->stdoutChannel.clear();
   }
   if (stdoutChannel.process) {
      stdoutChannel.process->stdinChannel.clear();
   }
}

/*! \internal
*/
void QProcessPrivate::cleanup()
{
   q_func()->setProcessState(QProcess::NotRunning);
#ifdef Q_OS_WIN
   if (pid) {
      CloseHandle(pid->hThread);
      CloseHandle(pid->hProcess);
      delete pid;
      pid = 0;
   }
   if (processFinishedNotifier) {
      processFinishedNotifier->setEnabled(false);

      delete processFinishedNotifier;
      processFinishedNotifier = 0;
   }

#endif
   pid = 0;
   sequenceNumber = 0;
   dying = false;

   if (stdoutChannel.notifier) {
      stdoutChannel.notifier->setEnabled(false);

      delete  stdoutChannel.notifier;
      stdoutChannel.notifier = 0;
   }

   if (stderrChannel.notifier) {
      stderrChannel.notifier->setEnabled(false);

      delete stderrChannel.notifier;
      stderrChannel.notifier = 0;
   }

   if (stdinChannel.notifier) {
      stdinChannel.notifier->setEnabled(false);

      delete stdinChannel.notifier;
      stdinChannel.notifier = 0;
   }

   if (startupSocketNotifier) {
      startupSocketNotifier->setEnabled(false);

      delete startupSocketNotifier;
      startupSocketNotifier = 0;
   }

   if (deathNotifier) {
      deathNotifier->setEnabled(false);

      delete deathNotifier;
      deathNotifier = 0;
   }

   if (notifier) {
      delete notifier;
      notifier = 0;
   }

   destroyPipe(stdoutChannel.pipe);
   destroyPipe(stderrChannel.pipe);
   destroyPipe(stdinChannel.pipe);
   destroyPipe(childStartedPipe);
   destroyPipe(deathPipe);

#ifdef Q_OS_UNIX
   serial = 0;
#endif

}

/*! \internal
*/
bool QProcessPrivate::_q_canReadStandardOutput()
{
   Q_Q(QProcess);
   qint64 available = bytesAvailableFromStdout();
   if (available == 0) {
      if (stdoutChannel.notifier) {
         stdoutChannel.notifier->setEnabled(false);
      }
      destroyPipe(stdoutChannel.pipe);
#if defined QPROCESS_DEBUG
      qDebug("QProcessPrivate::canReadStandardOutput(), 0 bytes available");
#endif
      return false;
   }

   char *ptr = outputReadBuffer.reserve(available);
   qint64 readBytes = readFromStdout(ptr, available);
   if (readBytes == -1) {
      processError = QProcess::ReadError;
      q->setErrorString(QProcess::tr("Error reading from process"));
      emit q->error(processError);
#if defined QPROCESS_DEBUG
      qDebug("QProcessPrivate::canReadStandardOutput(), failed to read from the process");
#endif
      return false;
   }
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::canReadStandardOutput(), read %d bytes from the process' output",
          int(readBytes));
#endif

   if (stdoutChannel.closed) {
      outputReadBuffer.chop(readBytes);
      return false;
   }

   outputReadBuffer.chop(available - readBytes);

   bool didRead = false;
   if (readBytes == 0) {
      if (stdoutChannel.notifier) {
         stdoutChannel.notifier->setEnabled(false);
      }
   } else if (processChannel == QProcess::StandardOutput) {
      didRead = true;
      if (!emittedReadyRead) {
         emittedReadyRead = true;
         emit q->readyRead();
         emittedReadyRead = false;
      }
   }
   emit q->readyReadStandardOutput();
   return didRead;
}

/*! \internal
*/
bool QProcessPrivate::_q_canReadStandardError()
{
   Q_Q(QProcess);
   qint64 available = bytesAvailableFromStderr();
   if (available == 0) {
      if (stderrChannel.notifier) {
         stderrChannel.notifier->setEnabled(false);
      }
      destroyPipe(stderrChannel.pipe);
      return false;
   }

   char *ptr = errorReadBuffer.reserve(available);
   qint64 readBytes = readFromStderr(ptr, available);
   if (readBytes == -1) {
      processError = QProcess::ReadError;
      q->setErrorString(QProcess::tr("Error reading from process"));
      emit q->error(processError);
      return false;
   }
   if (stderrChannel.closed) {
      errorReadBuffer.chop(readBytes);
      return false;
   }

   errorReadBuffer.chop(available - readBytes);

   bool didRead = false;
   if (readBytes == 0) {
      if (stderrChannel.notifier) {
         stderrChannel.notifier->setEnabled(false);
      }
   } else if (processChannel == QProcess::StandardError) {
      didRead = true;
      if (!emittedReadyRead) {
         emittedReadyRead = true;
         emit q->readyRead();
         emittedReadyRead = false;
      }
   }
   emit q->readyReadStandardError();
   return didRead;
}

/*! \internal
*/
bool QProcessPrivate::_q_canWrite()
{
   Q_Q(QProcess);
   if (stdinChannel.notifier) {
      stdinChannel.notifier->setEnabled(false);
   }

   if (writeBuffer.isEmpty()) {
#if defined QPROCESS_DEBUG
      qDebug("QProcessPrivate::canWrite(), not writing anything (empty write buffer).");
#endif
      return false;
   }

   qint64 written = writeToStdin(writeBuffer.readPointer(),
                                 writeBuffer.nextDataBlockSize());
   if (written < 0) {
      destroyPipe(stdinChannel.pipe);
      processError = QProcess::WriteError;
      q->setErrorString(QProcess::tr("Error writing to process"));
      emit q->error(processError);
      return false;
   }

#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::canWrite(), wrote %d bytes to the process input", int(written));
#endif

   if (written != 0) {
      writeBuffer.free(written);
      if (!emittedBytesWritten) {
         emittedBytesWritten = true;
         emit q->bytesWritten(written);
         emittedBytesWritten = false;
      }
   }
   if (stdinChannel.notifier && !writeBuffer.isEmpty()) {
      stdinChannel.notifier->setEnabled(true);
   }
   if (writeBuffer.isEmpty() && stdinChannel.closed) {
      closeWriteChannel();
   }
   return true;
}

/*! \internal
*/
bool QProcessPrivate::_q_processDied()
{
   Q_Q(QProcess);
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::_q_processDied()");
#endif
#ifdef Q_OS_UNIX
   if (!waitForDeadChild()) {
      return false;
   }
#endif
#ifdef Q_OS_WIN
   if (processFinishedNotifier) {
      processFinishedNotifier->setEnabled(false);
   }
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
      processError = QProcess::Crashed;
      q->setErrorString(QProcess::tr("Process crashed"));
      emit q->error(processError);
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
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::_q_processDied() process is dead");
#endif
   return true;
}

/*! \internal
*/
bool QProcessPrivate::_q_startupNotification()
{
   Q_Q(QProcess);
#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::startupNotification()");
#endif

   if (startupSocketNotifier) {
      startupSocketNotifier->setEnabled(false);
   }
   if (processStarted()) {
      q->setProcessState(QProcess::Running);
      emit q->started();
      return true;
   }

   q->setProcessState(QProcess::NotRunning);
   processError = QProcess::FailedToStart;
   emit q->error(processError);
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

void QProcess::_q_notified()
{
   Q_D(QProcess);
   d->_q_notified();
}


/*! \internal
*/
void QProcessPrivate::closeWriteChannel()
{

#if defined QPROCESS_DEBUG
   qDebug("QProcessPrivate::closeWriteChannel()");
#endif

   if (stdinChannel.notifier) {
      stdinChannel.notifier->setEnabled(false);

      if (stdinChannel.notifier) {
         delete stdinChannel.notifier;
         stdinChannel.notifier = 0;
      }
   }

#ifdef Q_OS_WIN
   // ### Find a better fix, feeding the process little by little instead.
   flushPipeWriter();
#endif

   destroyPipe(stdinChannel.pipe);
}

/*!
    Constructs a QProcess object with the given \a parent.
*/
QProcess::QProcess(QObject *parent)
   : QIODevice(*new QProcessPrivate, parent)
{
#if defined QPROCESS_DEBUG
   qDebug("QProcess::QProcess(%p)", parent);
#endif
}

/*!
    Destructs the QProcess object, i.e., killing the process.

    Note that this function will not return until the process is
    terminated.
*/
QProcess::~QProcess()
{
   Q_D(QProcess);
   if (d->processState != NotRunning) {
      qWarning("QProcess: Destroyed while process is still running.");
      kill();
      waitForFinished();
   }
#ifdef Q_OS_UNIX
   // make sure the process manager removes this entry
   d->findExitCode();
#endif
   d->cleanup();
}

/*!
    \obsolete
    Returns the read channel mode of the QProcess. This function is
    equivalent to processChannelMode()

    \sa processChannelMode()
*/
QProcess::ProcessChannelMode QProcess::readChannelMode() const
{
   return processChannelMode();
}

/*!
    \obsolete

    Use setProcessChannelMode(\a mode) instead.

    \sa setProcessChannelMode()
*/
void QProcess::setReadChannelMode(ProcessChannelMode mode)
{
   setProcessChannelMode(mode);
}

/*!
    \since 4.2

    Returns the channel mode of the QProcess standard output and
    standard error channels.

    \sa setProcessChannelMode(), ProcessChannelMode, setReadChannel()
*/
QProcess::ProcessChannelMode QProcess::processChannelMode() const
{
   Q_D(const QProcess);
   return d->processChannelMode;
}

/*!
    \since 4.2

    Sets the channel mode of the QProcess standard output and standard
    error channels to the \a mode specified.
    This mode will be used the next time start() is called. For example:

    \snippet doc/src/snippets/code/src_corelib_io_qprocess.cpp 0

    \sa processChannelMode(), ProcessChannelMode, setReadChannel()
*/
void QProcess::setProcessChannelMode(ProcessChannelMode mode)
{
   Q_D(QProcess);
   d->processChannelMode = mode;
}

/*!
    Returns the current read channel of the QProcess.

    \sa setReadChannel()
*/
QProcess::ProcessChannel QProcess::readChannel() const
{
   Q_D(const QProcess);
   return d->processChannel;
}

/*!
    Sets the current read channel of the QProcess to the given \a
    channel. The current input channel is used by the functions
    read(), readAll(), readLine(), and getChar(). It also determines
    which channel triggers QProcess to emit readyRead().

    \sa readChannel()
*/
void QProcess::setReadChannel(ProcessChannel channel)
{
   Q_D(QProcess);
   if (d->processChannel != channel) {
      QByteArray buf = d->buffer.readAll();
      if (d->processChannel == QProcess::StandardOutput) {
         for (int i = buf.size() - 1; i >= 0; --i) {
            d->outputReadBuffer.ungetChar(buf.at(i));
         }
      } else {
         for (int i = buf.size() - 1; i >= 0; --i) {
            d->errorReadBuffer.ungetChar(buf.at(i));
         }
      }
   }
   d->processChannel = channel;
}

/*!
    Closes the read channel \a channel. After calling this function,
    QProcess will no longer receive data on the channel. Any data that
    has already been received is still available for reading.

    Call this function to save memory, if you are not interested in
    the output of the process.

    \sa closeWriteChannel(), setReadChannel()
*/
void QProcess::closeReadChannel(ProcessChannel channel)
{
   Q_D(QProcess);

   if (channel == StandardOutput) {
      d->stdoutChannel.closed = true;
   } else {
      d->stderrChannel.closed = true;
   }
}

/*!
    Schedules the write channel of QProcess to be closed. The channel
    will close once all data has been written to the process. After
    calling this function, any attempts to write to the process will
    fail.

    Closing the write channel is necessary for programs that read
    input data until the channel has been closed. For example, the
    program "more" is used to display text data in a console on both
    Unix and Windows. But it will not display the text data until
    QProcess's write channel has been closed. Example:

    \snippet doc/src/snippets/code/src_corelib_io_qprocess.cpp 1

    The write channel is implicitly opened when start() is called.

    \sa closeReadChannel()
*/
void QProcess::closeWriteChannel()
{
   Q_D(QProcess);
   d->stdinChannel.closed = true; // closing
   if (d->writeBuffer.isEmpty()) {
      d->closeWriteChannel();
   }
}

/*!
    \since 4.2

    Redirects the process' standard input to the file indicated by \a
    fileName. When an input redirection is in place, the QProcess
    object will be in read-only mode (calling write() will result in
    error).

    If the file \a fileName does not exist at the moment start() is
    called or is not readable, starting the process will fail.

    Calling setStandardInputFile() after the process has started has no
    effect.

    \sa setStandardOutputFile(), setStandardErrorFile(),
        setStandardOutputProcess()
*/
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

/*!
    If QProcess has been assigned a working directory, this function returns
    the working directory that the QProcess will enter before the program has
    started. Otherwise, (i.e., no directory has been assigned,) an empty
    string is returned, and QProcess will use the application's current
    working directory instead.

    \sa setWorkingDirectory()
*/
QString QProcess::workingDirectory() const
{
   Q_D(const QProcess);
   return d->workingDirectory;
}

/*!
    Sets the working directory to \a dir. QProcess will start the
    process in this directory. The default behavior is to start the
    process in the working directory of the calling process.

    \note The working directory setting is ignored on Symbian;
    the private directory of the process is considered its working
    directory.

    \note On QNX, this may cause all application threads to
    temporarily freeze.

    \sa workingDirectory(), start()
*/
void QProcess::setWorkingDirectory(const QString &dir)
{
   Q_D(QProcess);
   d->workingDirectory = dir;
}

/*!
    Returns the native process identifier for the running process, if
    available.  If no process is currently running, 0 is returned.
*/
Q_PID QProcess::pid() const
{
   Q_D(const QProcess);
   return d->pid;
}

/*! \reimp

    This function operates on the current read channel.

    \sa readChannel(), setReadChannel()
*/
bool QProcess::canReadLine() const
{
   Q_D(const QProcess);
   const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                   ? &d->errorReadBuffer
                                   : &d->outputReadBuffer;
   return readBuffer->canReadLine() || QIODevice::canReadLine();
}

/*!
    Closes all communication with the process and kills it. After calling this
    function, QProcess will no longer emit readyRead(), and data can no
    longer be read or written.
*/
void QProcess::close()
{
   emit aboutToClose();
   while (waitForBytesWritten(-1))
      ;
   kill();
   waitForFinished(-1);
   QIODevice::close();
}

/*! \reimp

   Returns true if the process is not running, and no more data is available
   for reading; otherwise returns false.
*/
bool QProcess::atEnd() const
{
   Q_D(const QProcess);
   const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                   ? &d->errorReadBuffer
                                   : &d->outputReadBuffer;
   return QIODevice::atEnd() && (!isOpen() || readBuffer->isEmpty());
}

/*! \reimp
*/
bool QProcess::isSequential() const
{
   return true;
}

/*! \reimp
*/
qint64 QProcess::bytesAvailable() const
{
   Q_D(const QProcess);
   const QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                                   ? &d->errorReadBuffer
                                   : &d->outputReadBuffer;
#if defined QPROCESS_DEBUG
   qDebug("QProcess::bytesAvailable() == %i (%s)", readBuffer->size(),
          (d->processChannel == QProcess::StandardError) ? "stderr" : "stdout");
#endif
   return readBuffer->size() + QIODevice::bytesAvailable();
}

/*! \reimp
*/
qint64 QProcess::bytesToWrite() const
{
   Q_D(const QProcess);
   qint64 size = d->writeBuffer.size();
#ifdef Q_OS_WIN
   size += d->pipeWriterBytesToWrite();
#endif
   return size;
}

/*!
    Returns the type of error that occurred last.

    \sa state()
*/
QProcess::ProcessError QProcess::errorCode() const
{
   Q_D(const QProcess);
   return d->processError;
}

/*!
    Returns the current state of the process.

    \sa stateChanged(), error()
*/
QProcess::ProcessState QProcess::state() const
{
   Q_D(const QProcess);
   return d->processState;
}

/*!
    \deprecated
    Sets the environment that QProcess will use when starting a process to the
    \a environment specified which consists of a list of key=value pairs.

    For example, the following code adds the \c{C:\\BIN} directory to the list of
    executable paths (\c{PATHS}) on Windows:

    \snippet doc/src/snippets/qprocess-environment/main.cpp 0

    \note This function is less efficient than the setProcessEnvironment()
    function.

    \sa environment(), setProcessEnvironment(), systemEnvironment()
*/
void QProcess::setEnvironment(const QStringList &environment)
{
   setProcessEnvironment(QProcessEnvironmentPrivate::fromList(environment));
}

/*!
    \deprecated
    Returns the environment that QProcess will use when starting a
    process, or an empty QStringList if no environment has been set
    using setEnvironment() or setEnvironmentHash(). If no environment
    has been set, the environment of the calling process will be used.

    \note The environment settings are ignored on Windows CE and Symbian,
    as there is no concept of an environment.

    \sa processEnvironment(), setEnvironment(), systemEnvironment()
*/
QStringList QProcess::environment() const
{
   Q_D(const QProcess);
   return d->environment.toStringList();
}

/*!
    \since 4.6
    Sets the environment that QProcess will use when starting a process to the
    \a environment object.

    For example, the following code adds the \c{C:\\BIN} directory to the list of
    executable paths (\c{PATHS}) on Windows and sets \c{TMPDIR}:

    \snippet doc/src/snippets/qprocess-environment/main.cpp 1

    Note how, on Windows, environment variable names are case-insensitive.

    \sa processEnvironment(), QProcessEnvironment::systemEnvironment(), setEnvironment()
*/
void QProcess::setProcessEnvironment(const QProcessEnvironment &environment)
{
   Q_D(QProcess);
   d->environment = environment;
}

/*!
    \since 4.6
    Returns the environment that QProcess will use when starting a
    process, or an empty object if no environment has been set using
    setEnvironment() or setProcessEnvironment(). If no environment has
    been set, the environment of the calling process will be used.

    \note The environment settings are ignored on Windows CE,
    as there is no concept of an environment.

    \sa setProcessEnvironment(), setEnvironment(), QProcessEnvironment::isEmpty()
*/
QProcessEnvironment QProcess::processEnvironment() const
{
   Q_D(const QProcess);
   return d->environment;
}

/*!
    Blocks until the process has started and the started() signal has
    been emitted, or until \a msecs milliseconds have passed.

    Returns true if the process was started successfully; otherwise
    returns false (if the operation timed out or if an error
    occurred).

    This function can operate without an event loop. It is
    useful when writing non-GUI applications and when performing
    I/O operations in a non-GUI thread.

    \warning Calling this function from the main (GUI) thread
    might cause your user interface to freeze.

    If msecs is -1, this function will not time out.

    \sa started(), waitForReadyRead(), waitForBytesWritten(), waitForFinished()
*/
bool QProcess::waitForStarted(int msecs)
{
   Q_D(QProcess);
   if (d->processState == QProcess::Starting) {
      return d->waitForStarted(msecs);
   }

   return d->processState == QProcess::Running;
}

/*! \reimp
*/
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

/*! \reimp
*/
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
      if (!started) {
         return false;
      }
      if (msecs != -1) {
         msecs -= stopWatch.elapsed();
      }
   }

   return d->waitForBytesWritten(msecs);
}

/*!
    Blocks until the process has finished and the finished() signal
    has been emitted, or until \a msecs milliseconds have passed.

    Returns true if the process finished; otherwise returns false (if
    the operation timed out, if an error occurred, or if this QProcess
    is already finished).

    This function can operate without an event loop. It is
    useful when writing non-GUI applications and when performing
    I/O operations in a non-GUI thread.

    \warning Calling this function from the main (GUI) thread
    might cause your user interface to freeze.

    If msecs is -1, this function will not time out.

    \sa finished(), waitForStarted(), waitForReadyRead(), waitForBytesWritten()
*/
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
      if (msecs != -1) {
         msecs -= stopWatch.elapsed();
      }
   }

   return d->waitForFinished(msecs);
}

/*!
    Sets the current state of the QProcess to the \a state specified.

    \sa state()
*/
void QProcess::setProcessState(ProcessState state)
{
   Q_D(QProcess);
   if (d->processState == state) {
      return;
   }
   d->processState = state;
   emit stateChanged(state);
}

/*!
  This function is called in the child process context just before the
    program is executed on Unix or Mac OS X (i.e., after \e fork(), but before
    \e execve()). Reimplement this function to do last minute initialization
    of the child process. Example:

    \snippet doc/src/snippets/code/src_corelib_io_qprocess.cpp 4

    You cannot exit the process (by calling exit(), for instance) from
    this function. If you need to stop the program before it starts
    execution, your workaround is to emit finished() and then call
    exit().

    \warning This function is called by QProcess on Unix and Mac OS X
    only. On Windows and QNX, it is not called.
*/
void QProcess::setupChildProcess()
{
}

/*! \reimp
*/
qint64 QProcess::readData(char *data, qint64 maxlen)
{
   Q_D(QProcess);
   QRingBuffer *readBuffer = (d->processChannel == QProcess::StandardError)
                             ? &d->errorReadBuffer
                             : &d->outputReadBuffer;

   if (maxlen == 1 && !readBuffer->isEmpty()) {
      int c = readBuffer->getChar();
      if (c == -1) {
#if defined QPROCESS_DEBUG
         qDebug("QProcess::readData(%p \"%s\", %d) == -1",
                data, qt_prettyDebug(data, 1, maxlen).constData(), 1);
#endif
         return -1;
      }
      *data = (char) c;
#if defined QPROCESS_DEBUG
      qDebug("QProcess::readData(%p \"%s\", %d) == 1",
             data, qt_prettyDebug(data, 1, maxlen).constData(), 1);
#endif
      return 1;
   }

   qint64 bytesToRead = qint64(qMin(readBuffer->size(), (int)maxlen));
   qint64 readSoFar = 0;

   while (readSoFar < bytesToRead) {
      const char *ptr = readBuffer->readPointer();
      int bytesToReadFromThisBlock = qMin(bytesToRead - readSoFar, readBuffer->nextDataBlockSize());

      memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
      readSoFar += bytesToReadFromThisBlock;
      readBuffer->free(bytesToReadFromThisBlock);
   }

#if defined QPROCESS_DEBUG
   qDebug("QProcess::readData(%p \"%s\", %lld) == %lld",
          data, qt_prettyDebug(data, readSoFar, 16).constData(), maxlen, readSoFar);
#endif
   if (!readSoFar && d->processState == QProcess::NotRunning) {
      return -1;   // EOF
   }
   return readSoFar;
}

/*! \reimp
*/
qint64 QProcess::writeData(const char *data, qint64 len)
{
   Q_D(QProcess);

   if (d->stdinChannel.closed) {
#if defined QPROCESS_DEBUG
      qDebug("QProcess::writeData(%p \"%s\", %lld) == 0 (write channel closing)",
             data, qt_prettyDebug(data, len, 16).constData(), len);
#endif
      return 0;
   }

   if (len == 1) {
      d->writeBuffer.putChar(*data);
      if (d->stdinChannel.notifier) {
         d->stdinChannel.notifier->setEnabled(true);
      }
#if defined QPROCESS_DEBUG
      qDebug("QProcess::writeData(%p \"%s\", %lld) == 1 (written to buffer)",
             data, qt_prettyDebug(data, len, 16).constData(), len);
#endif
      return 1;
   }

   char *dest = d->writeBuffer.reserve(len);
   memcpy(dest, data, len);
   if (d->stdinChannel.notifier) {
      d->stdinChannel.notifier->setEnabled(true);
   }
#if defined QPROCESS_DEBUG
   qDebug("QProcess::writeData(%p \"%s\", %lld) == %lld (written to buffer)",
          data, qt_prettyDebug(data, len, 16).constData(), len, len);
#endif
   return len;
}

/*!
    Regardless of the current read channel, this function returns all
    data available from the standard output of the process as a
    QByteArray.

    \sa readyReadStandardOutput(), readAllStandardError(), readChannel(), setReadChannel()
*/
QByteArray QProcess::readAllStandardOutput()
{
   ProcessChannel tmp = readChannel();
   setReadChannel(StandardOutput);
   QByteArray data = readAll();
   setReadChannel(tmp);
   return data;
}

/*!
    Regardless of the current read channel, this function returns all
    data available from the standard error of the process as a
    QByteArray.

    \sa readyReadStandardError(), readAllStandardOutput(), readChannel(), setReadChannel()
*/
QByteArray QProcess::readAllStandardError()
{
   ProcessChannel tmp = readChannel();
   setReadChannel(StandardError);
   QByteArray data = readAll();
   setReadChannel(tmp);
   return data;
}

/*!
    Starts the given \a program in a new process, if none is already
    running, passing the command line arguments in \a arguments. The OpenMode
    is set to \a mode.

    The QProcess object will immediately enter the Starting state. If the
    process starts successfully, QProcess will emit started(); otherwise,
    error() will be emitted. If the QProcess object is already running a
    process, a warning may be printed at the console, and the existing
    process will continue running.

    \note Processes are started asynchronously, which means the started()
    and error() signals may be delayed. Call waitForStarted() to make
    sure the process has started (or has failed to start) and those signals
    have been emitted.

    \note No further splitting of the arguments is performed.

    \bold{Windows:} Arguments that contain spaces are wrapped in quotes.

    \sa pid(), started(), waitForStarted()
*/
void QProcess::start(const QString &program, const QStringList &arguments, OpenMode mode)
{
   Q_D(QProcess);
   if (d->processState != NotRunning) {
      qWarning("QProcess::start: Process is already running");
      return;
   }

#if defined QPROCESS_DEBUG
   qDebug() << "QProcess::start(" << program << ',' << arguments << ',' << mode << ')';
#endif

   d->outputReadBuffer.clear();
   d->errorReadBuffer.clear();

   if (d->stdinChannel.type != QProcessPrivate::Channel::Normal) {
      mode &= ~WriteOnly;   // not open for writing
   }
   if (d->stdoutChannel.type != QProcessPrivate::Channel::Normal &&
         (d->stderrChannel.type != QProcessPrivate::Channel::Normal ||
          d->processChannelMode == MergedChannels)) {
      mode &= ~ReadOnly;   // not open for reading
   }
   if (mode == 0) {
      mode = Unbuffered;
   }
   QIODevice::open(mode);

   d->stdinChannel.closed = false;
   d->stdoutChannel.closed = false;
   d->stderrChannel.closed = false;

   d->program = program;
   d->arguments = arguments;

   d->exitCode = 0;
   d->exitStatus = NormalExit;
   d->processError = QProcess::UnknownError;
   d->errorString.clear();
   d->startProcess();
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
      if (program.at(i) == QLatin1Char('"')) {
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

/*!
    \overload

    Starts the program \a program in a new process, if one is not already
    running. \a program is a single string of text containing both the
    program name and its arguments. The arguments are separated by one or
    more spaces. For example:

    \snippet doc/src/snippets/code/src_corelib_io_qprocess.cpp 5

    The \a program string can also contain quotes, to ensure that arguments
    containing spaces are correctly supplied to the new process. For example:

    \snippet doc/src/snippets/code/src_corelib_io_qprocess.cpp 6

    If the QProcess object is already running a process, a warning may be
    printed at the console, and the existing process will continue running.

    Note that, on Windows, quotes need to be both escaped and quoted.
    For example, the above code would be specified in the following
    way to ensure that \c{"My Documents"} is used as the argument to
    the \c dir executable:

    \snippet doc/src/snippets/code/src_corelib_io_qprocess.cpp 7

    The OpenMode is set to \a mode.
*/
void QProcess::start(const QString &program, OpenMode mode)
{
   QStringList args = parseCombinedArgString(program);
   if (args.isEmpty()) {
      Q_D(QProcess);
      d->processError = QProcess::FailedToStart;
      setErrorString(tr("No program defined"));
      emit error(d->processError);
      return;
   }

   QString prog = args.first();
   args.removeFirst();

   start(prog, args, mode);
}

/*!
    Attempts to terminate the process.

    The process may not exit as a result of calling this function (it is given
    the chance to prompt the user for any unsaved files, etc).

    On Windows, terminate() posts a WM_CLOSE message to all toplevel windows
    of the process and then to the main thread of the process itself. On Unix
    and Mac OS X the SIGTERM signal is sent.

    Console applications on Windows that do not run an event loop, or whose
    event loop does not handle the WM_CLOSE message, can only be terminated by
    calling kill().

    On Symbian, this function requires platform security capability
    \c PowerMgmt. If absent, the process will panic with KERN-EXEC 46.

    \note Terminating running processes from other processes will typically
    cause a panic in Symbian due to platform security.

    \sa {Symbian Platform Security Requirements}
    \sa kill()
*/
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

/*!
    Returns the exit code of the last process that finished.
*/
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
   if (!process.waitForFinished(-1)) {
      return -2;
   }
   return process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
}

int QProcess::execute(const QString &program)
{
   QProcess process;
   process.setReadChannelMode(ForwardedChannels);
   process.start(program);
   if (!process.waitForFinished(-1)) {
      return -2;
   }
   return process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
}

bool QProcess::startDetached(const QString &program, const QStringList &arguments,
                  const QString &workingDirectory, qint64 *pid)
{
   return QProcessPrivate::startDetached(program, arguments, workingDirectory, pid);
}

bool QProcess::startDetached(const QString &program, const QStringList &arguments)
{
   return QProcessPrivate::startDetached(program, arguments);
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


#if defined(Q_OS_MAC) && ! defined(Q_OS_IOS)
# include <crt_externs.h>
# define environ (*_NSGetEnviron())

#elif defined(Q_OS_MAC) && defined(Q_OS_IOS)
static char *qt_empty_environ[] = { 0 };
#define environ qt_empty_environ

#elif ! defined(Q_OS_WIN)
extern char **environ;
#endif


QStringList QProcess::systemEnvironment()
{
   QStringList tmp;
   const char *entry = nullptr;

   if (environ != nullptr) {

      for (int count = 0; (entry = environ[count]); ++count) {
         tmp << QString::fromLocal8Bit(entry);
      }
   }

   return tmp;
}

#endif // QT_NO_PROCESS

