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

#ifndef QTEMPORARYFILE_P_H
#define QTEMPORARYFILE_P_H

#include <qfsfileengine_p.h>
#include <qfilesystemengine_p.h>
#include <qfile_p.h>

QT_BEGIN_NAMESPACE

class QTemporaryFilePrivate : public QFilePrivate
{
   Q_DECLARE_PUBLIC(QTemporaryFile)

 protected:
   QTemporaryFilePrivate();
   ~QTemporaryFilePrivate();

   bool autoRemove;
   QString templateName;

   static QString defaultTemplateName();
};

class QTemporaryFileEngine : public QFSFileEngine
{
   Q_DECLARE_PRIVATE(QFSFileEngine)

 public:
   QTemporaryFileEngine(const QString &file, bool fileIsTemplate = true)
      : QFSFileEngine(), filePathIsTemplate(fileIsTemplate),
        filePathWasTemplate(fileIsTemplate) {
      Q_D(QFSFileEngine);
      d->fileEntry = QFileSystemEntry(file);

      if (!filePathIsTemplate) {
         QFSFileEngine::setFileName(file);
      }
   }

   ~QTemporaryFileEngine();

   bool isReallyOpen();
   void setFileName(const QString &file) override;
   void setFileTemplate(const QString &fileTemplate);

   bool open(QIODevice::OpenMode flags) override;
   bool remove() override;
   bool rename(const QString &newName) override;
   bool renameOverwrite(const QString &newName) override;
   bool close() override;

   bool filePathIsTemplate;
   bool filePathWasTemplate;
};

QT_END_NAMESPACE

#endif /* QTEMPORARYFILE_P_H */

