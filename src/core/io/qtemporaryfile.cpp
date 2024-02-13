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

#include <qtemporaryfile.h>

#ifndef QT_NO_TEMPORARYFILE

#include <qcoreapplication.h>
#include <qplatformdefs.h>
#include <qregularexpression.h>

#include <qfile_p.h>
#include <qtemporaryfile_p.h>
#include <qsystemerror_p.h>

#if ! defined(Q_OS_WIN)
#include <qcore_unix_p.h>       // overrides QT_OPEN
#include <errno.h>
#endif

#if defined(Q_OS_WIN)
using NativeFileHandle = HANDLE;
#else
using NativeFileHandle = int;
#endif

static QString createFileName(QString fName, size_t pos, size_t length)
{
   auto placeholderStart_iter = fName.begin() + pos;

   QString newText;

   for (size_t cnt = 0; cnt < length; ++cnt) {
      char32_t ch = static_cast<char32_t>((qrand() & 0xffff) % (26 + 26));

      if (ch < 26) {
         newText.append(static_cast<char32_t>(ch + U'A'));
      } else {
         newText.append(static_cast<char32_t>(ch - 26 + U'a'));
      }
   }

   fName.replace(placeholderStart_iter, placeholderStart_iter + length, newText);

   return fName;
}

static bool createFileFromTemplate(NativeFileHandle &fHandle, QString &fName,
      size_t pos, size_t length, quint32 mode, QSystemError &error)
{
   for (int cntLimit = 0; cntLimit < 100; ++cntLimit) {
      // create file and obtain handle

      fName = createFileName(fName, pos, length);

#if defined(Q_OS_WIN)
      (void) mode;

      fHandle = CreateFile(&fName.toStdWString()[0], GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

      if (fHandle != INVALID_HANDLE_VALUE) {
         // opened successfully
         return true;
      }

      DWORD err = GetLastError();

      if (err != ERROR_FILE_EXISTS) {
         // file name already in use, get another name
         continue;

      } else if (err == ERROR_ACCESS_DENIED) {
         DWORD attributes = GetFileAttributes(&fName.toStdWString()[0]);

         if (attributes == INVALID_FILE_ATTRIBUTES) {
            // Potential write error (read-only parent directory, etc)
            error = QSystemError(err, QSystemError::NativeError);
            return false;
         }

         // else file already exists as a directory

      } else if (err != ERROR_FILE_EXISTS) {
         error = QSystemError(err, QSystemError::NativeError);
         return false;

      }

#else
      fHandle = QT_OPEN(fName.constData(), QT_OPEN_CREAT | O_EXCL | QT_OPEN_RDWR | QT_OPEN_LARGEFILE,
                  static_cast<mode_t>(mode));

      if (fHandle != -1) {
         // opened successfully
         return true;
      }

      if (errno == EEXIST) {
         // file name already in use, get another name
         continue;

      } else {
         error = QSystemError(errno, QSystemError::NativeError);
         return false;
      }

#endif
   }

   return false;
}

QTemporaryFileEngine::~QTemporaryFileEngine()
{
   QFSFileEngine::close();
}

bool QTemporaryFileEngine::isReallyOpen()
{
   Q_D(QFSFileEngine);

   if (! ((nullptr == d->fh) && (-1 == d->fd)

#if defined Q_OS_WIN
         && (INVALID_HANDLE_VALUE == d->fileHandle)
#endif
         )) {

      return true;
   }

   return false;
}

void QTemporaryFileEngine::setFileName(const QString &file)
{
   // close the file first to prevent a warning
   QFSFileEngine::close();

   QFSFileEngine::setFileName(file);
}

void QTemporaryFileEngine::setFileTemplate(const QString &fileTemplate)
{
   Q_D(QFSFileEngine);

   if (filePathIsTemplate) {
      d->fileEntry = QFileSystemEntry(fileTemplate);
   }
}

bool QTemporaryFileEngine::open(QIODevice::OpenMode openMode)
{
   Q_D(QFSFileEngine);

   Q_ASSERT(! isReallyOpen());

   openMode |= QIODevice::ReadWrite;

   if (! filePathIsTemplate) {
      return QFSFileEngine::open(openMode);
   }

   QString tmpName = d->fileEntry.filePath();

   // ensure there is a placeholder mask
   QRegularExpression regExp("(X{6,})[^X/]*$");
   QRegularExpressionMatch match = regExp.match(tmpName);

   if (! match.hasMatch()) {
      tmpName.append(".XXXXXX");
   }

   QFileSystemEntry entry(tmpName, QFileSystemEntry::FromInternalPath());
   QString fName = QFileSystemEngine::absoluteName(entry).nativeFilePath();

   // Find mask in native path
   uint pos;
   uint length;

   match = regExp.match(fName);

   if (match.hasMatch()) {
      pos    = match.capturedStart(1) - fName.cbegin();
      length = match.capturedLength(1);

   } else {
      // something is wrong
      return false;
   }

   QSystemError error;

#if defined(Q_OS_WIN)
   NativeFileHandle &fHandle = d->fileHandle;
#else
   NativeFileHandle &fHandle = d->fd;
#endif

   if (! createFileFromTemplate(fHandle, fName, pos, length, fileMode, error)) {
      setError(QFile::OpenError, error.toString());
      return false;
   }

   d->fileEntry = QFileSystemEntry(fName, QFileSystemEntry::FromNativePath());

#if ! defined(Q_OS_WIN)
   d->closeFileHandle = true;
#endif

   filePathIsTemplate = false;
   d->openMode        = openMode;
   d->lastFlushFailed = false;
   d->tried_stat      = 0;

   return true;
}

bool QTemporaryFileEngine::remove()
{
   Q_D(QFSFileEngine);

   // Since the QTemporaryFileEngine::close() does not really close the file
   // we must explicitly call QFSFileEngine::close() before we remove it.

   QFSFileEngine::close();

   if (QFSFileEngine::remove()) {

      d->fileEntry.clear();

      // If a QTemporaryFile is constructed using a template file path, the path
      // is generated in QTemporaryFileEngine::open() and then filePathIsTemplate
      // is set to false. If remove() and then open() are called on the same
      // QTemporaryFile, the path is not regenerated. Here we ensure that if the
      // file path was generated, it will be generated again in the scenario above.

      filePathIsTemplate = filePathWasTemplate;
      return true;
   }

   return false;
}

bool QTemporaryFileEngine::rename(const QString &newName)
{
   // close the file first to prevent a warning
   QFSFileEngine::close();

   return QFSFileEngine::rename(newName);
}

bool QTemporaryFileEngine::renameOverwrite(const QString &newName)
{
   // close the file first to prevent a warning
   QFSFileEngine::close();

   return QFSFileEngine::renameOverwrite(newName);
}

bool QTemporaryFileEngine::close()
{
   // do not close the file, just seek to the front
   seek(0);
   setError(QFile::UnspecifiedError, QString());

   return true;
}

QTemporaryFilePrivate::QTemporaryFilePrivate()
   : autoRemove(true)
{
}

QTemporaryFilePrivate::~QTemporaryFilePrivate()
{
}

QAbstractFileEngine *QTemporaryFilePrivate::engine() const
{
   if (fileEngine == nullptr) {
      resetFileEngine();
   }

   return fileEngine;
}

void QTemporaryFilePrivate::resetFileEngine() const
{
   delete fileEngine;

   if (fileName.isEmpty()) {
      fileEngine = new QTemporaryFileEngine(templateName, 0600);

   } else {
      fileEngine = new QTemporaryFileEngine(fileName, 0600, false);

   }
}

static QString defaultTemplateName()
{
   QString baseName = QCoreApplication::applicationName();

   if (baseName.isEmpty()) {
      baseName = "cs_temp";
   }

   return QDir::tempPath() + QChar('/') + baseName + QString(".XXXXXX");
}

QTemporaryFile::QTemporaryFile()
   : QFile(*new QTemporaryFilePrivate, nullptr)
{
   Q_D(QTemporaryFile);
   d->templateName = defaultTemplateName();

   // uses the form "c_temp.XXXXXX"
}

QTemporaryFile::QTemporaryFile(const QString &templateName)
   : QFile(*new QTemporaryFilePrivate, nullptr)
{
   Q_D(QTemporaryFile);
   d->templateName = templateName;
}

QTemporaryFile::QTemporaryFile(QObject *parent)
   : QFile(*new QTemporaryFilePrivate, parent)
{
   Q_D(QTemporaryFile);
   d->templateName = defaultTemplateName();

   // uses the form "cs_temp.XXXXXX"
}

QTemporaryFile::QTemporaryFile(const QString &templateName, QObject *parent)
   : QFile(*new QTemporaryFilePrivate, parent)
{
   Q_D(QTemporaryFile);
   d->templateName = templateName;
}

QTemporaryFile::~QTemporaryFile()
{
   Q_D(QTemporaryFile);
   close();

   if (! d->fileName.isEmpty() && d->autoRemove) {
      remove();
   }
}

bool QTemporaryFile::autoRemove() const
{
   Q_D(const QTemporaryFile);
   return d->autoRemove;
}

void QTemporaryFile::setAutoRemove(bool b)
{
   Q_D(QTemporaryFile);
   d->autoRemove = b;
}

QString QTemporaryFile::fileName() const
{
   Q_D(const QTemporaryFile);

   if (d->fileName.isEmpty()) {
      return QString();
   }

   return d->engine()->fileName(QAbstractFileEngine::DefaultName);
}

QString QTemporaryFile::fileTemplate() const
{
   Q_D(const QTemporaryFile);
   return d->templateName;
}

void QTemporaryFile::setFileTemplate(const QString &name)
{
   Q_D(QTemporaryFile);
   d->templateName = name;

   if (d->fileEngine) {
      static_cast<QTemporaryFileEngine *>(d->fileEngine)->setFileTemplate(name);
   }
}

QTemporaryFile *QTemporaryFile::createNativeFile(QFile &file)
{
   if (QAbstractFileEngine *engine = file.d_func()->engine()) {
      if (engine->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::LocalDiskFlag) {
         return nullptr;   // local already
      }

      //cache
      bool wasOpen = file.isOpen();
      qint64 old_off = 0;

      if (wasOpen) {
         old_off = file.pos();
      } else {
         file.open(QIODevice::ReadOnly);
      }

      //dump data
      QTemporaryFile *ret = new QTemporaryFile;
      ret->open();
      file.seek(0);
      char buffer[1024];

      while (true) {
         qint64 len = file.read(buffer, 1024);

         if (len < 1) {
            break;
         }

         ret->write(buffer, len);
      }

      ret->seek(0);
      //restore

      if (wasOpen) {
         file.seek(old_off);
      } else {
         file.close();
      }

      //done
      return ret;
   }

   return nullptr;
}

bool QTemporaryFile::open(OpenMode flags)
{
   Q_D(QTemporaryFile);

   if (! d->fileName.isEmpty()) {
      if (static_cast<QTemporaryFileEngine *>(d->engine())->isReallyOpen()) {
         setOpenMode(flags);
         return true;
      }
   }

   // reset the engine state so it creates a new, unique file name from the template;
   d->resetFileEngine();

   if (QFile::open(flags)) {
      d->fileName = d->fileEngine->fileName(QAbstractFileEngine::DefaultName);
      return true;
   }

   return false;
}

#endif // QT_NO_TEMPORARYFILE
