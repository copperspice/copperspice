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

#include <qabstractprotocolhandler_p.h>
#include <qhttpnetworkconnectionchannel_p.h>

QAbstractProtocolHandler::QAbstractProtocolHandler(QHttpNetworkConnectionChannel *channel)
   : m_channel(channel), m_reply(0), m_socket(m_channel->socket), m_connection(m_channel->connection)
{
   Q_ASSERT(m_channel);
   Q_ASSERT(m_socket);
   Q_ASSERT(m_connection);
}

QAbstractProtocolHandler::~QAbstractProtocolHandler()
{
}

void QAbstractProtocolHandler::setReply(QHttpNetworkReply *reply)
{
   m_reply = reply;
}

