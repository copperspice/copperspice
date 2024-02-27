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

#ifndef QTEMPORARYFILE_P_H
#define QTEMPORARYFILE_P_H

#include <qfsfileengine_p.h>
#include <qfilesystemengine_p.h>
#include <qfile_p.h>

class QTemporaryFilePrivate : public QFilePrivate
{
 protected:
   QTemporaryFilePrivate();
   ~QTemporaryFilePrivate();

   QAbstractFileEngine *engine() const override;
   void resetFileEngine() const;
   bool autoRemove;
   QString templateName;

   static QString defaultTemplateName();

 private:
   Q_DECLARE_PUBLIC(QTemporaryFile)
   friend class QLockFilePrivate;
};

class QTemporaryFileEngine : public QFSFileEngine
{
   Q_DECLARE_PRIVATE(QFSFileEngine)

 public:
   QTemporaryFileEngine(const QString &file, quint32 mode, bool nameIsTemplate = true)
      : QFSFileEngine(), fileMode(mode), filePathIsTemplate(nameIsTemplate), filePathWasTemplate(nameIsTemplate)
   {
      Q_D(QFSFileEngine);

      d->fileEntry = QFileSystemEntry(file);

      if (! filePathIsTemplate) {
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

   quint32 fileMode;

   bool filePathIsTemplate;
   bool filePathWasTemplate;
};

#endif
