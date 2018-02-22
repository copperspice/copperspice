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

#include <qsslkey.h>
#include <qsslkey_p.h>

#include <CommonCrypto/CommonCrypto.h>

QT_USE_NAMESPACE

static QByteArray wrapCCCrypt(CCOperation ccOp, QSslKeyPrivate::Cipher cipher, const QByteArray &data,
                              const QByteArray &key, const QByteArray &iv)
{
   int blockSize;
   CCAlgorithm ccAlgorithm;

   switch (cipher) {
      case QSslKeyPrivate::DesCbc:
         blockSize = kCCBlockSizeDES;
         ccAlgorithm = kCCAlgorithmDES;
         break;

      case QSslKeyPrivate::DesEde3Cbc:
         blockSize = kCCBlockSize3DES;
         ccAlgorithm = kCCAlgorithm3DES;
         break;

      case QSslKeyPrivate::Rc2Cbc:
         blockSize = kCCBlockSizeRC2;
         ccAlgorithm = kCCAlgorithmRC2;
         break;
   };

   size_t plainLength = 0;
   QByteArray plain(data.size() + blockSize, 0);

   CCCryptorStatus status = CCCrypt(
                               ccOp, ccAlgorithm, kCCOptionPKCS7Padding,
                               key.constData(), key.size(),
                               iv.constData(),
                               data.constData(), data.size(),
                               plain.data(), plain.size(), &plainLength);

   if (status == kCCSuccess) {
      return plain.left(plainLength);
   }

   return QByteArray();
}

QByteArray QSslKeyPrivate::decrypt(Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
   return wrapCCCrypt(kCCDecrypt, cipher, data, key, iv);
}

QByteArray QSslKeyPrivate::encrypt(Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
   return wrapCCCrypt(kCCEncrypt, cipher, data, key, iv);
}
