/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QPROCESS_P_H
#define QPROCESS_P_H

#include <qprocess.h>
#include <qstringlist.h>
#include <qhash.h>
#include <qshareddata.h>
#include <qringbuffer_p.h>
#include <qiodevice_p.h>

#ifdef Q_OS_UNIX
#include <qorderedmutexlocker_p.h>
#endif

#ifdef Q_OS_WIN
#include <qt_windows.h>
typedef HANDLE Q_PIPE;
#define INVALID_Q_PIPE INVALID_HANDLE_VALUE
#else
typedef int Q_PIPE;
#define INVALID_Q_PIPE -1
#endif

#ifndef QT_NO_PROCESS

QT_BEGIN_NAMESPACE

class QSocketNotifier;
class QWindowsPipeReader;
class QWindowsPipeWriter;
class QWinEventNotifier;
class QTimer;

#ifdef Q_OS_WIN
class QProcEnvKey : public QString
{
 public:
   QProcEnvKey() {}
   explicit QProcEnvKey(const QString &other) : QString(other) {}
   QProcEnvKey(const QProcEnvKey &other) : QString(other) {}
   bool operator==(const QProcEnvKey &other) const {
      return !compare(other, Qt::CaseInsensitive);
   }
};

inline uint qHash(const QProcEnvKey &key)
{
   return qHash(key.toCaseFolded());
}

typedef QString QProcEnvValue;
#else
class QProcEnvKey
{
 public:
   QProcEnvKey() : hash(0) {}
   explicit QProcEnvKey(const QByteArray &other) : key(other), hash(qHash(key)) {}

   QProcEnvKey(const QProcEnvKey &other) {
      *this = other;
   }

   bool operator==(const QProcEnvKey &other) const {
      return key == other.key;
   }

   QByteArray key;
   uint hash;
};
inline uint qHash(const QProcEnvKey &key)
{
   return key.hash;
}

class QProcEnvValue
{
 public:
   QProcEnvValue() {}
   QProcEnvValue(const QProcEnvValue &other) {
      *this = other;
   }

   explicit QProcEnvValue(const QString &value) : stringValue(value) {}
   explicit QProcEnvValue(const QByteArray &value) : byteValue(value) {}

   bool operator==(const QProcEnvValue &other) const {
      return byteValue.isEmpty() && other.byteValue.isEmpty()
             ? stringValue == other.stringValue
             : bytes() == other.bytes();
   }

   QByteArray bytes() const {
      if (byteValue.isEmpty() && ! stringValue.isEmpty()) {
         byteValue = stringValue.toUtf8();
      }
      return byteValue;
   }
   QString string() const {
      if (stringValue.isEmpty() && ! byteValue.isEmpty()) {
         stringValue = QString::fromUtf8(byteValue);
      }
      return stringValue;
   }

   mutable QByteArray byteValue;
   mutable QString stringValue;
};

Q_DECLARE_TYPEINFO(QProcEnvValue, Q_MOVABLE_TYPE);
#endif

Q_DECLARE_TYPEINFO(QProcEnvKey, Q_MOVABLE_TYPE);

class QProcessEnvironmentPrivate: public QSharedData
{
 public:
   typedef QProcEnvKey Key;
   typedef QProcEnvValue Value;

#ifdef Q_OS_WIN

   inline Key prepareName(const QString &name) const {
      return Key(name);
   }

   inline QString nameToString(const Key &name) const {
      return name;
   }

   inline Value prepareValue(const QString &value) const {
      return value;
   }

   inline QString valueToString(const Value &value) const {
      return value;
   }

   struct MutexLocker {
      MutexLocker(const QProcessEnvironmentPrivate *) {}
   };

   struct OrderedMutexLocker {
      OrderedMutexLocker(const QProcessEnvironmentPrivate *,const QProcessEnvironmentPrivate *)
      {}
   };

#else

    inline Key prepareName(const QString &name) const  {
        Key &ent = nameMap[name];
        if (ent.key.isEmpty())
            ent = Key(name.toUtf8());
        return ent;
    }

    inline QString nameToString(const Key &name) const  {
      const QString sname = QString::fromUtf8(name.key);
      nameMap[sname] = name;
      return sname;
   }

   inline Value prepareValue(const QString &value) const {
      return Value(value);
   }

   inline QString valueToString(const Value &value) const {
      return value.string();
   }

   struct MutexLocker : public QMutexLocker {
      MutexLocker(const QProcessEnvironmentPrivate *d) : QMutexLocker(&d->mutex) {}
   };

   struct OrderedMutexLocker : public QOrderedMutexLocker {
      OrderedMutexLocker(const QProcessEnvironmentPrivate *d1, const QProcessEnvironmentPrivate *d2)
      : QOrderedMutexLocker(&d1->mutex, &d2->mutex)  { }
   };

   QProcessEnvironmentPrivate() : QSharedData() {}
   QProcessEnvironmentPrivate(const QProcessEnvironmentPrivate &other)
      : QSharedData()
   {
      MutexLocker locker(&other);
      hash    = other.hash;
      nameMap = other.nameMap;
   }
#endif

   typedef QHash<Key, Value> Hash;
   Hash hash;

#ifdef Q_OS_UNIX
   typedef QHash<QString, Key> NameHash;
   mutable NameHash nameMap;

