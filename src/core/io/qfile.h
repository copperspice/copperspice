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

#ifndef QFILE_H
#define QFILE_H

#include <qfiledevice.h>
#include <qstring.h>
#include <stdio.h>

#ifdef open
#error qfile.h must be included before any header file that defines open
#endif

class QTemporaryFile;
class QFilePrivate;

class Q_CORE_EXPORT QFile : public QFileDevice
{
   CORE_CS_OBJECT(QFile)
   Q_DECLARE_PRIVATE(QFile)

 public:
   QFile();
   QFile(const QString &name);

   explicit QFile(QObject *parent);
   QFile(const QString &name, QObject *parent);

   QFile(const QFile &) = delete;
   QFile &operator=(const QFile &) = delete;

   ~QFile();

   QString fileName() const override;
   void setFileName(const QString &name);

   using EncoderFn = QByteArray (*)(const QString &fileName);
   using DecoderFn = QString (*)(const QByteArray &localfileName);

   static QByteArray encodeName(const QString &fileName);
   static QString decodeName(const QByteArray &localFileName);

   static QString decodeName(const char *localFileName) {
      return decodeName(QByteArray(localFileName));
   }

   static void setEncodingFunction(EncoderFn function);
   static void setDecodingFunction(DecoderFn function);

   bool exists() const;
   static bool exists(const QString &fileName);

   QString readLink() const;
   static QString readLink(const QString &fileName);

   QString symLinkTarget() const {
      return readLink();
   }

   static QString symLinkTarget(const QString &fileName) {
      return readLink(fileName);
   }

   bool remove();
   static bool remove(const QString &fileName);

   bool rename(const QString &newName);
   static bool rename(const QString &oldName, const QString &newName);

   bool link(const QString &newName);
   static bool link(const QString &oldName, const QString &newName);

   bool copy(const QString &newName);
   static bool copy(const QString &oldName, const QString &newName);

   bool open(OpenMode mode) override;
   bool open(FILE *fHandle, OpenMode mode, FileHandleFlags handleFlags = DontCloseHandle);
   bool open(int fd, OpenMode mode, FileHandleFlags handleFlags = DontCloseHandle);

   qint64 size() const override;

   bool resize(qint64 sz) override;
   static bool resize(const QString &filename, qint64 sz);

   Permissions permissions() const override;
   static Permissions permissions(const QString &fileName);
   bool setPermissions(Permissions permissions) override;
   static bool setPermissions(const QString &fileName, Permissions permissions);

 protected:
   QFile(QFilePrivate &dd, QObject *parent = nullptr);

 private:
   friend class QTemporaryFile;
};

#endif
