/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QRESOURCE_P_H
#define QRESOURCE_P_H

#include <QtCore/qabstractfileengine.h>

QT_BEGIN_NAMESPACE

class QResourceFileEnginePrivate;

class QResourceFileEngine : public QAbstractFileEngine
{
   Q_DECLARE_PRIVATE(QResourceFileEngine)

 public:
   explicit QResourceFileEngine(const QString &path);
   ~QResourceFileEngine();

   virtual void setFileName(const QString &file);

   virtual bool open(QIODevice::OpenMode flags) ;
   virtual bool close();
   virtual bool flush();
   virtual qint64 size() const;
   virtual qint64 pos() const;
   virtual bool atEnd() const;
   virtual bool seek(qint64);
   virtual qint64 read(char *data, qint64 maxlen);
   virtual qint64 write(const char *data, qint64 len);

   virtual bool remove();
   virtual bool copy(const QString &newName);
   virtual bool rename(const QString &newName);
   virtual bool link(const QString &newName);

   virtual bool isSequential() const;

   virtual bool isRelativePath() const;

   virtual bool mkdir(const QString &dirName, bool createParentDirectories) const;
   virtual bool rmdir(const QString &dirName, bool recurseParentDirectories) const;

   virtual bool setSize(qint64 size);

   virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;

   virtual bool caseSensitive() const;

   virtual FileFlags fileFlags(FileFlags type) const;

   virtual bool setPermissions(uint perms);

   virtual QString fileName(QAbstractFileEngine::FileName file) const;

   virtual uint ownerId(FileOwner) const;
   virtual QString owner(FileOwner) const;

   virtual QDateTime fileTime(FileTime time) const;

   virtual Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);
   virtual Iterator *endEntryList();

   bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0);
   bool supportsExtension(Extension extension) const;
};

QT_END_NAMESPACE

#endif // QRESOURCE_P_H
