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

#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include "qdesigner.h"
#include "qdesigner_server.h"

#include <qevent.h>

QT_BEGIN_NAMESPACE

// ### review

QDesignerServer::QDesignerServer(QObject *parent)
    : QObject(parent)
{
    m_socket = 0;
    m_server = new QTcpServer(this);
    m_server->listen(QHostAddress::LocalHost, 0);
    if (m_server->isListening())
    {
        connect(m_server, SIGNAL(newConnection()),
                this, SLOT(handleNewConnection()));
    }
}

QDesignerServer::~QDesignerServer()
{
}

quint16 QDesignerServer::serverPort() const
{
    return m_server ? m_server->serverPort() : 0;
}

void QDesignerServer::sendOpenRequest(int port, const QStringList &files)
{
    QTcpSocket *sSocket = new QTcpSocket();
    sSocket->connectToHost(QHostAddress::LocalHost, port);
    if(sSocket->waitForConnected(3000))
    {
        foreach(const QString &file, files)
        {
            QFileInfo fi(file);
            sSocket->write(fi.absoluteFilePath().toUtf8() + '\n');
        }
        sSocket->waitForBytesWritten(3000);
        sSocket->close();
    }
    delete sSocket;
}

void QDesignerServer::readFromClient()
{
    while (m_socket->canReadLine()) {
        QString file = QString::fromUtf8(m_socket->readLine());
        if (!file.isNull()) {
            file.remove(QLatin1Char('\n'));
            file.remove(QLatin1Char('\r'));
            qDesigner->postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

void QDesignerServer::socketClosed()
{
    m_socket = 0;
}

void QDesignerServer::handleNewConnection()
{
    // no need for more than one connection
    if (m_socket == 0) {
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, SIGNAL(readyRead()),
                this, SLOT(readFromClient()));
        connect(m_socket, SIGNAL(disconnected()),
                this, SLOT(socketClosed()));
    }
}


QDesignerClient::QDesignerClient(quint16 port, QObject *parent)
: QObject(parent)
{
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost(QHostAddress::LocalHost, port);
    connect(m_socket, SIGNAL(readyRead()),
                this, SLOT(readFromSocket()));
 
}

QDesignerClient::~QDesignerClient()
{
    m_socket->close();
    m_socket->flush();
}

void QDesignerClient::readFromSocket()
{
    while (m_socket->canReadLine()) {
        QString file = QString::fromUtf8(m_socket->readLine());
        if (!file.isNull()) {
            file.remove(QLatin1Char('\n'));
            file.remove(QLatin1Char('\r'));
            if (QFile::exists(file))
                qDesigner->postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

QT_END_NAMESPACE
