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

#include <qradiotuner.h>

#include <qmediaservice.h>
#include <qpair.h>
#include <qradiodata.h>
#include <qradiotunercontrol.h>

#include <qmediaobject_p.h>
#include <qmediaserviceprovider_p.h>

class QRadioTunerPrivate : public QMediaObjectPrivate
{
 public:
    QRadioTunerPrivate()
      : provider(nullptr), control(nullptr), radioData(nullptr)
    {
    }

    QMediaServiceProvider *provider;
    QRadioTunerControl* control;
    QRadioData *radioData;
};

QRadioTuner::QRadioTuner(QObject *parent)
   : QMediaObject(*new QRadioTunerPrivate, parent,
     QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_RADIO))
{
    Q_D(QRadioTuner);

    d->provider = QMediaServiceProvider::defaultServiceProvider();

    if (d->service != nullptr) {
        d->control = dynamic_cast<QRadioTunerControl *>(d->service->requestControl(QRadioTunerControl_iid));

        if (d->control != nullptr) {
            connect(d->control, &QRadioTunerControl::stateChanged,            this, &QRadioTuner::stateChanged);
            connect(d->control, &QRadioTunerControl::bandChanged,             this, &QRadioTuner::bandChanged);
            connect(d->control, &QRadioTunerControl::frequencyChanged,        this, &QRadioTuner::frequencyChanged);
            connect(d->control, &QRadioTunerControl::stereoStatusChanged,     this, &QRadioTuner::stereoStatusChanged);
            connect(d->control, &QRadioTunerControl::searchingChanged,        this, &QRadioTuner::searchingChanged);
            connect(d->control, &QRadioTunerControl::signalStrengthChanged,   this, &QRadioTuner::signalStrengthChanged);
            connect(d->control, &QRadioTunerControl::volumeChanged,           this, &QRadioTuner::volumeChanged);
            connect(d->control, &QRadioTunerControl::mutedChanged,            this, &QRadioTuner::mutedChanged);
            connect(d->control, &QRadioTunerControl::stationFound,            this, &QRadioTuner::stationFound);
            connect(d->control, &QRadioTunerControl::antennaConnectedChanged, this, &QRadioTuner::antennaConnectedChanged);
            connect(d->control, &QRadioTunerControl::error,                   this, &QRadioTuner::error);
        }

        d->radioData = new QRadioData(this, this);
    }
}

QRadioTuner::~QRadioTuner()
{
    Q_D(QRadioTuner);

    if (d->radioData)
        delete d->radioData;

    if (d->service && d->control)
        d->service->releaseControl(d->control);

    d->provider->releaseService(d->service);
}

QMultimedia::AvailabilityStatus QRadioTuner::availability() const
{
    if (d_func()->control == nullptr)
        return QMultimedia::ServiceMissing;

    if (! d_func()->control->isAntennaConnected())
        return QMultimedia::ResourceError;

    return QMediaObject::availability();
}

QRadioTuner::State QRadioTuner::state() const
{
    return d_func()->control ? d_func()->control->state() : QRadioTuner::StoppedState;
}

QRadioTuner::Band QRadioTuner::band() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->band();
    }

    return QRadioTuner::FM;
}

int QRadioTuner::frequency() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->frequency();
    }

    return 0;
}

int QRadioTuner::frequencyStep(QRadioTuner::Band band) const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->frequencyStep(band);
    }

    return 0;
}

QPair<int,int> QRadioTuner::frequencyRange(QRadioTuner::Band band) const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->frequencyRange(band);
    }

    return qMakePair<int,int>(0,0);
}

bool QRadioTuner::isStereo() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->isStereo();
    }

    return false;
}

QRadioTuner::StereoMode QRadioTuner::stereoMode() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->stereoMode();
    }

    return QRadioTuner::Auto;
}

void QRadioTuner::setStereoMode(QRadioTuner::StereoMode mode)
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        return d->control->setStereoMode(mode);
    }
}

bool QRadioTuner::isBandSupported(QRadioTuner::Band band) const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->isBandSupported(band);
    }

    return false;
}

void QRadioTuner::start()
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        d->control->start();
    }
}

void QRadioTuner::stop()
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        d->control->stop();
    }
}

int QRadioTuner::signalStrength() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->signalStrength();
    }

    return 0;
}

int QRadioTuner::volume() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->volume();
    }

    return 0;
}

bool QRadioTuner::isMuted() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->isMuted();
    }

    return false;
}

void QRadioTuner::setBand(QRadioTuner::Band band)
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->setBand(band);
    }
}

void QRadioTuner::setFrequency(int frequency)
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->setFrequency(frequency);
    }
}

void QRadioTuner::setVolume(int volume)
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->setVolume(volume);
    }
}

void QRadioTuner::setMuted(bool muted)
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->setMuted(muted);
    }
}

bool QRadioTuner::isSearching() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->isSearching();
    }

    return false;
}

bool QRadioTuner::isAntennaConnected() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->isAntennaConnected();
    }

    return false;
}

void QRadioTuner::searchForward()
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->searchForward();
    }
}

void QRadioTuner::searchBackward()
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->searchBackward();
    }
}

void QRadioTuner::searchAllStations(QRadioTuner::SearchMode searchMode)
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        d->control->searchAllStations(searchMode);
    }
}

void QRadioTuner::cancelSearch()
{
    Q_D(QRadioTuner);

    if (d->control != nullptr) {
        d->control->cancelSearch();
    }
}

QRadioTuner::Error QRadioTuner::error() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->error();
    }

    return QRadioTuner::ResourceError;
}

QString QRadioTuner::errorString() const
{
    Q_D(const QRadioTuner);

    if (d->control != nullptr) {
        return d->control->errorString();
    }

    return QString();
}

QRadioData *QRadioTuner::radioData() const
{
    return d_func()->radioData;
}
