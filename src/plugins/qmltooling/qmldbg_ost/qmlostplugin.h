/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QMLOSTPLUGIN_H
#define QMLOSTPLUGIN_H

#include <QtGui/QStylePlugin>
#include <qdeclarativedebugserverconnection_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeDebugServer;
class QmlOstPluginPrivate;

class QmlOstPlugin : public QObject, public QDeclarativeDebugServerConnection
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QmlOstPlugin)
    Q_DISABLE_COPY(QmlOstPlugin)
    Q_INTERFACES(QDeclarativeDebugServerConnection)


public:
    QmlOstPlugin();
    ~QmlOstPlugin();

    void setServer(QDeclarativeDebugServer *server);
    void setPort(int port, bool bock);

    bool isConnected() const;
    void send(const QByteArray &message);
    void disconnect();
    bool waitForMessage();

private Q_SLOTS:
    void readyRead();

private:
    QmlOstPluginPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QMLOSTPLUGIN_H
