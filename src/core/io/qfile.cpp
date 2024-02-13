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

#include <qfile.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qfsfileengine.h>
#include <qlist.h>
#include <qplatformdefs.h>
#include <qtemporaryfile.h>

#include <qfile_p.h>
#include <qfilesystemengine_p.h>
#include <qiodevice_p.h>
#include <qsystemerror_p.h>

static QByteArray locale_encode(const QString &f)
{

#if defined(Q_OS_DARWIN)
   // Mac always expects UTF-8 and decomposed
   return f.normalized(QString::NormalizationForm_D).toUtf8();
#else
   return f.toUtf8();
#endif
}

static QString locale_decode(const QByteArray &f)
{
#if defined(Q_OS_DARWIN)
   // Mac always gives us UTF-8 and decomposed, we want composed
   return QString::fromUtf8(f).normalized(QString::NormalizationForm_C);
#else
   return QString::fromUtf8(f);
#endif
}

QFile::EncoderFn QFilePrivate::encoder = locale_encode;
QFile::DecoderFn QFilePrivate::decoder = locale_decode;

QFilePrivate::QFilePrivate()
{
}

QFilePrivate::~QFilePrivate()
{
}

bool QFilePrivate::openExternalFile(int flags, int fd, QFile::FileHandleFlags handleFlags)
{
#ifdef QT_NO_FSFILEENGINE
   (void) flags;
   (void) fd;

   return false;

#else
   delete fileEngine;
   fileEngine = nullptr;
   QFSFileEngine *fe = new QFSFileEngine;
   fileEngine = fe;

   return fe->open(QIODevice::OpenMode(flags), fd, handleFlags);
#endif
}

bool QFilePrivate::openExternalFile(int flags, FILE *fh, QFile::FileHandleFlags handleFlags)
{
#ifdef QT_NO_FSFILEENGINE
   (void) flags;
   (void) fh;

   return false;

#else
   delete fileEngine;
   fileEngine = nullptr;

   QFSFileEngine *fe = new QFSFileEngine;
   fileEngine = fe;

   return fe->open(QIODevice::OpenMode(flags), fh, handleFlags);
#endif
}

QAbstractFileEngine *QFilePrivate::engine() const
{
   if (! fileEngine) {
      fileEngine = QAbstractFileEngine::create(fileName);
   }

   return fileEngine;
}

QFile::QFile()
   : QFileDevice(*new QFilePrivate, nullptr)
{
}

QFile::QFile(QObject *parent)
   : QFileDevice(*new QFilePrivate, parent)
{
}

QFile::QFile(const QString &name)
   : QFileDevice(*new QFilePrivate, nullptr)
{
   Q_D(QFile);
   d->fileName = name;
}

QFile::QFile(const QString &name, QObject *parent)
   : QFileDevice(*new QFilePrivate, parent)
{
   Q_D(QFile);
   d->fileName = name;
}

QFile::QFile(QFilePrivate &dd, QObject *parent)
   : QFileDevice(dd, parent)
{
}

QFile::~QFile()
{
}

QString QFile::fileName() const
{
   Q_D(const QFile);
   return d->engine()->fileName(QAbstractFileEngine::DefaultName);
}

void QFile::setFileName(const QString &name)
{
   Q_D(QFile);

   if (isOpen()) {
      qWarning("QFile::setFileName() File (%s) is already opened", csPrintable(fileName()));
      close();
   }

   if (d->fileEngine) { //get a new file engine later
      delete d->fileEngine;
      d->fileEngine = nullptr;
   }

   d->fileName = name;
}

QByteArray QFile::encodeName(const QString &fileName)
{
   return (*QFilePrivate::encoder)(fileName);
}

QString QFile::decodeName(const QByteArray &localFileName)
{
   return (*QFilePrivate::decoder)(localFileName);
}

void QFile::setEncodingFunction(EncoderFn f)
{
   if (!f) {
      f = locale_encode;
   }

   QFilePrivate::encoder = f;
}

