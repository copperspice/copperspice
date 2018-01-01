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

/*****************************************************
** Copyright (C) 2012 David Faure <faure@kde.org>
*****************************************************/

#include <qplatformdefs.h>
#include <qsavefile.h>
#include <qsavefile_p.h>
#include <qfileinfo.h>
#include <qabstractfileengine_p.h>
#include <qdebug.h>
#include <qtemporaryfile.h>
#include <qiodevice_p.h>
#include <qtemporaryfile_p.h>

#ifdef Q_OS_UNIX
#include <errno.h>
#endif

QT_BEGIN_NAMESPACE

QSaveFilePrivate::QSaveFilePrivate()
   : writeError(QFileDevice::NoError),
     useTemporaryFile(true),
     directWriteFallback(false)
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
   : QFileDevice(*new QSaveFilePrivate, 0)
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
      d->fileEngine = 0;
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
      qWarning("QSaveFile::open: File (%s) already open", qPrintable(fileName()));
      return false;
   }
   unsetError();
   if ((mode & (ReadOnly | WriteOnly)) == 0) {
      qWarning("QSaveFile::open: Open mode not specified");
      return false;
   }
   // In the future we could implement ReadWrite by copying from the existing file to the temp file...
   if ((mode & ReadOnly) || (mode & Append)) {
      qWarning("QSaveFile::open: Unsupported open mode 0x%x", int(mode));
      return false;
   }

   // check if existing file is writable
   QFileInfo existingFile(d->fileName);
   if (existingFile.exists() && !existingFile.isWritable()) {
      d->setError(QFileDevice::WriteError, QSaveFile::tr("Existing file %1 is not writable").arg(d->fileName));
      d->writeError = QFileDevice::WriteError;
      return false;
   }
   d->fileEngine = new QTemporaryFileEngine(d->fileName);

   // Same as in QFile: QIODevice provides the buffering, so there's no need to request it from the file engine.
   if (!d->fileEngine->open(mode | QIODevice::Unbuffered)) {
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
      d->fileEngine = 0;
      return false;
   }

   d->useTemporaryFile = true;
   QFileDevice::open(mode);
   if (existingFile.exists()) {
      setPermissions(existingFile.permissions());
   }
   return true;
}

/*!
  \reimp
  This method has been made private so that it cannot be called, in order to prevent mistakes.
  In order to finish writing the file, call commit().
  If instead you want to abort writing, call cancelWriting().
*/
void QSaveFile::close()
{
   qFatal("QSaveFile::close called");
}

bool QSaveFile::commit()
{
   Q_D(QSaveFile);
   if (!d->fileEngine) {
      return false;
   }

   if (!isOpen()) {
      qWarning("QSaveFile::commit: File (%s) is not open", qPrintable(fileName()));
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
         d->fileEngine = 0;
         return false;
      }
      // atomically replace old file with new file
      // Can't use QFile::rename for that, must use the file engine directly
      Q_ASSERT(d->fileEngine);
      if (!d->fileEngine->renameOverwrite(d->fileName)) {
         d->setError(d->fileEngine->error(), d->fileEngine->errorString());
         d->fileEngine->remove();
         delete d->fileEngine;
         d->fileEngine = 0;
         return false;
      }
   }
   delete d->fileEngine;
   d->fileEngine = 0;
   return true;
}

/*!
  Cancels writing the new file.

  If the application changes its mind while saving, it can call cancelWriting(),
  which sets an error code so that commit() will discard the temporary file.

  Alternatively, it can simply make sure not to call commit().

  Further write operations are possible after calling this method, but none
  of it will have any effect, the written file will be discarded.

  This method has no effect when direct write fallback is used. This is the case
  when saving over an existing file in a readonly directory: no temporary file can
  be created, so the existing file is overwritten no matter what, and cancelWriting()
  cannot do anything about that, the contents of the existing file will be lost.

  \sa commit()
*/
void QSaveFile::cancelWriting()
{
   Q_D(QSaveFile);
   if (!isOpen()) {
      return;
   }
   d->setError(QFileDevice::WriteError, QSaveFile::tr("Writing canceled by application"));
   d->writeError = QFileDevice::WriteError;
}

/*!
  \reimp
*/
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

/*!
  Allows writing over the existing file if necessary.

  QSaveFile creates a temporary file in the same directory as the final
  file and atomically renames it. However this is not possible if the
  directory permissions do not allow creating new files.
  In order to preserve atomicity guarantees, open() fails when it
  cannot create the temporary file.

  In order to allow users to edit files with write permissions in a
  directory with restricted permissions, call setDirectWriteFallback() with
  \a enabled set to true, and the following calls to open() will fallback to
  opening the existing file directly and writing into it, without the use of
  a temporary file.
  This does not have atomicity guarantees, i.e. an application crash or
  for instance a power failure could lead to a partially-written file on disk.
  It also means cancelWriting() has no effect, in such a case.

  Typically, to save documents edited by the user, call setDirectWriteFallback(true),
  and to save application internal files (configuration files, data files, ...), keep
  the default setting which ensures atomicity.

  \sa directWriteFallback()
*/
void QSaveFile::setDirectWriteFallback(bool enabled)
{
   Q_D(QSaveFile);
   d->directWriteFallback = enabled;
}

/*!
  Returns true if the fallback solution for saving files in read-only
  directories is enabled.

  \sa setDirectWriteFallback()
*/
bool QSaveFile::directWriteFallback() const
{
   Q_D(const QSaveFile);
   return d->directWriteFallback;
}

QT_END_NAMESPACE
