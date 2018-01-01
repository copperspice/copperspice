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

#ifndef QVNCCURSOR_H
#define QVNCCURSOR_H

#include "../fb_base/fb_base.h"
#include <QList>
#include <QImage>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE

class QVNCScreen;
class QVNCServer;

class QVNCCursor : public QPlatformSoftwareCursor {
public:
    QVNCCursor(QVNCServer *, QVNCScreen *);

    // input methods
    void setCursorMode(bool vnc);
    void changeCursor(QCursor * widgetCursor, QWidget * widget);

    // output methods
    QRect drawCursor(QPainter &);

    // VNC client communication
    void sendClientCursor();
    void clearClientCursor();
private:
    bool useVncCursor;      // VNC or local

    QVNCServer * server;    // VNC server to get events from
};

QT_END_NAMESPACE

#endif // QVNCCURSOR_H
