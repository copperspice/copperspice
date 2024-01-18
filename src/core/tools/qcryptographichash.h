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

#ifndef QCRYPTOGRAPHICHASH_H
#define QCRYPTOGRAPHICHASH_H

#include <qbytearray.h>

class QCryptographicHashPrivate;
class QIODevice;

class Q_CORE_EXPORT QCryptographicHash
{
 public:
   enum Algorithm {
      Md4,
      Md5,
      Sha1,
      Sha224,
      Sha256,
      Sha384,
      Sha512,
      Keccak_224,
      Keccak_256,
      Keccak_384,
      Keccak_512,
      Sha3_224,
      Sha3_256,
      Sha3_384,
      Sha3_512
   };

   explicit QCryptographicHash(Algorithm method);

   QCryptographicHash(const QCryptographicHash &) = delete;
   QCryptographicHash &operator=(const QCryptographicHash &) = delete;

   ~QCryptographicHash();

   void reset();

   void addData(const char *data, int length);
   void addData(const QByteArray &data);
   bool addData(QIODevice *device);

   QByteArray result() const;

   static QByteArray hash(const QByteArray &data, Algorithm method);

 private:
   QCryptographicHashPrivate *d;
};

#endif
