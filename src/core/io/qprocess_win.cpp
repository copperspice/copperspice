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

#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qelapsedtimer.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qregularexpression.h>
#include <qthread.h>
#include <qtimer.h>
#include <qwaitcondition.h>
#include <qwineventnotifier.h>

#include <qfsfileengine_p.h>
#include <qprocess_p.h>
#include <qsystemlibrary_p.h>
#include <qthread_p.h>
#include <qwindowspipereader_p.h>
#include <qwindowspipewriter_p.h>

#ifndef PIPE_REJECT_REMOTE_CLIENTS
#define PIPE_REJECT_REMOTE_CLIENTS 0x08
#endif

#ifndef QT_NO_PROCESS

#define NOTIFYTIMEOUT 100

static void qt_create_pipe(Q_PIPE *pipe, bool isInputPipe)
{
   // Open the pipes.  Make non-inheritable copies of input write and output
   // read handles to avoid non-closable handles (this is done by the
   // DuplicateHandle() call).

   SECURITY_ATTRIBUTES secAtt = { sizeof(SECURITY_ATTRIBUTES), nullptr, false };

   HANDLE hServer;
   wchar_t pipeName[256];
   unsigned int attempts = 1000;

   while (true) {
      _snwprintf(pipeName, sizeof(pipeName) / sizeof(pipeName[0]), L"\\\\.\\pipe\\qt-%X", qrand());

      DWORD dwOpenMode = FILE_FLAG_OVERLAPPED;
      DWORD dwOutputBufferSize = 0;
      DWORD dwInputBufferSize = 0;
      const DWORD dwPipeBufferSize = 1024 * 1024;

      if (isInputPipe) {
         dwOpenMode |= PIPE_ACCESS_OUTBOUND;
         dwOutputBufferSize = dwPipeBufferSize;

      } else {
         dwOpenMode |= PIPE_ACCESS_INBOUND;
         dwInputBufferSize = dwPipeBufferSize;
      }

      DWORD dwPipeFlags = PIPE_TYPE_BYTE | PIPE_WAIT;

      if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA) {
         dwPipeFlags |= PIPE_REJECT_REMOTE_CLIENTS;
      }

      // only one pipe instance
      hServer = CreateNamedPipe(pipeName, dwOpenMode, dwPipeFlags, 1, dwOutputBufferSize, dwInputBufferSize, 0, &secAtt);

      if (hServer != INVALID_HANDLE_VALUE) {
         break;
      }

      DWORD dwError = GetLastError();

      if (dwError != ERROR_PIPE_BUSY || ! --attempts) {
         qErrnoWarning(dwError, "QProcess: CreateNamedPipe failed.");
         return;
      }
   }

   secAtt.bInheritHandle = TRUE;
   const HANDLE hClient = CreateFile(pipeName,
         (isInputPipe ? (GENERIC_READ | FILE_WRITE_ATTRIBUTES) : GENERIC_WRITE), 0,
         &secAtt, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

   if (hClient == INVALID_HANDLE_VALUE) {
      qErrnoWarning("QProcess: CreateFile failed");
      CloseHandle(hServer);
      return;
   }

   ConnectNamedPipe(hServer, nullptr);

   if (isInputPipe) {
      pipe[0] = hClient;
      pipe[1] = hServer;
   } else {
      pipe[0] = hServer;
      pipe[1] = hClient;
   }
}

static void duplicateStdWriteChannel(Q_PIPE *pipe, DWORD nStdHandle)
{
   pipe[0] = INVALID_Q_PIPE;
   HANDLE hStdWriteChannel = GetStdHandle(nStdHandle);
   HANDLE hCurrentProcess  = GetCurrentProcess();
   DuplicateHandle(hCurrentProcess, hStdWriteChannel, hCurrentProcess, &pipe[1], 0, TRUE, DUPLICATE_SAME_ACCESS);
}