void QFile::setDecodingFunction(DecoderFn f)
{
   if (! f) {
      f = locale_decode;
   }

   QFilePrivate::decoder = f;
}

bool QFile::exists() const
{
   Q_D(const QFile);

   // 0x1000000 = QAbstractFileEngine::Refresh, forcing an update

   return (d->engine()->fileFlags(QAbstractFileEngine::FlagsMask | QAbstractFileEngine::FileFlag(0x1000000))
         & QAbstractFileEngine::ExistsFlag);
}

bool QFile::exists(const QString &fileName)
{
   return QFileInfo(fileName).exists();
}

QString QFile::readLink() const
{
   Q_D(const QFile);
   return d->engine()->fileName(QAbstractFileEngine::LinkName);
}

QString QFile::readLink(const QString &fileName)
{
   return QFileInfo(fileName).readLink();
}

bool QFile::remove()
{
   Q_D(QFile);

   if (d->fileName.isEmpty()) {
      qWarning("QFile::remove() File name can not be empty");
      return false;
   }

   unsetError();
   close();

   if (error() == QFile::NoError) {
      if (d->engine()->remove()) {
         unsetError();
         return true;
      }

      d->setError(QFile::RemoveError, d->fileEngine->errorString());
   }

   return false;
}


bool QFile::remove(const QString &fileName)
{
   return QFile(fileName).remove();
}


bool QFile::rename(const QString &newName)
{
   Q_D(QFile);

   if (d->fileName.isEmpty()) {
      qWarning("QFile::rename() File name can not be empty");
      return false;
   }

   if (d->fileName == newName) {
      d->setError(QFile::RenameError, tr("Destination file is the same file."));
      return false;
   }

   if (!exists()) {
      d->setError(QFile::RenameError, tr("Source file does not exist."));
      return false;
   }

   // If the file exists and it is a case-changing rename ("foo" -> "Foo"),
   // compare Ids to make sure it really is a different file.
   if (QFile::exists(newName)) {
      if (d->fileName.compare(newName, Qt::CaseInsensitive)
            || QFileSystemEngine::id(QFileSystemEntry(d->fileName)) != QFileSystemEngine::id(QFileSystemEntry(newName))) {
         // ### Race condition. If a file is moved in after this, it /will/ be
         // overwritten. On Unix, the proper solution is to use hardlinks:
         // return ::link(old, new) && ::remove(old);
         d->setError(QFile::RenameError, tr("Destination file exists"));
         return false;
      }

#ifdef Q_OS_LINUX
      // rename() on Linux simply does nothing when renaming "foo" to "Foo" on a case-insensitive
      // FS, such as FAT32. Move the file away and rename in 2 steps to work around.
      QTemporaryFile tempFile(d->fileName + QString(".XXXXXX"));
      tempFile.setAutoRemove(false);

      if (!tempFile.open(QIODevice::ReadWrite)) {
         d->setError(QFile::RenameError, tempFile.errorString());
         return false;
      }

      tempFile.close();

      if (!d->fileEngine->rename(tempFile.fileName())) {
         d->setError(QFile::RenameError, tr("Error while renaming."));
         return false;
      }

      if (tempFile.rename(newName)) {
         d->fileEngine->setFileName(newName);
         d->fileName = newName;
         return true;
      }

      d->setError(QFile::RenameError, tempFile.errorString());
      // We need to restore the original file

      if (!tempFile.rename(d->fileName)) {
         d->setError(QFile::RenameError, errorString() + '\n' + tr("Unable to restore from %1: %2")
               .formatArgs(QDir::toNativeSeparators(tempFile.fileName()), tempFile.errorString()));
      }

      return false;
#endif
   }

   unsetError();
   close();

   if (error() == QFile::NoError) {
      if (d->engine()->rename(newName)) {
         unsetError();
         // engine was able to handle the new name so we just reset it
         d->fileEngine->setFileName(newName);
         d->fileName = newName;
         return true;
      }

      if (isSequential()) {
         d->setError(QFile::RenameError, tr("Will not rename sequential file using block copy"));
         return false;
      }

      QFile out(newName);

      if (open(QIODevice::ReadOnly)) {
         if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            bool error = false;
            char block[4096];
            qint64 bytes;

            while ((bytes = read(block, sizeof(block))) > 0) {
               if (bytes != out.write(block, bytes)) {
                  d->setError(QFile::RenameError, out.errorString());
                  error = true;
                  break;
               }
            }

            if (bytes == -1) {
               d->setError(QFile::RenameError, errorString());
               error = true;
            }

            if (!error) {
               if (!remove()) {
                  d->setError(QFile::RenameError, tr("Can not remove source file"));
                  error = true;
               }
            }

            if (error) {
               out.remove();
            } else {
               d->fileEngine->setFileName(newName);
               setPermissions(permissions());
               unsetError();
               setFileName(newName);
            }

            close();
            return !error;
         }

         close();
      }

      d->setError(QFile::RenameError, out.isOpen() ? errorString() : out.errorString());
   }

   return false;
}

