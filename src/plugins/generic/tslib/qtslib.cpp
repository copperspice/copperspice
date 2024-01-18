/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#include "qtslib.h"

#include <QSocketNotifier>
#include <QStringList>
#include <QPoint>
#include <QWindowSystemInterface>
#include <qnamespace.h>

#include <errno.h>
#include <tslib.h>

#include <qdebug.h>

QT_BEGIN_NAMESPACE

QTsLibMouseHandler::QTsLibMouseHandler(const QString &key,
                                                 const QString &specification)
    : m_notify(0), m_x(0), m_y(0), m_pressed(0), m_rawMode(false)
{
    qDebug() << "QTsLibMouseHandler" << key << specification;
    setObjectName(QLatin1String("TSLib Mouse Handler"));

    QByteArray device = "/dev/input/event1";
    if (specification.startsWith("/dev/"))
        device = specification.toLocal8Bit();

    m_dev =  ts_open(device.constData(), 1);

    if (ts_config(m_dev)) {
        perror("Error configuring\n");
    }


    m_rawMode =  !key.compare(QLatin1String("TslibRaw"), Qt::CaseInsensitive);

    int fd = ts_fd(m_dev);
    if (fd >= 0) {
        m_notify = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(m_notify, SIGNAL(activated(int)), this, SLOT(readMouseData()));
    } else {
        qWarning("Unable to open mouse input device '%s': %s", device.constData(), strerror(errno));
        return;
    }
}


QTsLibMouseHandler::~QTsLibMouseHandler()
{
    if (m_dev)
        ts_close(m_dev);
}


static bool get_sample(struct tsdev *dev, struct ts_sample *sample, bool rawMode)
{
    if (rawMode) {
        return (ts_read_raw(dev, sample, 1) == 1);
    } else {
        int ret = ts_read(dev, sample, 1);
        return ( ret == 1);
    }
}


void QTsLibMouseHandler::readMouseData()
{
    ts_sample sample;

    while (get_sample(m_dev, &sample, m_rawMode)) {
        bool pressed = sample.pressure;
        int x = sample.x;
        int y = sample.y;

        // work around missing coordinates on mouse release
        if (sample.pressure == 0 && sample.x == 0 && sample.y == 0) {
            x = m_x;
            y = m_y;
        }

        if (!m_rawMode) {
            //filtering: ignore movements of 2 pixels or less
            int dx = x - m_x;
            int dy = y - m_y;
            if (dx*dx <= 4 && dy*dy <= 4 && pressed == m_pressed)
                continue;
        }
        QPoint pos(x, y);

        //printf("handleMouseEvent %d %d %d %ld\n", m_x, m_y, pressed, sample.tv.tv_usec);

        QWindowSystemInterface::handleMouseEvent(0, pos, pos, pressed ? Qt::LeftButton : Qt::NoButton);

        m_x = x;
        m_y = y;
        m_pressed = pressed;
    }
}

