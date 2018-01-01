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

#ifndef QDIRECTFBMOUSE_H
#define QDIRECTFBMOUSE_H

#include <qglobal.h>
#include <QtGui/qmouse_qws.h>

#ifndef QT_NO_QWS_DIRECTFB

QT_BEGIN_NAMESPACE

class QDirectFBMouseHandlerPrivate;

class QDirectFBMouseHandler : public QWSMouseHandler
{

public:
    explicit QDirectFBMouseHandler(const QString &driver = QString(),const QString &device = QString());
    ~QDirectFBMouseHandler();

    void suspend();
    void resume();

protected:
    QDirectFBMouseHandlerPrivate *d;
};

QT_END_NAMESPACE


#endif // QT_NO_QWS_DIRECTFB

#endif // QDIRECTFBMOUSE_H
