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

#ifndef QSSLCIPHER_P_H
#define QSSLCIPHER_P_H

#include <qsslcipher.h>

QT_BEGIN_NAMESPACE

class QSslCipherPrivate
{
 public:
   QSslCipherPrivate()
      : isNull(true), supportedBits(0), bits(0),
        exportable(false), protocol(QSsl::UnknownProtocol) {
   }

   bool isNull;
   QString name;
   int supportedBits;
   int bits;
   QString keyExchangeMethod;
   QString authenticationMethod;
   QString encryptionMethod;
   bool exportable;
   QString protocolString;
   QSsl::SslProtocol protocol;
};

QT_END_NAMESPACE

#endif
