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

#ifndef QSCREENVNC_QWS_H
#define QSCREENVNC_QWS_H

#include <QtGui/qscreenproxy_qws.h>

#ifndef QT_NO_QWS_VNC

QT_BEGIN_NAMESPACE

class QVNCScreenPrivate;

class QVNCScreen : public QProxyScreen
{

public:
    explicit QVNCScreen(int display_id);
    virtual ~QVNCScreen();

    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();

    void setDirty(const QRect&);

private:
    friend class QVNCCursor;
    friend class QVNCClientCursor;
    friend class QVNCServer;
    friend class QVNCScreenPrivate;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    bool swapBytes() const;
#endif

    QVNCScreenPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_VNC
#endif // QSCREENVNC_QWS_H
