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

#ifndef QURLINFO_P_H
#define QURLINFO_P_H

#include <qdatetime.h>
#include <qstring.h>
#include <qiodevice.h>


#ifndef QT_NO_FTP
class QUrl;
class QUrlInfoPrivate;

class Q_NETWORK_EXPORT QUrlInfo
{
 public:
   enum PermissionSpec {
      ReadOwner = 00400, WriteOwner = 00200, ExeOwner = 00100,
      ReadGroup = 00040, WriteGroup = 00020, ExeGroup = 00010,
      ReadOther = 00004, WriteOther = 00002, ExeOther = 00001
   };

   QUrlInfo();
   QUrlInfo(const QUrlInfo &ui);
   QUrlInfo(const QString &name, int permissions, const QString &owner,
            const QString &group, qint64 size, const QDateTime &lastModified,
            const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
            bool isWritable, bool isReadable, bool isExecutable);
   QUrlInfo(const QUrl &url, int permissions, const QString &owner,
            const QString &group, qint64 size, const QDateTime &lastModified,
            const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
            bool isWritable, bool isReadable, bool isExecutable);
   QUrlInfo &operator=(const QUrlInfo &ui);
   virtual ~QUrlInfo();

   virtual void setName(const QString &name);
   virtual void setDir(bool b);
   virtual void setFile(bool b);
   virtual void setSymLink(bool b);
   virtual void setOwner(const QString &s);
   virtual void setGroup(const QString &s);
   virtual void setSize(qint64 size);
   virtual void setWritable(bool b);
   virtual void setReadable(bool b);
   virtual void setPermissions(int p);
   virtual void setLastModified(const QDateTime &dt);
   void setLastRead(const QDateTime &dt);

   bool isValid() const;

   QString name() const;
   int permissions() const;
   QString owner() const;
   QString group() const;
   qint64 size() const;
   QDateTime lastModified() const;
   QDateTime lastRead() const;
   bool isDir() const;
   bool isFile() const;
   bool isSymLink() const;
   bool isWritable() const;
   bool isReadable() const;
   bool isExecutable() const;

   static bool greaterThan(const QUrlInfo &i1, const QUrlInfo &i2, int sortBy);
   static bool lessThan(const QUrlInfo &i1, const QUrlInfo &i2, int sortBy);
   static bool equal(const QUrlInfo &i1, const QUrlInfo &i2,int sortBy);

   bool operator==(const QUrlInfo &i) const;
   inline bool operator!=(const QUrlInfo &i) const {
      return !operator==(i);
   }

 private:
   QUrlInfoPrivate *d;
};

#endif // QT_NO_FTP


#endif // QURLINFO_H
