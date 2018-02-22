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

#ifndef QABSTRACTPROTOCOLHANDLER_H
#define QABSTRACTPROTOCOLHANDLER_H

#include <qglobal.h>

class QHttpNetworkConnectionChannel;
class QHttpNetworkReply;
class QAbstractSocket;
class QHttpNetworkConnection;

class QAbstractProtocolHandler {

 public:
    QAbstractProtocolHandler(QHttpNetworkConnectionChannel *channel);
    virtual ~QAbstractProtocolHandler();

    virtual void _q_receiveReply() = 0;
    virtual void _q_readyRead() = 0;
    virtual bool sendRequest() = 0;
    void setReply(QHttpNetworkReply *reply);

 protected:
    QHttpNetworkConnectionChannel *m_channel;
    QHttpNetworkReply *m_reply;
    QAbstractSocket *m_socket;
    QHttpNetworkConnection *m_connection;
};

#endif // QABSTRACTPROTOCOLHANDLER_H
