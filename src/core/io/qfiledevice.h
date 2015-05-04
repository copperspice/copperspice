/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QFILEDEVICE_H
#define QFILEDEVICE_H

#include <QtCore/qiodevice.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QFileDevicePrivate;

class Q_CORE_EXPORT QFileDevice : public QIODevice
{
   CS_OBJECT(QFileDevice)
   Q_DECLARE_PRIVATE(QFileDevice)

 public:
   enum FileError {
      NoError = 0,
      ReadError = 1,
      WriteError = 2,
      FatalError = 3,
      ResourceError = 4,
      OpenError = 5,
      AbortError = 6,
      TimeOutError = 7,
      UnspecifiedError = 8,
      RemoveError = 9,
      RenameError = 10,
      PositionError = 11,
      ResizeError = 12,
      PermissionsError = 13,
      CopyError = 14
   };

   enum Permission {
      ReadOwner = 0x4000, WriteOwner = 0x2000, ExeOwner = 0x1000,
      ReadUser  = 0x0400, WriteUser  = 0x0200, ExeUser  = 0x0100,
      ReadGroup = 0x0040, WriteGroup = 0x0020, ExeGroup = 0x0010,
      ReadOther = 0x0004, WriteOther = 0x0002, ExeOther = 0x0001
   };
   using Permissions = QFlags<Permission>;

   enum FileHandleFlag {
      AutoCloseHandle = 0x0001,
      DontCloseHandle = 0
   };
   using FileHandleFlags = QFlags<FileHandleFlag>;

   ~QFileDevice();

   FileError error() const;
   void unsetError();

   virtual void close();

   bool isSequential() const;

   int handle() const;
   virtual QString fileName() const;

   qint64 pos() const;
   bool seek(qint64 offset);
   bool atEnd() const;
   bool flush();

   qint64 size() const;

   virtual bool resize(qint64 sz);
   virtual Permissions permissions() const;
   virtual bool setPermissions(Permissions permissionSpec);

   enum MemoryMapFlags {
      NoOptions = 0
   };

   uchar *map(qint64 offset, qint64 size, MemoryMapFlags flags = NoOptions);
   bool unmap(uchar *address);

 protected:
   QFileDevice();

   explicit QFileDevice(QObject *parent);
   QFileDevice(QFileDevicePrivate &dd, QObject *parent = 0);

   qint64 readData(char *data, qint64 maxlen);
   qint64 writeData(const char *data, qint64 len);
   qint64 readLineData(char *data, qint64 maxlen);

 private:
   Q_DISABLE_COPY(QFileDevice)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileDevice::Permissions)

QT_END_NAMESPACE

#endif // QFILEDEVICE_H