   mutable QMutex mutex;
#endif

   static QProcessEnvironment fromList(const QStringList &list);
   QStringList toList() const;
   QStringList keys() const;
   void insert(const QProcessEnvironmentPrivate &other);
};
/**   \cond INTERNAL (notation so DoxyPress will not parse this class  */

template<>
inline void QSharedDataPointer<QProcessEnvironmentPrivate>::detach()
{
   if (d && d->ref.load() == 1) {
      return;
   }

   QProcessEnvironmentPrivate *x = (d ? new QProcessEnvironmentPrivate(*d) : new QProcessEnvironmentPrivate);
   x->ref.ref();

   if (d && !d->ref.deref()) {
      delete d;
   }
   d = x;
}

/**   \endcond   */

class QProcessPrivate : public QIODevicePrivate
{
 public:
   Q_DECLARE_PUBLIC(QProcess)

   struct Channel {
      enum ProcessChannelType {
         Normal = 0,
         PipeSource = 1,
         PipeSink = 2,
         Redirect = 3
         // if you add "= 4" here, increase the number of bits below
      };

      Channel() : process(0), notifier(0), type(Normal), closed(false), append(false) {
         pipe[0] = INVALID_Q_PIPE;
         pipe[1] = INVALID_Q_PIPE;
#ifdef Q_OS_WIN
            reader = 0;
#endif
      }

      void clear();

      Channel &operator=(const QString &fileName) {
         clear();
         file = fileName;
         type = fileName.isEmpty() ? Normal : Redirect;
         return *this;
      }

      void pipeTo(QProcessPrivate *other) {
         clear();
         process = other;
         type = PipeSource;
      }

      void pipeFrom(QProcessPrivate *other) {
         clear();
         process = other;
         type = PipeSink;
      }

      QString file;
      QProcessPrivate *process;
      QSocketNotifier *notifier;

#ifdef Q_OS_WIN
        union {
            QWindowsPipeReader *reader;
            QWindowsPipeWriter *writer;
        };
#endif
        QRingBuffer buffer;
      Q_PIPE pipe[2];

      unsigned type : 2;
      bool closed : 1;
      bool append : 1;
   };

   QProcessPrivate();
   virtual ~QProcessPrivate();

   bool _q_canReadStandardOutput();
   bool _q_canReadStandardError();
   bool _q_canWrite();
   bool _q_startupNotification();
   bool _q_processDied();


   QProcess::ProcessChannel processChannel;
   QProcess::ProcessChannelMode processChannelMode;
   QProcess::InputChannelMode inputChannelMode;
   QProcess::ProcessError processError;
   QProcess::ProcessState processState;
   QString workingDirectory;
   Q_PID pid;
   int sequenceNumber;

   bool dying;
   bool emittedReadyRead;
   bool emittedBytesWritten;

   Channel stdinChannel;
   Channel stdoutChannel;
   Channel stderrChannel;
   bool openChannel(Channel &channel);
   void closeChannel(Channel *channel);
   void closeWriteChannel();
   bool tryReadFromChannel(Channel *channel); // obviously, only stdout and stderr

   QString program;
   QStringList arguments;

#if defined(Q_OS_WIN)
   QString nativeArguments;
#endif

   QProcessEnvironment environment;

   Q_PIPE childStartedPipe[2];

   void destroyPipe(Q_PIPE pipe[2]);

   QSocketNotifier *startupSocketNotifier;
   QSocketNotifier *deathNotifier;

    int forkfd;

#ifdef Q_OS_WIN
   QTimer *stdinWriteTrigger;
   QWinEventNotifier *processFinishedNotifier;
#endif

   void start(QIODevice::OpenMode mode);
   void startProcess();

#if defined(Q_OS_UNIX)
   void execChild(const char *workingDirectory, char **path, char **argv, char **envp);
#endif

    bool processStarted(QString *errorMessage = nullptr);
   void terminateProcess();
   void killProcess();
   void findExitCode();

#ifdef Q_OS_UNIX
   bool waitForDeadChild();
#endif

#ifdef Q_OS_WIN
   bool drainOutputPipes();
   void flushPipeWriter();
   qint64 pipeWriterBytesToWrite() const;
#endif

   static bool startDetached(const QString &program, const QStringList &arguments,
                             const QString &workingDirectory = QString(), qint64 *pid = 0);

   int exitCode;
   QProcess::ExitStatus exitStatus;
   bool crashed;


   bool waitForStarted(int msecs = 30000);
   bool waitForReadyRead(int msecs = 30000);
   bool waitForBytesWritten(int msecs = 30000);
   bool waitForFinished(int msecs = 30000);
   bool waitForWrite(int msecs = 30000);

   qint64 bytesAvailableInChannel(const Channel *channel) const;
   qint64 readFromChannel(const Channel *channel, char *data, qint64 maxlen);
   bool writeToStdin();

   void cleanup();
   void setError(QProcess::ProcessError error, const QString &description = QString());
   void setErrorAndEmit(QProcess::ProcessError error, const QString &description = QString());



};

QT_END_NAMESPACE

#endif // QT_NO_PROCESS

#endif // QPROCESS_P_H
