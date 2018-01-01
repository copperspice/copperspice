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

#ifndef QAHISCREEN_H
#define QAHISCREEN_H

#include <QtGui/qscreenlinuxfb_qws.h>

#ifndef QT_NO_QWS_AHI

QT_BEGIN_NAMESPACE

class QAhiScreenPrivate;

class QAhiScreen : public QScreen
{

public:
    QAhiScreen(int displayId);
    ~QAhiScreen();

    bool connect(const QString &displaySpec);
    void disconnect();
    bool initDevice();
    void shutdownDevice();
    void setMode(int width, int height, int depth);

    void blit(const QImage &image, const QPoint &topLeft,
              const QRegion &region);
    void solidFill(const QColor &color, const QRegion &region);

private:
    bool configure();

    QAhiScreenPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_AHI
#endif // QAHISCREEN_H
