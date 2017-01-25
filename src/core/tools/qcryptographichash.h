/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QCRYPTOGRAPHICHASH_H
#define QCRYPTOGRAPHICHASH_H

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QCryptographicHashPrivate;
class QIODevice;

class Q_CORE_EXPORT QCryptographicHash
{
 public:
   enum Algorithm {
      Md4,
      Md5,
      Sha1
   };

   QCryptographicHash(Algorithm method);
   ~QCryptographicHash();

   void reset();

   void addData(const char *data, int length);
   void addData(const QByteArray &data);
   bool addData(QIODevice *device);

   QByteArray result() const;

   static QByteArray hash(const QByteArray &data, Algorithm method);

 private:
   Q_DISABLE_COPY(QCryptographicHash)
   QCryptographicHashPrivate *d;
};

QT_END_NAMESPACE

#endif