bool QFile::rename(const QString &oldName, const QString &newName)
{
   return QFile(oldName).rename(newName);
}


bool QFile::link(const QString &linkName)
{
   Q_D(QFile);

   if (d->fileName.isEmpty()) {
      qWarning("QFile::link() File name can not be empty");
      return false;
   }

   QFileInfo fi(linkName);

   if (d->engine()->link(fi.absoluteFilePath())) {
      unsetError();
      return true;
   }

   d->setError(QFile::RenameError, d->fileEngine->errorString());
   return false;
}

bool QFile::link(const QString &fileName, const QString &linkName)
{
   return QFile(fileName).link(linkName);
}

bool QFile::copy(const QString &newName)
{
   Q_D(QFile);

   if (d->fileName.isEmpty()) {
      qWarning("QFile::copy() File name can not be empty");
      return false;
   }

   if (QFile(newName).exists()) {
      // ### Race condition. If a file is moved in after thi, it WILL be overwritten
      // On Unix, the proper solution is to use hardlinks
      // return ::link(old, new) && ::remove(old); See also rename().

      d->setError(QFile::CopyError, tr("Destination file exists"));
      return false;
   }

   unsetError();
   close();

   if (error() == QFile::NoError) {
      if (d->engine()->copy(newName)) {
         unsetError();
         return true;

      } else {
         bool error = false;

         if (!open(QFile::ReadOnly)) {
            error = true;
            d->setError(QFile::CopyError, tr("Can not open %1 for input").formatArg(d->fileName));

         } else {
            QString fileTemplate = "%1/cs_temp.XXXXXX";

#ifdef QT_NO_TEMPORARYFILE
            QFile out(fileTemplate.formatArg(QFileInfo(newName).path()));

            if (! out.open(QIODevice::ReadWrite)) {
               error = true;
            }

#else
            QTemporaryFile out(fileTemplate.formatArg(QFileInfo(newName).path()));

            if (! out.open()) {
               out.setFileTemplate(fileTemplate.formatArg(QDir::tempPath()));

               if (!out.open()) {
                  error = true;
               }
            }

#endif

            if (error) {
               out.close();
               close();
               d->setError(QFile::CopyError, tr("Can not open for output"));

            } else {
               char block[4096];
               qint64 totalRead = 0;

               while (!atEnd()) {
                  qint64 in = read(block, sizeof(block));

                  if (in <= 0) {
                     break;
                  }

                  totalRead += in;

                  if (in != out.write(block, in)) {
                     close();
                     d->setError(QFile::CopyError, tr("Failure to write block"));
                     error = true;
                     break;
                  }
               }

               if (totalRead != size()) {
                  // Unable to read from the source. The error string is already set from read().
                  error = true;
               }

               if (!error && !out.rename(newName)) {
                  error = true;
                  close();
                  d->setError(QFile::CopyError, tr("Can not create %1 for output").formatArg(newName));
               }

#ifdef QT_NO_TEMPORARYFILE

               if (error) {
                  out.remove();
               }

#else

               if (!error) {
                  out.setAutoRemove(false);
               }

#endif
            }
         }

         if (! error) {
            QFile::setPermissions(newName, permissions());
            close();
            unsetError();
            return true;
         }
      }
   }

   return false;
}