bool QProcessPrivate::openChannel(Channel &channel)
{
   Q_Q(QProcess);

   if (&channel == &stderrChannel && processChannelMode == QProcess::MergedChannels) {
      return DuplicateHandle(GetCurrentProcess(), stdoutChannel.pipe[1], GetCurrentProcess(),
            &stderrChannel.pipe[1], 0, TRUE, DUPLICATE_SAME_ACCESS);
   }

   if (channel.type == Channel::Normal) {
      // piping this channel to our own process
      if (&channel == &stdinChannel) {
         if (inputChannelMode != QProcess::ForwardedInputChannel) {
            qt_create_pipe(channel.pipe, true);

         } else {
            channel.pipe[1] = INVALID_Q_PIPE;
            HANDLE hStdReadChannel = GetStdHandle(STD_INPUT_HANDLE);
            HANDLE hCurrentProcess = GetCurrentProcess();

            DuplicateHandle(hCurrentProcess, hStdReadChannel, hCurrentProcess,
                  &channel.pipe[0], 0, TRUE, DUPLICATE_SAME_ACCESS);
         }

      } else {
         if (&channel == &stdoutChannel) {
            if (processChannelMode != QProcess::ForwardedChannels && processChannelMode != QProcess::ForwardedOutputChannel) {
               if (! stdoutChannel.reader) {
                  stdoutChannel.reader = new QWindowsPipeReader(q);
                  q->connect(stdoutChannel.reader, &QWindowsPipeReader::readyRead, q, &QProcess::_q_canReadStandardOutput);
               }

            } else {
               duplicateStdWriteChannel(channel.pipe, STD_OUTPUT_HANDLE);
            }

         } else {
            if (processChannelMode != QProcess::ForwardedChannels && processChannelMode !=
                  QProcess::ForwardedErrorChannel) {

               if (! stderrChannel.reader) {
                  stderrChannel.reader = new QWindowsPipeReader(q);

                  q->connect(stderrChannel.reader, &QWindowsPipeReader::readyRead,
                        q, &QProcess::_q_canReadStandardError);
               }

            } else {
               duplicateStdWriteChannel(channel.pipe, STD_ERROR_HANDLE);
            }
         }

         if (channel.reader) {
            qt_create_pipe(channel.pipe, false);
            channel.reader->setHandle(channel.pipe[0]);
            channel.reader->startAsyncRead();
         }
      }

      return true;

   } else if (channel.type == Channel::Redirect) {
      // we're redirecting the channel to/from a file
      SECURITY_ATTRIBUTES secAtt = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

      if (&channel == &stdinChannel) {
         // try to open in read-only mode
         channel.pipe[1] = INVALID_Q_PIPE;
         channel.pipe[0] = CreateFile(&QFSFileEnginePrivate::longFileName(channel.file).toStdWString()[0],
               GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &secAtt,
               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

         if (channel.pipe[0] != INVALID_Q_PIPE) {
            return true;
         }

         setErrorAndEmit(QProcess::FailedToStart, QProcess::tr("Could not open input redirection for reading"));

      } else {
         // open in write mode
         channel.pipe[0] = INVALID_Q_PIPE;
         channel.pipe[1] = CreateFile(&QFSFileEnginePrivate::longFileName(channel.file).toStdWString()[0],
               GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &secAtt,
               channel.append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

         if (channel.pipe[1] != INVALID_Q_PIPE) {
            if (channel.append) {
               SetFilePointer(channel.pipe[1], 0, nullptr, FILE_END);
            }

            return true;
         }

         setErrorAndEmit(QProcess::FailedToStart, QProcess::tr("Could not open output redirection for writing"));
      }

      // could not open file
      cleanup();

      return false;

   } else {
      Q_ASSERT_X(channel.process, "QProcess::start", "Internal error");

      Channel *source;
      Channel *sink;

      if (channel.type == Channel::PipeSource) {
         // we are the source
         source = &channel;
         sink   = &channel.process->stdinChannel;

         if (source->pipe[1] != INVALID_Q_PIPE) {
            // already constructed by the sink make it inheritable
            HANDLE tmpHandle = source->pipe[1];

            if (! DuplicateHandle(GetCurrentProcess(), tmpHandle, GetCurrentProcess(),
                  &source->pipe[1], 0, TRUE, DUPLICATE_SAME_ACCESS)) {
               return false;
            }

            CloseHandle(tmpHandle);
            return true;
         }

         Q_ASSERT(source == &stdoutChannel);
         Q_ASSERT(sink->process == this && sink->type == Channel::PipeSink);

         qt_create_pipe(source->pipe, false); // source is stdout
         sink->pipe[0]   = source->pipe[0];
         source->pipe[0] = INVALID_Q_PIPE;

         return true;

      } else {
         // we are the sink;
         source = &channel.process->stdoutChannel;
         sink   = &channel;

         if (sink->pipe[0] != INVALID_Q_PIPE) {
            // already constructed by the source, make it inheritable
            HANDLE tmpHandle = sink->pipe[0];

            if (! DuplicateHandle(GetCurrentProcess(), tmpHandle, GetCurrentProcess(),
                  &sink->pipe[0], 0, TRUE, DUPLICATE_SAME_ACCESS)) {
               return false;
            }

            CloseHandle(tmpHandle);
            return true;
         }

         Q_ASSERT(sink == &stdinChannel);
         Q_ASSERT(source->process == this && source->type == Channel::PipeSource);

         qt_create_pipe(sink->pipe, /* in = */ true); // sink is stdin
         source->pipe[1] = sink->pipe[1];
         sink->pipe[1]   = INVALID_Q_PIPE;

         return true;
      }
   }
}

void QProcessPrivate::destroyPipe(Q_PIPE pipe[2])
{
   if (pipe[0] != INVALID_Q_PIPE) {
      CloseHandle(pipe[0]);
      pipe[0] = INVALID_Q_PIPE;
   }

   if (pipe[1] != INVALID_Q_PIPE) {
      CloseHandle(pipe[1]);
      pipe[1] = INVALID_Q_PIPE;
   }
}

void QProcessPrivate::closeChannel(Channel *channel)
{
   if (channel == &stdinChannel) {
      delete stdinChannel.writer;
      stdinChannel.writer = nullptr;

   } else if (channel->reader) {
      channel->reader->stop();
      channel->reader->deleteLater();
      channel->reader = nullptr;
   }

   destroyPipe(channel->pipe);
}

static QString qt_create_commandline(const QString &program, const QStringList &arguments)
{
   QString args;

   if (! program.isEmpty()) {
      QString programName = program;

      if (! programName.startsWith('\"') && ! programName.endsWith('\"') && programName.contains(' ')) {
         programName = '\"' + programName + '\"';
      }

      programName.replace('/', '\\');

      // add the prgram as the first arg ... it works better
      args = programName + ' ';
   }

   for (int i = 0; i < arguments.size(); ++i) {
      QString tmp = arguments.at(i);

      // Quotes are escaped and their preceding backslashes are doubled.
      tmp.replace(QRegularExpression8("(\\\\*)\""), "\\1\\1\\\"");

      if (tmp.isEmpty() || tmp.contains(QLatin1Char(' ')) || tmp.contains('\t')) {

         // The argument must not end with a \ since this would be interpreted
         // as escaping the quote -- rather put the \ behind the quote: e.g.
         // rather use "foo"\ than "foo\"

         int i = tmp.length();

         while (i > 0 && tmp.at(i - 1) == '\\') {
            --i;
         }

         tmp.insert(i, QLatin1Char('"'));
         tmp.prepend(QLatin1Char('"'));
      }

      args += QLatin1Char(' ') + tmp;
   }

   return args;
}

QProcessEnvironment QProcessEnvironment::systemEnvironment()
{
   QProcessEnvironment env;

   // Calls to setenv() affect the low-level environment as well.
   // This is not the case the other way round.
   if (wchar_t *envStrings = GetEnvironmentStringsW()) {

      for (const wchar_t *entry = envStrings; *entry; ) {
         const int entryLen = int(wcslen(entry));

         if (const wchar_t *equal = wcschr(entry + 1, L'=')) {
            int nameLen   = equal - entry;

            QString name  = QString::fromStdWString(std::wstring(entry, nameLen));
            QString value = QString::fromStdWString(std::wstring(equal + 1, entryLen - nameLen - 1));
            env.d->hash.insert(QProcEnvKey(name), value);
         }

         entry += entryLen + 1;
      }

      FreeEnvironmentStringsW(envStrings);
   }

   return env;
}

static QByteArray qt_create_environment(const QHash<QProcEnvKey, QProcEnvValue> &environment)
{
   QByteArray envlist;

   if (! environment.isEmpty()) {
      QHash<QProcEnvKey, QProcEnvValue> copy = environment;

      // add PATH if necessary (for DLL loading)
      QProcEnvKey pathKey("PATH");

      if (! copy.contains(pathKey)) {
         QByteArray path = qgetenv("PATH");

         if (! path.isEmpty()) {
            copy.insert(pathKey, QString::fromUtf8(path));
         }
      }

      // add systemroot if needed
      QProcEnvKey rootKey("SystemRoot");

      if (! copy.contains(rootKey)) {
         QByteArray systemRoot = qgetenv("SystemRoot");

         if (! systemRoot.isEmpty()) {
            copy.insert(rootKey, QString::fromUtf8(systemRoot));
         }
      }

      int pos  = 0;
      auto it  = copy.constBegin();
      auto end = copy.constEnd();

      static const wchar_t equal = L'=';
      static const wchar_t nul   = L'\0';

      for ( ; it != end; ++it) {
         std::wstring key   = it.key().toStdWString();
         std::wstring value = it.value().toStdWString();

         uint tmpSize = sizeof(wchar_t) * (key.size() + value.size() + 2);

         // ignore empty strings
         if (tmpSize == sizeof(wchar_t) * 2) {
            continue;
         }

         envlist.resize(envlist.size() + tmpSize);

         tmpSize = key.size() * sizeof(wchar_t);
         memcpy(envlist.data() + pos, &key[0], tmpSize);
         pos += tmpSize;

         memcpy(envlist.data() + pos, &equal, sizeof(wchar_t));
         pos += sizeof(wchar_t);

         tmpSize = value.size() * sizeof(wchar_t);
         memcpy(envlist.data() + pos, &value[0], tmpSize);
         pos += tmpSize;

         memcpy(envlist.data() + pos, &nul, sizeof(wchar_t));
         pos += sizeof(wchar_t);
      }

      // add the 2 terminating 0 (actually 4, just to be on the safe side)
      envlist.resize( envlist.size() + 4 );
      envlist[pos++] = 0;
      envlist[pos++] = 0;
      envlist[pos++] = 0;
      envlist[pos++] = 0;
   }

   return envlist;
}

void QProcessPrivate::startProcess()
{
   Q_Q(QProcess);

   bool success = false;

   if (pid) {
      CloseHandle(pid->hThread);
      CloseHandle(pid->hProcess);
      delete pid;
      pid = nullptr;
   }

   pid = new PROCESS_INFORMATION;
   memset(pid, 0, sizeof(PROCESS_INFORMATION));

   q->setProcessState(QProcess::Starting);

   if (! openChannel(stdinChannel) || ! openChannel(stdoutChannel) || ! openChannel(stderrChannel)) {
      return;
   }

   QString args = qt_create_commandline(program, arguments);
   QByteArray envlist;

   if (environment.d.constData()) {
      envlist = qt_create_environment(environment.d.constData()->hash);
   }

   if (! nativeArguments.isEmpty()) {
      if (! args.isEmpty()) {
         args += ' ';
      }

      args += nativeArguments;
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("Creating process");
   qDebug("   program : [%s]", program.toLatin1().constData());
   qDebug("   args : %s", args.toLatin1().constData());
   qDebug("   pass environment : %s", environment.isEmpty() ? "no" : "yes");
#endif

   DWORD dwCreationFlags = (GetConsoleWindow() ? 0 : CREATE_NO_WINDOW);
   dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

   STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), nullptr, nullptr, nullptr,
         (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
         0, 0, 0, STARTF_USESTDHANDLES, 0, 0, nullptr, stdinChannel.pipe[0], stdoutChannel.pipe[1], stderrChannel.pipe[1] };

   success = CreateProcess(nullptr, &args.toStdWString()[0], nullptr, nullptr, TRUE, dwCreationFlags,
         environment.isEmpty() ? nullptr : envlist.data(),
         workingDirectory.isEmpty() ? nullptr : &QDir::toNativeSeparators(workingDirectory).toStdWString()[0],
         &startupInfo, pid);

   QString errorString;

   if (! success) {
      // Capture the error string before we do CloseHandle below
      errorString = QProcess::tr("Process failed to start: %1").formatArg(qt_error_string());
   }

   if (stdinChannel.pipe[0] != INVALID_Q_PIPE) {
      CloseHandle(stdinChannel.pipe[0]);
      stdinChannel.pipe[0] = INVALID_Q_PIPE;
   }

   if (stdoutChannel.pipe[1] != INVALID_Q_PIPE) {
      CloseHandle(stdoutChannel.pipe[1]);
      stdoutChannel.pipe[1] = INVALID_Q_PIPE;
   }

   if (stderrChannel.pipe[1] != INVALID_Q_PIPE) {
      CloseHandle(stderrChannel.pipe[1]);
      stderrChannel.pipe[1] = INVALID_Q_PIPE;
   }

   if (!success) {
      cleanup();
      setErrorAndEmit(QProcess::FailedToStart, errorString);

      q->setProcessState(QProcess::NotRunning);
      return;
   }

   q->setProcessState(QProcess::Running);

   // User can call kill()/terminate() from the stateChanged() slot so check before proceeding
   if (! pid) {
      return;
   }

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

   if (threadData->eventDispatcher) {
      processFinishedNotifier = new QWinEventNotifier(pid->hProcess, q);

      QObject::connect(processFinishedNotifier, &QWinEventNotifier::activated, q, &QProcess::_q_processDied);
      processFinishedNotifier->setEnabled(true);
   }

   _q_startupNotification();
}

bool QProcessPrivate::processStarted(QString *)
{
   return processState == QProcess::Running;
}

qint64 QProcessPrivate::bytesAvailableInChannel(const Channel *channel) const
{
   Q_ASSERT(channel->pipe[0] != INVALID_Q_PIPE);
   Q_ASSERT(channel->reader);

   DWORD bytesAvail = channel->reader->bytesAvailable();

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcessPrivate::bytesAvailableInChannel(%lld) == %ld", channel - &stdinChannel, bytesAvail);
#endif

   return bytesAvail;
}

qint64 QProcessPrivate::readFromChannel(const Channel *channel, char *data, qint64 maxlen)
{
   Q_ASSERT(channel->pipe[0] != INVALID_Q_PIPE);
   Q_ASSERT(channel->reader);
   return channel->reader->read(data, maxlen);
}

static BOOL QT_WIN_CALLBACK qt_terminateApp(HWND hwnd, LPARAM procId)
{
   DWORD currentProcId = 0;
   GetWindowThreadProcessId(hwnd, &currentProcId);

   if (currentProcId == (DWORD)procId) {
      PostMessage(hwnd, WM_CLOSE, 0, 0);
   }

   return TRUE;
}

void QProcessPrivate::terminateProcess()
{
   if (pid) {
      EnumWindows(qt_terminateApp, (LPARAM)pid->dwProcessId);
      PostThreadMessage(pid->dwThreadId, WM_CLOSE, 0, 0);
   }
}

void QProcessPrivate::killProcess()
{
   if (pid) {
      TerminateProcess(pid->hProcess, 0xf291);
   }
}

bool QProcessPrivate::waitForStarted(int)
{
   if (processStarted()) {
      return true;
   }

   if (processError == QProcess::FailedToStart) {
      return false;
   }

   setError(QProcess::Timedout);
   return false;
}

bool QProcessPrivate::drainOutputPipes()
{
   if (! stdoutChannel.reader && ! stderrChannel.reader) {
      return false;
   }

   bool someReadyReadEmitted = false;

   while (true) {
      bool readyReadEmitted = false;
      bool readOperationActive = false;

      if (stdoutChannel.reader) {
         readyReadEmitted |= stdoutChannel.reader->waitForReadyRead(0);
         readOperationActive = stdoutChannel.reader && stdoutChannel.reader->isReadOperationActive();
      }

      if (stderrChannel.reader) {
         readyReadEmitted |= stderrChannel.reader->waitForReadyRead(0);
         readOperationActive |= stderrChannel.reader && stderrChannel.reader->isReadOperationActive();
      }

      someReadyReadEmitted |= readyReadEmitted;

      if (! readOperationActive || !readyReadEmitted) {
         break;
      }

      QThread::yieldCurrentThread();
   }

   return someReadyReadEmitted;
}

bool QProcessPrivate::waitForReadyRead(int msecs)
{
   QIncrementalSleepTimer timer(msecs);

   while (true) {
      if (! stdinChannel.buffer.isEmpty() && !_q_canWrite()) {
         return false;
      }

      if (stdinChannel.writer && stdinChannel.writer->waitForWrite(0)) {
         timer.resetIncrements();
      }

      if ((stdoutChannel.reader && stdoutChannel.reader->waitForReadyRead(0))
            || (stderrChannel.reader && stderrChannel.reader->waitForReadyRead(0))) {
         return true;
      }

      if (! pid) {
         return false;
      }

      if (WaitForSingleObjectEx(pid->hProcess, 0, false) == WAIT_OBJECT_0) {
         bool readyReadEmitted = drainOutputPipes();

         if (pid) {
            _q_processDied();
         }

         return readyReadEmitted;
      }

      Sleep(timer.nextSleepTime());

      if (timer.hasTimedOut()) {
         break;
      }
   }

   setError(QProcess::Timedout);
   return false;
}

bool QProcessPrivate::waitForBytesWritten(int msecs)
{
   QIncrementalSleepTimer timer(msecs);

   while (true) {
      // Check if we have any data pending: the pipe writer has
      // bytes waiting to written, or it has written data since the
      // last time we called pipeWriter->waitForWrite().
      bool pendingDataInPipe = stdinChannel.writer && stdinChannel.writer->bytesToWrite();

      // If we don't have pending data, and our write buffer is
      // empty, we fail.
      if (!pendingDataInPipe && stdinChannel.buffer.isEmpty()) {
         return false;
      }

      // If we don't have pending data and we do have data in our
      // write buffer, try to flush that data over to the pipe
      // writer.  Fail on error.
      if (!pendingDataInPipe)  {
         if (!_q_canWrite()) {
            return false;
         }
      }

      // Wait for the pipe writer to acknowledge that it has
      // written. This will succeed if either the pipe writer has
      // already written the data, or if it manages to write data
      // within the given timeout. If the write buffer was non-empty
      // and the pipeWriter is now dead, that means _q_canWrite()
      // destroyed the writer after it successfully wrote the last batch.

      if (! stdinChannel.writer || stdinChannel.writer->waitForWrite(0)) {
         return true;
      }

      // check if we can read stdout
      if (stdoutChannel.pipe[0] != INVALID_Q_PIPE && bytesAvailableInChannel(&stdoutChannel) != 0) {
         tryReadFromChannel(&stdoutChannel);
         timer.resetIncrements();
      }

      // Check if we can read stderr.
      if (stderrChannel.pipe[0] != INVALID_Q_PIPE && bytesAvailableInChannel(&stderrChannel) != 0) {
         tryReadFromChannel(&stderrChannel);
         timer.resetIncrements();
      }

      // Check if the process died while reading.
      if (! pid) {
         return false;
      }

      // Wait for the process to signal any change in its state,
      // such as incoming data, or if the process died.
      if (WaitForSingleObjectEx(pid->hProcess, 0, false) == WAIT_OBJECT_0) {
         _q_processDied();
         return false;
      }

      // Only wait for as long as we've been asked.
      if (timer.hasTimedOut()) {
         break;
      }
   }

   setError(QProcess::Timedout);
   return false;
}

bool QProcessPrivate::waitForFinished(int msecs)
{
#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QProcessPrivate::waitForFinished(%d)", msecs);
#endif

   QIncrementalSleepTimer timer(msecs);

   while (true) {
      if (! stdinChannel.buffer.isEmpty() && !_q_canWrite()) {
         return false;
      }

      if (stdinChannel.writer && stdinChannel.writer->waitForWrite(0)) {
         timer.resetIncrements();
      }

      if (stdoutChannel.reader && stdoutChannel.reader->waitForReadyRead(0)) {
         timer.resetIncrements();
      }

      if (stderrChannel.reader && stderrChannel.reader->waitForReadyRead(0)) {
         timer.resetIncrements();
      }

      if (!pid) {
         drainOutputPipes();
         return true;
      }

      if (WaitForSingleObject(pid->hProcess, timer.nextSleepTime()) == WAIT_OBJECT_0) {
         drainOutputPipes();

         if (pid) {
            _q_processDied();
         }

         return true;
      }

      if (timer.hasTimedOut()) {
         break;
      }
   }

   setError(QProcess::Timedout);

   return false;
}

void QProcessPrivate::findExitCode()
{
   DWORD theExitCode;
   Q_ASSERT(pid);

   if (GetExitCodeProcess(pid->hProcess, &theExitCode)) {
      exitCode = theExitCode;
      // for now we assume a crash if exit code is less than -1 or the magic number
      // our magic number, see killProcess

      crashed = (exitCode == 0xf291 || (theExitCode >= 0x80000000 && theExitCode < 0xD0000000));
   }
}

void QProcessPrivate::flushPipeWriter()
{
   if (stdinChannel.writer && stdinChannel.writer->bytesToWrite() > 0) {
      stdinChannel.writer->waitForWrite(ULONG_MAX);
   }
}

qint64 QProcessPrivate::pipeWriterBytesToWrite() const
{
   return stdinChannel.writer ? stdinChannel.writer->bytesToWrite() : qint64(0);
}

bool QProcessPrivate::writeToStdin()
{
   Q_Q(QProcess);

   if (! stdinChannel.writer) {
      stdinChannel.writer = new QWindowsPipeWriter(stdinChannel.pipe[1], q);

      QObject::connect(stdinChannel.writer, &QWindowsPipeWriter::bytesWritten, q, &QProcess::bytesWritten);

      QObject::connect(stdinChannel.writer, &QWindowsPipeWriter::canWrite, q,
            [this]() { this->_q_canWrite(); } );

   } else {
      if (stdinChannel.writer->isWriteOperationActive()) {
         return true;
      }
   }

   stdinChannel.writer->write(stdinChannel.buffer.read());
   return true;
}

bool QProcessPrivate::waitForWrite(int msecs)
{
   if (! stdinChannel.writer || stdinChannel.writer->waitForWrite(msecs)) {
      return true;
   }

   setError(QProcess::Timedout);
   return false;
}

// Use ShellExecuteEx() to trigger an UAC prompt when CreateProcess()fails
// with ERROR_ELEVATION_REQUIRED.
static bool startDetachedUacPrompt(const QString &programIn, const QStringList &arguments, const QString &workingDir, qint64 *pid)
{
   using ShellExecuteExType = BOOL (WINAPI *)(SHELLEXECUTEINFOW *);

   static const ShellExecuteExType shellExecuteEx = // XP ServicePack 1 onwards.
         reinterpret_cast<ShellExecuteExType>(QSystemLibrary::resolve("shell32", "ShellExecuteExW"));

   if (! shellExecuteEx) {
      return false;
   }

   const QString args = qt_create_commandline(QString(), arguments); // needs arguments only
   SHELLEXECUTEINFOW shellExecuteExInfo;
   memset(&shellExecuteExInfo, 0, sizeof(SHELLEXECUTEINFOW));

   shellExecuteExInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
   shellExecuteExInfo.fMask  = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_UNICODE | SEE_MASK_FLAG_NO_UI;
   shellExecuteExInfo.lpVerb = L"runas";

   const QString program = QDir::toNativeSeparators(programIn);

   std::wstring tmp_program = program.toStdWString();
   shellExecuteExInfo.lpFile = reinterpret_cast<LPCWSTR>(&tmp_program[0]);

   std::wstring tmp_args;

   if (! args.isEmpty()) {
      tmp_args = args.toStdWString();
      shellExecuteExInfo.lpParameters = reinterpret_cast<LPCWSTR>(&tmp_args[0]);
   }

   std::wstring tmp_dir;

   if (! workingDir.isEmpty()) {
      tmp_dir = workingDir.toStdWString();
      shellExecuteExInfo.lpDirectory = reinterpret_cast<LPCWSTR>(&tmp_dir[0]);
   }

   shellExecuteExInfo.nShow = SW_SHOWNORMAL;

   if (! shellExecuteEx(&shellExecuteExInfo)) {
      return false;
   }

   if (pid) {
      *pid = qint64(GetProcessId(shellExecuteExInfo.hProcess));
   }

   CloseHandle(shellExecuteExInfo.hProcess);
   return true;
}

bool QProcessPrivate::startDetached(const QString &program, const QStringList &arguments, const QString &workingDir, qint64 *pid)
{
   static const DWORD errorElevationRequired = 740;

   QString args = qt_create_commandline(program, arguments);
   bool success = false;

   PROCESS_INFORMATION pinfo;

   STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), nullptr, nullptr, nullptr,
         (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
         0, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr};

   success = CreateProcess(nullptr, &args.toStdWString()[0], nullptr, nullptr,
         FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_CONSOLE, nullptr,
         workingDir.isEmpty() ? nullptr : &workingDir.toStdWString()[0], &startupInfo, &pinfo);

   if (success) {
      CloseHandle(pinfo.hThread);
      CloseHandle(pinfo.hProcess);

      if (pid) {
         *pid = pinfo.dwProcessId;
      }

   } else if (GetLastError() == errorElevationRequired) {
      success = startDetachedUacPrompt(program, arguments, workingDir, pid);
   }

   return success;
}

#endif // QT_NO_PROCESS
