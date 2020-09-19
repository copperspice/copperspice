/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qpair.h>
#include <qmediaservice.h>
#include <qradiotunercontrol.h>
#include <qradiodata.h>

#include <qmediaobject_p.h>
#include <qmediaserviceprovider_p.h>

class QRadioTunerPrivate : public QMediaObjectPrivate
{
public:
    QRadioTunerPrivate():provider(0), control(0), radioData(0)
    {}

    QMediaServiceProvider *provider;
    QRadioTunerControl* control;
    QRadioData *radioData;
};

QRadioTuner::QRadioTuner(QObject *parent):
    QMediaObject(*new QRadioTunerPrivate, parent,
                  QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_RADIO))
{
    Q_D(QRadioTuner);

    d->provider = QMediaServiceProvider::defaultServiceProvider();

    if (d->service != 0) {
        d->control = qobject_cast<QRadioTunerControl*>(d->service->requestControl(QRadioTunerControl_iid));

        if (d->control != 0) {
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

/*!
    Destroys a radio tuner.
*/

QRadioTuner::~QRadioTuner()
{
    Q_D(QRadioTuner);

    if (d->radioData)
        delete d->radioData;

    if (d->service && d->control)
        d->service->releaseControl(d->control);

    d->provider->releaseService(d->service);
}

/*!
    Returns the availability of the radio tuner.
*/
QMultimedia::AvailabilityStatus QRadioTuner::availability() const
{
    if (d_func()->control == 0)
        return QMultimedia::ServiceMissing;

    if (!d_func()->control->isAntennaConnected())
        return QMultimedia::ResourceError;

    return QMediaObject::availability();
}

/*!
    \property QRadioTuner::state
    Return the current radio tuner state.

    \sa QRadioTuner::State
*/

QRadioTuner::State QRadioTuner::state() const
{
    return d_func()->control ?
            d_func()->control->state() : QRadioTuner::StoppedState;
}

/*!
    \property QRadioTuner::band
    \brief the frequency band a radio tuner is tuned to.

    \sa QRadioTuner::Band
*/

QRadioTuner::Band QRadioTuner::band() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->band();

    return QRadioTuner::FM;
}

/*!
    \property QRadioTuner::frequency
    \brief the frequency in Hertz a radio tuner is tuned to.
*/

int QRadioTuner::frequency() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->frequency();

    return 0;
}

/*!
    Returns the number of Hertz to increment the frequency by when stepping through frequencies
    within a given \a band.
*/

int QRadioTuner::frequencyStep(QRadioTuner::Band band) const
{
    Q_D(const QRadioTuner);

    if(d->control != 0)
        return d->control->frequencyStep(band);

    return 0;
}

/*!
    Returns a frequency \a band's minimum and maximum frequency.
*/

QPair<int,int> QRadioTuner::frequencyRange(QRadioTuner::Band band) const
{
    Q_D(const QRadioTuner);

    if(d->control != 0)
        return d->control->frequencyRange(band);

    return qMakePair<int,int>(0,0);
}

/*!
    \property QRadioTuner::stereo
    \brief whether a radio tuner is receiving a stereo signal.
*/

bool QRadioTuner::isStereo() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->isStereo();

    return false;
}


/*!
    \property QRadioTuner::stereoMode
    \brief the stereo mode of a radio tuner.
*/

QRadioTuner::StereoMode QRadioTuner::stereoMode() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->stereoMode();

    return QRadioTuner::Auto;
}

void QRadioTuner::setStereoMode(QRadioTuner::StereoMode mode)
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        return d->control->setStereoMode(mode);
}

/*!
    Identifies if a frequency \a band is supported by a radio tuner.

    Returns true if the band is supported, and false if it is not.
*/

bool QRadioTuner::isBandSupported(QRadioTuner::Band band) const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->isBandSupported(band);

    return false;
}

/*!
    Activate the radio device.
*/

void QRadioTuner::start()
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        d->control->start();
}

/*!
    Deactivate the radio device.
*/

void QRadioTuner::stop()
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        d->control->stop();
}

/*!
    \property QRadioTuner::signalStrength
    \brief the strength of the current radio signal as a percentage.
*/

int QRadioTuner::signalStrength() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->signalStrength();

    return 0;
}

/*!
    \property QRadioTuner::volume
    \brief the volume of a radio tuner's audio output as a percentage.
*/


int QRadioTuner::volume() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->volume();

    return 0;
}

/*!
    \property QRadioTuner::muted
    \brief whether a radio tuner's audio output is muted.
*/

bool QRadioTuner::isMuted() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->isMuted();

    return false;
}

/*!
    Sets a radio tuner's frequency \a band.

    Changing the band will reset the \l frequency to the new band's minimum frequency.
*/

void QRadioTuner::setBand(QRadioTuner::Band band)
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->setBand(band);
}

/*!
    Sets a radio tuner's \a frequency.

    If the tuner is set to a frequency outside the current \l band, the band will be changed to
    one occupied by the new frequency.
*/

void QRadioTuner::setFrequency(int frequency)
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->setFrequency(frequency);
}

void QRadioTuner::setVolume(int volume)
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->setVolume(volume);
}

void QRadioTuner::setMuted(bool muted)
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->setMuted(muted);
}

/*!
    \property QRadioTuner::searching
    \brief whether a radio tuner is currently scanning for a signal.

    \sa searchForward(), searchBackward(), cancelSearch()
*/

bool QRadioTuner::isSearching() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->isSearching();

    return false;
}

/*!
    \property QRadioTuner::antennaConnected
    \brief whether there is an antenna connected
*/
bool QRadioTuner::isAntennaConnected() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->isAntennaConnected();

    return false;
}

/*!
    Starts a forward scan for a signal, starting from the current \l frequency.

    \sa searchBackward(), cancelSearch(), searching
*/

void QRadioTuner::searchForward()
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->searchForward();
}

/*!
    Starts a backwards scan for a signal, starting from the current \l frequency.

    \sa searchForward(), cancelSearch(), searching
*/

void QRadioTuner::searchBackward()
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->searchBackward();
}

/*!
    \enum QRadioTuner::SearchMode

    Enumerates how the radio tuner should search for stations.

    \value SearchFast           Use only signal strength when searching.
    \value SearchGetStationId   After finding a strong signal, wait for the RDS station id (PI) before continuing.
*/

/*!
    Search all stations in current band

    Emits QRadioTuner::stationFound(int, QString) for every found station.
    After searching is completed, QRadioTuner::searchingChanged(bool) is
    emitted (false). If \a searchMode is set to SearchGetStationId, searching
    waits for station id (PI) on each frequency.

    \sa searchForward(), searchBackward(), searching
*/

void QRadioTuner::searchAllStations(QRadioTuner::SearchMode searchMode)
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        d->control->searchAllStations(searchMode);
}

/*!
    Stops scanning for a signal.

    \sa searchForward(), searchBackward(), searching
*/

void QRadioTuner::cancelSearch()
{
    Q_D(QRadioTuner);

    if (d->control != 0)
        d->control->cancelSearch();
}

QRadioTuner::Error QRadioTuner::error() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->error();

    return QRadioTuner::ResourceError;
}

QString QRadioTuner::errorString() const
{
    Q_D(const QRadioTuner);

    if (d->control != 0)
        return d->control->errorString();

    return QString();
}

QRadioData *QRadioTuner::radioData() const
{
    return d_func()->radioData;
}