bool QFile::copy(const QString &fileName, const QString &newName)
{
   return QFile(fileName).copy(newName);
}

bool QFile::open(OpenMode mode)
{
   Q_D(QFile);

   if (isOpen()) {
      qWarning("QFile::open() File (%s) already open", csPrintable(fileName()));
      return false;
   }

   if (mode & Append) {
      mode |= WriteOnly;
   }

   unsetError();

   if ((mode & (ReadOnly | WriteOnly)) == 0) {
      qWarning("QIODevice::open() Read/Write file access was not specified");
      return false;
   }

   // QIODevice provides the buffering, so there is no need to request it from the file engine.
   if (d->engine()->open(mode | QIODevice::Unbuffered)) {
      QIODevice::open(mode);

      if (mode & Append) {
         // file engine should have done this in open(), workaround for backward compatibility
         seek(size());
      }

      return true;
   }

   QFile::FileError err = d->fileEngine->error();

   if (err == QFile::UnspecifiedError) {
      err = QFile::OpenError;
   }

   d->setError(err, d->fileEngine->errorString());

   return false;
}

bool QFile::open(FILE *fh, OpenMode mode, FileHandleFlags handleFlags)
{
   Q_D(QFile);

   if (isOpen()) {
      qWarning("QFile::open() File (%s) already open", csPrintable(fileName()));
      return false;
   }

   if (mode & Append) {
      mode |= WriteOnly;
   }

   unsetError();

   if ((mode & (ReadOnly | WriteOnly)) == 0) {
      qWarning("QFile::open() Read/Write file access was not specified");
      return false;
   }

   if (d->openExternalFile(mode, fh, handleFlags)) {
      QIODevice::open(mode);

      if (mode & Append) {
         seek(size());
      } else {
         qint64 pos = (qint64)QT_FTELL(fh);

         if (pos != -1) {
            seek(pos);
         }
      }

      return true;
   }

   return false;
}

bool QFile::open(int fd, OpenMode mode, FileHandleFlags handleFlags)
{
   Q_D(QFile);

   if (isOpen()) {
      qWarning("QFile::open() File (%s) already open", csPrintable(fileName()));
      return false;
   }

   if (mode & Append) {
      mode |= WriteOnly;
   }

   unsetError();

   if ((mode & (ReadOnly | WriteOnly)) == 0) {
      qWarning("QFile::open() Read/Write file access was not specified");
      return false;
   }

   if (d->openExternalFile(mode, fd, handleFlags)) {
      QIODevice::open(mode);

      if (mode & Append) {
         seek(size());

      } else {
         qint64 pos = (qint64)QT_LSEEK(fd, QT_OFF_T(0), SEEK_CUR);

         if (pos != -1) {
            seek(pos);
         }
      }

      return true;
   }

   return false;
}

bool QFile::resize(qint64 sz)
{
   return QFileDevice::resize(sz);
}

bool QFile::resize(const QString &fileName, qint64 sz)
{
   return QFile(fileName).resize(sz);
}

QFile::Permissions QFile::permissions() const
{
   return QFileDevice::permissions();
}

QFile::Permissions QFile::permissions(const QString &fileName)
{
   return QFile(fileName).permissions();
}

bool QFile::setPermissions(Permissions permissions)
{
   return QFileDevice::setPermissions(permissions);
}

bool QFile::setPermissions(const QString &fileName, Permissions permissions)
{
   return QFile(fileName).setPermissions(permissions);
}

qint64 QFile::size() const
{
   return QFileDevice::size();
}
