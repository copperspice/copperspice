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

#include <QPainter>
#include <QTcpSocket>
#include <arpa/inet.h>
#include <QBitmap>
#include <QApplication>

#include <QDebug>

#include "qvnccursor.h"
#include "qvncserver.h"
#include "qvncintegration.h"

QT_BEGIN_NAMESPACE

QVNCCursor::QVNCCursor(QVNCServer * srvr, QVNCScreen *scr )
        :QPlatformSoftwareCursor(scr), useVncCursor(false), server(srvr)
{
}

void QVNCCursor::changeCursor(QCursor * widgetCursor, QWidget * widget)
{
    QPlatformSoftwareCursor::changeCursor(widgetCursor, widget);
    if (useVncCursor) {
        server->setDirtyCursor();
    } else {
        setDirty();
    }
}

void QVNCCursor::setCursorMode(bool vnc)
{
    if (vnc) {
        setDirty();
        server->setDirtyCursor();
    } else {
        server->setDirtyCursor();
    }
    useVncCursor = vnc;
}

QRect QVNCCursor::drawCursor(QPainter & painter)
{
    if (useVncCursor)
        return QRect();

    return QPlatformSoftwareCursor::drawCursor(painter);
}

void QVNCCursor::clearClientCursor()
{
    QTcpSocket *socket = server->clientSocket();
    if (!socket) {
        return;
    }
    // FramebufferUpdate header
    {
        const quint16 tmp[6] = { htons(0),
                                 htons(1),
                                 htons(0), htons(0),
                                 htons(0),
                                 htons(0) };
        socket->write((char*)tmp, sizeof(tmp));

        const quint32 encoding = htonl(-239);
        socket->write((char*)(&encoding), sizeof(encoding));
    }
}

void QVNCCursor::sendClientCursor()
{
    if (useVncCursor == false) {
        clearClientCursor();
        return;
    }
    QImage *image = graphic->image();
    if (image->isNull())
        return;
    QTcpSocket *socket = server->clientSocket();
    if (!socket) {
        return;
    }
    // FramebufferUpdate header
    {
        const quint16 tmp[6] = { htons(0),
                                 htons(1),
                                 htons(graphic->hotspot().x()), htons(graphic->hotspot().y()),
                                 htons(image->width()),
                                 htons(image->height()) };
        socket->write((char*)tmp, sizeof(tmp));

        const quint32 encoding = htonl(-239);
        socket->write((char*)(&encoding), sizeof(encoding));
    }

    // write pixels
    //Q_ASSERT(cursor->hasAlphaChannel());
    const QImage img = image->convertToFormat(QImage::Format_RGB32);
    const int n = server->clientBytesPerPixel() * img.width();
    char *buffer = new char[n];
    for (int i = 0; i < img.height(); ++i) {
        server->convertPixels(buffer, (const char*)img.scanLine(i), img.width());
        socket->write(buffer, n);
    }
    delete[] buffer;

    // write mask
    const QImage bitmap = image->createAlphaMask().convertToFormat(QImage::Format_Mono);
    Q_ASSERT(bitmap.depth() == 1);
    Q_ASSERT(bitmap.size() == img.size());
    const int width = (bitmap.width() + 7) / 8;
    for (int i = 0; i < bitmap.height(); ++i)
        socket->write((const char*)bitmap.scanLine(i), width);
}

QT_END_NAMESPACE
