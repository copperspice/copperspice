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

#ifndef QHOSTADDRESS_P_H
#define QHOSTADDRESS_P_H

QT_BEGIN_NAMESPACE

#include "qhostaddress.h"
#include "qabstractsocket.h"

class QNetmaskAddress: public QHostAddress
{
   int length;
 public:
   QNetmaskAddress() : QHostAddress(), length(-1) { }

   bool setAddress(const QString &address);
   bool setAddress(const QHostAddress &address);

   int prefixLength() const;
   void setPrefixLength(QAbstractSocket::NetworkLayerProtocol proto, int len);
};


QT_END_NAMESPACE

#endif
