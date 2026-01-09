/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QZIPWRITER_P_H
#define QZIPWRITER_P_H

#ifndef QT_NO_TEXTODFWRITER

#include <qfile.h>
#include <qstring.h>

class QZipWriterPrivate;

class Q_GUI_EXPORT QZipWriter
{
 public:
   enum WriterStatus {
      NoError,
      FileWriteError,
      FileOpenError,
      FilePermissionsError,
      FileError
   };

   enum CompressionPolicy {
      AlwaysCompress,
      NeverCompress,
      AutoCompress
   };

   explicit QZipWriter(const QString &fileName, QIODevice::OpenMode mode = (QIODevice::WriteOnly | QIODevice::Truncate) );
   explicit QZipWriter(QIODevice *device);

   QZipWriter(const QZipWriter &) = delete;
   QZipWriter &operator=(const QZipWriter &) = delete;

   ~QZipWriter();

   QIODevice *device() const;

   bool isWritable() const;
   bool exists() const;

   WriterStatus status() const;

   void setCompressionPolicy(CompressionPolicy policy);
   CompressionPolicy compressionPolicy() const;

   void setCreationPermissions(QFileDevice::Permissions permissions);
   QFileDevice::Permissions creationPermissions() const;

   void addFile(const QString &fileName, const QByteArray &data);
   void addFile(const QString &fileName, QIODevice *device);

   void addDirectory(const QString &dirName);
   void addSymLink(const QString &fileName, const QString &destination);

   void close();

 private:
   QZipWriterPrivate *d;
};

#endif // QT_NO_TEXTODFWRITER

#endif // QZIPWRITER_H
