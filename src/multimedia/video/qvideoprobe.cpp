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

#include <qvideoprobe.h>

#include <qsharedpointer.h>
#include <qpointer.h>
#include <qmediavideoprobecontrol.h>
#include <qmediaservice.h>
#include <qmediarecorder.h>

class QVideoProbePrivate {
  public:
    QPointer<QMediaObject> source;
    QPointer<QMediaVideoProbeControl> probee;
};

QVideoProbe::QVideoProbe(QObject *parent)
    : QObject(parent), d(new QVideoProbePrivate)
{
}

QVideoProbe::~QVideoProbe()
{
    if (d->source) {
        if (d->probee) {
            disconnect(d->probee.data(), &QMediaVideoProbeControl::videoFrameProbed, this, &QVideoProbe::videoFrameProbed);
            disconnect(d->probee.data(), &QMediaVideoProbeControl::flush,            this, &QVideoProbe::flush);
        }

        d->source.data()->service()->releaseControl(d->probee.data());
    }
}

bool QVideoProbe::setSource(QMediaObject *source)
{
    // Need to:
    // 1) disconnect from current source if necessary
    // 2) see if new one has the probe control
    // 3) connect if so

    // in case source was destroyed but probe control is still valid
    if (! d->source && d->probee) {
        disconnect(d->probee.data(), &QMediaVideoProbeControl::videoFrameProbed, this, &QVideoProbe::videoFrameProbed);
        disconnect(d->probee.data(), &QMediaVideoProbeControl::flush,            this, &QVideoProbe::flush);
        d->probee.clear();
    }

    if (source != d->source.data()) {
        if (d->source) {
            Q_ASSERT(d->probee);
            disconnect(d->probee.data(), &QMediaVideoProbeControl::videoFrameProbed, this, &QVideoProbe::videoFrameProbed);
            disconnect(d->probee.data(), &QMediaVideoProbeControl::flush,            this, &QVideoProbe::flush);

            d->source.data()->service()->releaseControl(d->probee.data());
            d->source.clear();
            d->probee.clear();
        }

        if (source) {
            QMediaService *service = source->service();
            if (service) {
                d->probee = service->requestControl<QMediaVideoProbeControl*>();
            }

            if (d->probee) {
               connect(d->probee.data(), &QMediaVideoProbeControl::videoFrameProbed, this, &QVideoProbe::videoFrameProbed);
               connect(d->probee.data(), &QMediaVideoProbeControl::flush,            this, &QVideoProbe::flush);
               d->source = source;
            }
        }
    }

    return (! source || d->probee != nullptr);
}

bool QVideoProbe::setSource(QMediaRecorder *mediaRecorder)
{
    QMediaObject *source = mediaRecorder ? mediaRecorder->mediaObject() : nullptr;
    bool result = setSource(source);

    if (! mediaRecorder) {
       return true;
    }

    if (mediaRecorder && ! source) {
       return false;
    }

    return result;
}

bool QVideoProbe::isActive() const
{
    return d->probee != nullptr;
}
