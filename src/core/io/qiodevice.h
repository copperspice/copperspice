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

#ifndef QIODEVICE_H
#define QIODEVICE_H

#include <qobject.h>
#include <qstring.h>
#include <QScopedPointer>

#ifdef open
#error qiodevice.h must be included before any header file that defines open
#endif

class QByteArray;
class QIODevicePrivate;

class Q_CORE_EXPORT QIODevice : public QObject
{
   CORE_CS_OBJECT(QIODevice)

 public:
   enum OpenModeFlag {
      NotOpen    = 0x0000,
      ReadOnly   = 0x0001,
      WriteOnly  = 0x0002,
      ReadWrite  = ReadOnly | WriteOnly,
      Append     = 0x0004,
      Truncate   = 0x0008,
      Text       = 0x0010,
      Unbuffered = 0x0020
   };
   using OpenMode = QFlags<OpenModeFlag>;

   CORE_CS_ENUM(OpenModeFlag)
   CORE_CS_FLAG(OpenModeFlag, OpenMode)

   QIODevice();

   explicit QIODevice(QObject *parent);
   virtual ~QIODevice();

   OpenMode openMode() const;

   void setTextModeEnabled(bool enabled);
   bool isTextModeEnabled() const;

   bool isOpen() const;
   bool isReadable() const;
   bool isWritable() const;
   virtual bool isSequential() const;

   virtual bool open(OpenMode mode);
   virtual void close();

   // ### Qt5/pos() and seek() should not be virtual, and
   // ### seek() should call a virtual seekData() function.
   virtual qint64 pos() const;
   virtual qint64 size() const;
   virtual bool seek(qint64 pos);
   virtual bool atEnd() const;
   virtual bool reset();

   virtual qint64 bytesAvailable() const;
   virtual qint64 bytesToWrite() const;

   qint64 read(char *data, qint64 maxlen);
   QByteArray read(qint64 maxlen);
   QByteArray readAll();
   qint64 readLine(char *data, qint64 maxlen);
   QByteArray readLine(qint64 maxlen = 0);
   virtual bool canReadLine() const;

   qint64 write(const char *data, qint64 len);
   qint64 write(const char *data);
   inline qint64 write(const QByteArray &data) {
      return write(data.constData(), data.size());
   }

   qint64 peek(char *data, qint64 maxlen);
   QByteArray peek(qint64 maxlen);

   virtual bool waitForReadyRead(int msecs);
   virtual bool waitForBytesWritten(int msecs);

   void ungetChar(char c);
   bool putChar(char c);
   bool getChar(char *c);

   QString errorString() const;

   CORE_CS_SIGNAL_1(Public, void readyRead())
   CORE_CS_SIGNAL_2(readyRead)
   CORE_CS_SIGNAL_1(Public, void bytesWritten(qint64 bytes))
   CORE_CS_SIGNAL_2(bytesWritten, bytes)
   CORE_CS_SIGNAL_1(Public, void aboutToClose())
   CORE_CS_SIGNAL_2(aboutToClose)
   CORE_CS_SIGNAL_1(Public, void readChannelFinished())
   CORE_CS_SIGNAL_2(readChannelFinished)

 protected:
   QIODevice(QIODevicePrivate &dd, QObject *parent = nullptr);

   virtual qint64 readData(char *data, qint64 maxlen) = 0;
   virtual qint64 readLineData(char *data, qint64 maxlen);
   virtual qint64 writeData(const char *data, qint64 len) = 0;

   void setOpenMode(OpenMode openMode);

   void setErrorString(const QString &errorString);

   QScopedPointer<QIODevicePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QIODevice)
   Q_DISABLE_COPY(QIODevice)

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QIODevice::OpenMode)

class QDebug;
Q_CORE_EXPORT QDebug operator<<(QDebug debug, QIODevice::OpenMode modes);

#endif // QIODEVICE_H
