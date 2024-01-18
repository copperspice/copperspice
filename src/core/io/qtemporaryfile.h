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

#ifndef QTEMPORARYFILE_H
#define QTEMPORARYFILE_H

#include <qiodevice.h>
#include <qfile.h>

#ifdef open
#error qtemporaryfile.h must be included before any header file that defines open
#endif

#ifndef QT_NO_TEMPORARYFILE

class QTemporaryFilePrivate;
class QLockFilePrivate;

class Q_CORE_EXPORT QTemporaryFile : public QFile
{
   CORE_CS_OBJECT(QTemporaryFile)

 public:
   QTemporaryFile();
   explicit QTemporaryFile(const QString &tempPath);

   explicit QTemporaryFile(QObject *parent);
   QTemporaryFile(const QString &tempPath, QObject *parent);

   QTemporaryFile(const QTemporaryFile &) = delete;
   QTemporaryFile &operator=(const QTemporaryFile &) = delete;

   ~QTemporaryFile();

   bool autoRemove() const;
   void setAutoRemove(bool b);

   // hide open(flags)
   bool open() {
      return open(QIODevice::ReadWrite);
   }

   QString fileName() const override;
   QString fileTemplate() const;
   void setFileTemplate(const QString &name);

   static QTemporaryFile *createNativeFile(const QString &fileName) {
      QFile file(fileName);
      return createNativeFile(file);
   }

   static QTemporaryFile *createNativeFile(QFile &file);

 protected:
   bool open(OpenMode flags) override;

 private:
   Q_DECLARE_PRIVATE(QTemporaryFile)

   friend class QFile;
   friend class QLockFilePrivate;
};

#endif // QT_NO_TEMPORARYFILE

#endif
