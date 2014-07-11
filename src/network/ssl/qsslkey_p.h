/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QSSLKEY_P_H
#define QSSLKEY_P_H

#include <qsslkey.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>

QT_BEGIN_NAMESPACE

class QSslKeyPrivate
{
 public:
   inline QSslKeyPrivate()
      : rsa(0)
      , dsa(0) {
      clear();
   }

   inline ~QSslKeyPrivate() {
      clear();
   }

   void clear(bool deep = true);

   void decodePem(const QByteArray &pem, const QByteArray &passPhrase,
                  bool deepClear = true);
   QByteArray pemHeader() const;
   QByteArray pemFooter() const;
   QByteArray pemFromDer(const QByteArray &der) const;
   QByteArray derFromPem(const QByteArray &pem) const;

   bool isNull;
   QSsl::KeyType type;
   QSsl::KeyAlgorithm algorithm;
   RSA *rsa;
   DSA *dsa;

   QAtomicInt ref;

 private:
   Q_DISABLE_COPY(QSslKeyPrivate)
};

QT_END_NAMESPACE

#endif // QSSLKEY_P_H
