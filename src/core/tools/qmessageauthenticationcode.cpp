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

#include <qmessageauthenticationcode.h>
#include <qvarlengtharray.h>

#include <stdint.h>

#include "../../3rdparty/rfc6234/sha.h"

static int qt_hash_block_size(QCryptographicHash::Algorithm method)
{
   switch (method) {
      case QCryptographicHash::Md4:
         return 64;

      case QCryptographicHash::Md5:
         return 64;

      case QCryptographicHash::Sha1:
         return SHA1_Message_Block_Size;

      case QCryptographicHash::Sha224:
         return SHA224_Message_Block_Size;

      case QCryptographicHash::Sha256:
         return SHA256_Message_Block_Size;

      case QCryptographicHash::Sha384:
         return SHA384_Message_Block_Size;

      case QCryptographicHash::Sha512:
         return SHA512_Message_Block_Size;

      case QCryptographicHash::Sha3_224:
      case QCryptographicHash::Keccak_224:
         return 144;

      case QCryptographicHash::Sha3_256:
      case QCryptographicHash::Keccak_256:
         return 136;

      case QCryptographicHash::Sha3_384:
      case QCryptographicHash::Keccak_384:
         return 104;

      case QCryptographicHash::Sha3_512:
      case QCryptographicHash::Keccak_512:
         return 72;

   }

   return 0;
}

class QMessageAuthenticationCodePrivate
{
 public:
   QMessageAuthenticationCodePrivate(QCryptographicHash::Algorithm m)
      : messageHash(m), method(m), messageHashInited(false)
   {
   }

   QByteArray key;
   QByteArray result;
   QCryptographicHash messageHash;
   QCryptographicHash::Algorithm method;
   bool messageHashInited;

   void initMessageHash();
};

void QMessageAuthenticationCodePrivate::initMessageHash()
{
   if (messageHashInited) {
      return;
   }

   messageHashInited = true;

   const int blockSize = qt_hash_block_size(method);

   if (key.size() > blockSize) {
      QCryptographicHash hash(method);
      hash.addData(key);
      key = hash.result();
      hash.reset();
   }

   if (key.size() < blockSize) {
      const int size = key.size();
      key.resize(blockSize);
      memset(key.data() + size, 0, blockSize - size);
   }

   QVarLengthArray<char> iKeyPad(blockSize);
   const char *const keyData = key.constData();

   for (int i = 0; i < blockSize; ++i) {
      iKeyPad[i] = keyData[i] ^ 0x36;
   }

   messageHash.addData(iKeyPad.data(), iKeyPad.size());
}

QMessageAuthenticationCode::QMessageAuthenticationCode(QCryptographicHash::Algorithm method,
      const QByteArray &key)
   : d(new QMessageAuthenticationCodePrivate(method))
{
   d->key = key;
}

QMessageAuthenticationCode::~QMessageAuthenticationCode()
{
   delete d;
}

void QMessageAuthenticationCode::reset()
{
   d->result.clear();
   d->messageHash.reset();
   d->messageHashInited = false;
}

void QMessageAuthenticationCode::setKey(const QByteArray &key)
{
   reset();
   d->key = key;
}

void QMessageAuthenticationCode::addData(const char *data, int length)
{
   d->initMessageHash();
   d->messageHash.addData(data, length);
}

void QMessageAuthenticationCode::addData(const QByteArray &data)
{
   d->initMessageHash();
   d->messageHash.addData(data);
}

bool QMessageAuthenticationCode::addData(QIODevice *device)
{
   d->initMessageHash();
   return d->messageHash.addData(device);
}

QByteArray QMessageAuthenticationCode::result() const
{
   if (! d->result.isEmpty()) {
      return d->result;
   }

   d->initMessageHash();

   const int blockSize = qt_hash_block_size(d->method);

   QByteArray hashedMessage = d->messageHash.result();

   QVarLengthArray<char> oKeyPad(blockSize);
   const char *const keyData = d->key.constData();

   for (int i = 0; i < blockSize; ++i) {
      oKeyPad[i] = keyData[i] ^ 0x5c;
   }

   QCryptographicHash hash(d->method);
   hash.addData(oKeyPad.data(), oKeyPad.size());
   hash.addData(hashedMessage);

   d->result = hash.result();
   return d->result;
}

QByteArray QMessageAuthenticationCode::hash(const QByteArray &message,
      const QByteArray &key, QCryptographicHash::Algorithm method)
{
   QMessageAuthenticationCode mac(method);
   mac.setKey(key);
   mac.addData(message);
   return mac.result();
}
