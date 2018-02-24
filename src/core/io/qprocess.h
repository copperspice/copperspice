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

#ifndef QPROCESS_H
#define QPROCESS_H

#include <QtCore/qiodevice.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PROCESS

#if ! defined(Q_OS_WIN)
typedef qint64 Q_PID;
#else
QT_END_NAMESPACE
typedef struct _PROCESS_INFORMATION *Q_PID;
QT_BEGIN_NAMESPACE
#endif

class QProcessPrivate;
class QProcessEnvironmentPrivate;

class Q_CORE_EXPORT QProcessEnvironment
{
 public:
   QProcessEnvironment();
   QProcessEnvironment(const QProcessEnvironment &other);
   ~QProcessEnvironment();
   QProcessEnvironment &operator=(QProcessEnvironment && other)  {
      swap(other);
      return *this;
   }

   QProcessEnvironment &operator=(const QProcessEnvironment &other);

   bool operator==(const QProcessEnvironment &other) const;
   inline bool operator!=(const QProcessEnvironment &other) const {
      return !(*this == other);
   }

   bool isEmpty() const;
   void clear();

   bool contains(const QString &name) const;
   void insert(const QString &name, const QString &value);
   void remove(const QString &name);
   QString value(const QString &name, const QString &defaultValue = QString()) const;

   QStringList toStringList() const;

   QStringList keys() const;

   void insert(const QProcessEnvironment &e);

   static QProcessEnvironment systemEnvironment();

   void swap(QProcessEnvironment &other) {
      qSwap(d, other.d);
   }

 private:
   friend class QProcessPrivate;
   friend class QProcessEnvironmentPrivate;
   QSharedDataPointer<QProcessEnvironmentPrivate> d;
};

class Q_CORE_EXPORT QProcess : public QIODevice
{
   CORE_CS_OBJECT(QProcess)

 public:
   enum ProcessError {
      FailedToStart, //### file not found, resource error
      Crashed,
      Timedout,
      ReadError,
      WriteError,
      UnknownError
   };
   enum ProcessState {
      NotRunning,
      Starting,
      Running
   };
   enum ProcessChannel {
      StandardOutput,
      StandardError
   };
   enum ProcessChannelMode {
      SeparateChannels,
      MergedChannels,
      ForwardedChannels,
      ForwardedOutputChannel,
      ForwardedErrorChannel
   };
   enum InputChannelMode {
      ManagedInputChannel,
       ForwardedInputChannel
   };

   enum ExitStatus {
      NormalExit,
      CrashExit
   };

   explicit QProcess(QObject *parent = nullptr);
   virtual ~QProcess();

   void start(const QString &program, const QStringList &arguments, OpenMode mode = ReadWrite);
   void start(const QString &command, OpenMode mode = ReadWrite);
    void start(OpenMode mode = ReadWrite);
    bool open(OpenMode mode = ReadWrite) override;
    QString program() const;
    void setProgram(const QString &program);
    QStringList arguments() const;
    void setArguments(const QStringList & arguments);
   ProcessChannelMode readChannelMode() const;
   void setReadChannelMode(ProcessChannelMode mode);
   ProcessChannelMode processChannelMode() const;
   void setProcessChannelMode(ProcessChannelMode mode);
   InputChannelMode inputChannelMode() const;
   void setInputChannelMode(InputChannelMode mode);

   ProcessChannel readChannel() const;
   void setReadChannel(ProcessChannel channel);

   void closeReadChannel(ProcessChannel channel);
   void closeWriteChannel();

   void setStandardInputFile(const QString &fileName);
   void setStandardOutputFile(const QString &fileName, OpenMode mode = Truncate);
   void setStandardErrorFile(const QString &fileName,  OpenMode mode = Truncate);
   void setStandardOutputProcess(QProcess *destination);

#if defined(Q_OS_WIN)
   QString nativeArguments() const;
   void setNativeArguments(const QString &arguments);
#endif

