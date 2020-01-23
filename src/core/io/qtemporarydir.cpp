/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qtemporarydir.h>

#ifndef QT_NO_TEMPORARYFILE

#include <qdiriterator.h>
#include <qdir_p.h>
#include <qdebug.h>
#include <qstring16.h>

#if defined(QT_BUILD_CORE_LIB)
#include <qcoreapplication.h>
#endif

#include <stdlib.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <qfsfileengine_p.h>
#endif

class QTemporaryDirPrivate
{
 public:
   QTemporaryDirPrivate();
   ~QTemporaryDirPrivate();

   void create(const QString &templateName);

   QString path;
   bool autoRemove;
   bool success;
};

QTemporaryDirPrivate::QTemporaryDirPrivate()
   : autoRemove(true),
     success(false)
{
}

QTemporaryDirPrivate::~QTemporaryDirPrivate()
{
}

static QString defaultTemplateName()
{
   QString baseName;

#if defined(QT_BUILD_CORE_LIB)
   baseName = QCoreApplication::applicationName();
   if (baseName.isEmpty())
#endif

      baseName = "cs_temp";

   return QDir::tempPath() + '/' + baseName + "-XXXXXX";
}

void QTemporaryDirPrivate::create(const QString &templateName)
{
#ifdef Q_OS_WIN
   QString buffer = templateName;

   // Windows mktemp believes 26 temp files per process ought to be enough for everyone
   // add a few random char, before the XXXXXX template.

   for (int i = 0 ; i < 4 ; ++i) {
      buffer += QChar((qrand() & 0xffff) % (26) + 'A');
   }

   if (! buffer.endsWith("XXXXXX")) {
      buffer += "XXXXXX";
   }

   QFileSystemEntry baseEntry(buffer);
   QFileSystemEntry::NativePath basePath = baseEntry.nativeFilePath();

   std::wstring array = basePath.toStdWString();

   if (_wmktemp(&array[0]) && ::CreateDirectory(array.c_str(), 0)) {
      success = true;

      QFileSystemEntry entry(QString::fromStdWString(array), QFileSystemEntry::FromNativePath());
      path = entry.filePath();
   }

#else
   QByteArray buffer = QFile::encodeName(templateName);

   if (! buffer.endsWith("XXXXXX")) {
      buffer += "XXXXXX";
   }

   if (mkdtemp(buffer.data())) {
      // modifies buffer
      success = true;
      path = QFile::decodeName(buffer.constData());
   }
#endif
}

QTemporaryDir::QTemporaryDir()
   : d_ptr(new QTemporaryDirPrivate)
{
   d_ptr->create(defaultTemplateName());
}

QTemporaryDir::QTemporaryDir(const QString &templateName)
   : d_ptr(new QTemporaryDirPrivate)
{
   if (templateName.isEmpty()) {
      d_ptr->create(defaultTemplateName());
   } else {
      d_ptr->create(templateName);
   }
}

QTemporaryDir::~QTemporaryDir()
{
   if (d_ptr->autoRemove) {
      remove();
   }
}

bool QTemporaryDir::isValid() const
{
   return d_ptr->success;
}

QString QTemporaryDir::path() const
{
   return d_ptr->path;
}

bool QTemporaryDir::autoRemove() const
{
   return d_ptr->autoRemove;
}

void QTemporaryDir::setAutoRemove(bool b)
{
   d_ptr->autoRemove = b;
}

/*!
    Removes the temporary directory, including all its contents.
*/
bool QTemporaryDir::remove()
{
   if (!d_ptr->success) {
      return false;
   }
   Q_ASSERT(!path().isEmpty());
   Q_ASSERT(path() != QLatin1String("."));

   const bool result = QDir(path()).removeRecursively();
   if (!result) {
      qWarning() << "QTemporaryDir: Unable to remove"
                 << QDir::toNativeSeparators(path())
                 << "most likely due to the presence of read-only files.";
   }
   return result;
}

#endif // QT_NO_TEMPORARYFILE
