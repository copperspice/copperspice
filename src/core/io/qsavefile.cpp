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

/*****************************************************
** Copyright (c) 2012 David Faure <faure@kde.org>
*****************************************************/

#include <qsavefile.h>

#include <qplatformdefs.h>

#include <qfileinfo.h>
#include <qdebug.h>
#include <qtemporaryfile.h>

#include <qsavefile_p.h>
#include <qabstractfileengine_p.h>
#include <qiodevice_p.h>
#include <qtemporaryfile_p.h>

#ifdef Q_OS_UNIX
#include <errno.h>
#endif

QSaveFilePrivate::QSaveFilePrivate()
   : writeError(QFileDevice::NoError), useTemporaryFile(true), directWriteFallback(false)
{
}

QSaveFilePrivate::~QSaveFilePrivate()
{
}

QSaveFile::QSaveFile(QObject *parent)
   : QFileDevice(*new QSaveFilePrivate, parent)
{
}

QSaveFile::QSaveFile(const QString &name)
   : QFileDevice(*new QSaveFilePrivate, nullptr)
{
   Q_D(QSaveFile);
   d->fileName = name;
}

QSaveFile::QSaveFile(const QString &name, QObject *parent)
   : QFileDevice(*new QSaveFilePrivate, parent)
{
   Q_D(QSaveFile);
   d->fileName = name;
}

QSaveFile::~QSaveFile()
{
   Q_D(QSaveFile);
   QFileDevice::close();

   if (d->fileEngine) {
      d->fileEngine->remove();
      delete d->fileEngine;
      d->fileEngine = nullptr;
   }
}

QString QSaveFile::fileName() const
{
   return d_func()->fileName;
}

void QSaveFile::setFileName(const QString &name)
{
   d_func()->fileName = name;
}

bool QSaveFile::open(OpenMode mode)
{
   Q_D(QSaveFile);

   if (isOpen()) {
      qWarning("QSaveFile::open() File already open, %s", csPrintable(fileName()));
      return false;
   }

   unsetError();

   if ((mode & (QIODevice::ReadOnly | QIODevice::WriteOnly)) == 0) {
      qWarning("QSaveFile::open() Open mode not specified");
      return false;
   }

   // In the future we could implement ReadWrite by copying from the existing file to the temp file
   if ((mode & QIODevice::ReadOnly) || (mode & QIODevice::Append)) {
      qWarning("QSaveFile::open() Unsupported open mode 0x%x", int(mode));
      return false;
   }

   // check if existing file is writable
   QFileInfo existingFile(d->fileName);

   if (existingFile.exists() && ! existingFile.isWritable()) {
      d->setError(QFileDevice::WriteError, QSaveFile::tr("Existing file %1 is not writable").formatArg(d->fileName));
      d->writeError = QFileDevice::WriteError;
      return false;
   }

   d->fileEngine = new QTemporaryFileEngine(d->fileName, 0666);

   // Same as in QFile: QIODevice provides the buffering, so there's no need to request it from the file engine.
   if (! d->fileEngine->open(mode | QIODevice::Unbuffered)) {
      QFileDevice::FileError err = d->fileEngine->error();

#ifdef Q_OS_UNIX

      if (d->directWriteFallback && err == QFileDevice::OpenError && errno == EACCES) {
         delete d->fileEngine;
         d->fileEngine = QAbstractFileEngine::create(d->fileName);

         if (d->fileEngine->open(mode | QIODevice::Unbuffered)) {
            d->useTemporaryFile = false;
            QFileDevice::open(mode);
            return true;
         }

         err = d->fileEngine->error();
      }

#endif

      if (err == QFileDevice::UnspecifiedError) {
         err = QFileDevice::OpenError;
      }

      d->setError(err, d->fileEngine->errorString());
      delete d->fileEngine;
      d->fileEngine = nullptr;
      return false;
   }

   d->useTemporaryFile = true;
   QFileDevice::open(mode);

   if (existingFile.exists()) {
      setPermissions(existingFile.permissions());
   }

   return true;
}

void QSaveFile::close()
{
   qFatal("QSaveFile::close called");
}

bool QSaveFile::commit()
{
   Q_D(QSaveFile);

   if (! d->fileEngine) {
      return false;
   }

   if (!isOpen()) {
      qWarning("QSaveFile::commit() File is not open, %s", csPrintable(fileName()));
      return false;
   }

   QFileDevice::close(); // calls flush()

   // Sync to disk if possible. Ignore errors (e.g. not supported).
   d->fileEngine->syncToDisk();

   if (d->useTemporaryFile) {
      if (d->writeError != QFileDevice::NoError) {
         d->fileEngine->remove();
         d->writeError = QFileDevice::NoError;
         delete d->fileEngine;
         d->fileEngine = nullptr;
         return false;
      }

      // atomically replace old file with new file
      // Can not use QFile::rename for that, must use the file engine directly
      Q_ASSERT(d->fileEngine);

      if (! d->fileEngine->renameOverwrite(d->fileName)) {
         d->setError(d->fileEngine->error(), d->fileEngine->errorString());
         d->fileEngine->remove();
         delete d->fileEngine;
         d->fileEngine = nullptr;

         return false;
      }
   }

   delete d->fileEngine;
   d->fileEngine = nullptr;

   return true;
}

void QSaveFile::cancelWriting()
{
   Q_D(QSaveFile);

   if (! isOpen()) {
      return;
   }

   d->setError(QFileDevice::WriteError, QSaveFile::tr("Writing canceled by application"));
   d->writeError = QFileDevice::WriteError;
}

qint64 QSaveFile::writeData(const char *data, qint64 len)
{
   Q_D(QSaveFile);

   if (d->writeError != QFileDevice::NoError) {
      return -1;
   }

   const qint64 ret = QFileDevice::writeData(data, len);

   if (d->error != QFileDevice::NoError) {
      d->writeError = d->error;
   }

   return ret;
}

void QSaveFile::setDirectWriteFallback(bool enabled)
{
   Q_D(QSaveFile);
   d->directWriteFallback = enabled;
}

bool QSaveFile::directWriteFallback() const
{
   Q_D(const QSaveFile);
   return d->directWriteFallback;
}