   QString workingDirectory() const;
   void setWorkingDirectory(const QString &dir);

   void setEnvironment(const QStringList &environment);
   QStringList environment() const;
   void setProcessEnvironment(const QProcessEnvironment &environment);
   QProcessEnvironment processEnvironment() const;

   QProcess::ProcessError error() const;

   QProcess::ProcessState state() const;

   // #### Qt5/Q_PID is a pointer on Windows and a value on Unix
   Q_PID pid() const;
   qint64 processId() const;

   bool waitForStarted(int msecs = 30000);
   bool waitForReadyRead(int msecs = 30000) override;
   bool waitForBytesWritten(int msecs = 30000) override;
   bool waitForFinished(int msecs = 30000);

   QByteArray readAllStandardOutput();
   QByteArray readAllStandardError();

   int exitCode() const;
   QProcess::ExitStatus exitStatus() const;

   // QIODevice
   qint64 bytesAvailable() const override;
   qint64 bytesToWrite() const override;
   bool isSequential() const override;
   bool canReadLine() const override;
   void close() override;
   bool atEnd() const override;

   static int execute(const QString &program, const QStringList &arguments);
   static int execute(const QString &program);

   static bool startDetached(const QString &program, const QStringList &arguments,
                  const QString &workingDirectory = QString(), qint64 *pid = nullptr);

   static bool startDetached(const QString &program);

   static QStringList systemEnvironment();
   static QString nullDevice();

   CORE_CS_SLOT_1(Public, void terminate())
   CORE_CS_SLOT_2(terminate)

   CORE_CS_SLOT_1(Public, void kill())
   CORE_CS_SLOT_2(kill)

   CORE_CS_SIGNAL_1(Public, void started())
   CORE_CS_SIGNAL_2(started)

   CORE_CS_SIGNAL_1(Public, void finished(int exitCode))
   CORE_CS_SIGNAL_OVERLOAD(finished, (int), exitCode)

   CORE_CS_SIGNAL_1(Public, void finished(int exitCode, QProcess::ExitStatus exitStatus))
   CORE_CS_SIGNAL_OVERLOAD(finished, (int, QProcess::ExitStatus), exitCode, exitStatus)

   CORE_CS_SIGNAL_1(Public, void errorOccurred(QProcess::ProcessError error))
   CORE_CS_SIGNAL_2(errorOccurred, error)

   CORE_CS_SIGNAL_1(Public, void stateChanged(QProcess::ProcessState state))
   CORE_CS_SIGNAL_2(stateChanged, state)

   CORE_CS_SIGNAL_1(Public, void readyReadStandardOutput())
   CORE_CS_SIGNAL_2(readyReadStandardOutput)

   CORE_CS_SIGNAL_1(Public, void readyReadStandardError())
   CORE_CS_SIGNAL_2(readyReadStandardError)

 protected:
   void setProcessState(ProcessState state);
   virtual void setupChildProcess();

   // QIODevice
   qint64 readData(char *data, qint64 maxlen) override;
   qint64 writeData(const char *data, qint64 len) override;

 private:
   Q_DECLARE_PRIVATE(QProcess)
   Q_DISABLE_COPY(QProcess)

   CORE_CS_SLOT_1(Private, bool _q_canReadStandardOutput())
   CORE_CS_SLOT_2(_q_canReadStandardOutput)

   CORE_CS_SLOT_1(Private, bool _q_canReadStandardError())
   CORE_CS_SLOT_2(_q_canReadStandardError)

   CORE_CS_SLOT_1(Private, bool _q_canWrite())
   CORE_CS_SLOT_2(_q_canWrite)

   CORE_CS_SLOT_1(Private, bool _q_startupNotification())
   CORE_CS_SLOT_2(_q_startupNotification)

   CORE_CS_SLOT_1(Private, bool _q_processDied())
   CORE_CS_SLOT_2(_q_processDied)


   friend class QProcessManager;
};

#endif // QT_NO_PROCESS

#endif // QPROCESS_H
