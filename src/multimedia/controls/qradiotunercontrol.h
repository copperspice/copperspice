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

#ifndef QRADIOTUNERCONTROL_H
#define QRADIOTUNERCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qradiotuner.h>

class Q_MULTIMEDIA_EXPORT QRadioTunerControl : public QMediaControl
{
    MULTI_CS_OBJECT(QRadioTunerControl)

public:
    ~QRadioTunerControl();

    virtual QRadioTuner::State state() const = 0;

    virtual QRadioTuner::Band band() const = 0;
    virtual void setBand(QRadioTuner::Band band) = 0;
    virtual bool isBandSupported(QRadioTuner::Band band) const = 0;

    virtual int frequency() const = 0;
    virtual int frequencyStep(QRadioTuner::Band band) const = 0;
    virtual QPair<int,int> frequencyRange(QRadioTuner::Band band) const = 0;
    virtual void setFrequency(int frequency) = 0;

    virtual bool isStereo() const = 0;
    virtual QRadioTuner::StereoMode stereoMode() const = 0;
    virtual void setStereoMode(QRadioTuner::StereoMode mode) = 0;

    virtual int signalStrength() const = 0;

    virtual int volume() const = 0;
    virtual void setVolume(int volume) = 0;

    virtual bool isMuted() const = 0;
    virtual void setMuted(bool muted) = 0;

    virtual bool isSearching() const = 0;

    virtual bool isAntennaConnected() const { return true; }

    virtual void searchForward() = 0;
    virtual void searchBackward() = 0;
    virtual void searchAllStations(QRadioTuner::SearchMode searchMode = QRadioTuner::SearchFast) = 0;
    virtual void cancelSearch() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual QRadioTuner::Error error() const = 0;
    virtual QString errorString() const = 0;

    MULTI_CS_SIGNAL_1(Public, void stateChanged(QRadioTuner::State state))
    MULTI_CS_SIGNAL_2(stateChanged,state)
    MULTI_CS_SIGNAL_1(Public, void bandChanged(QRadioTuner::Band band))
    MULTI_CS_SIGNAL_2(bandChanged,band)
    MULTI_CS_SIGNAL_1(Public, void frequencyChanged(int frequency))
    MULTI_CS_SIGNAL_2(frequencyChanged,frequency)
    MULTI_CS_SIGNAL_1(Public, void stereoStatusChanged(bool stereo))
    MULTI_CS_SIGNAL_2(stereoStatusChanged,stereo)
    MULTI_CS_SIGNAL_1(Public, void searchingChanged(bool searching))
    MULTI_CS_SIGNAL_2(searchingChanged,searching)
    MULTI_CS_SIGNAL_1(Public, void signalStrengthChanged(int signalStrength))
    MULTI_CS_SIGNAL_2(signalStrengthChanged,signalStrength)
    MULTI_CS_SIGNAL_1(Public, void volumeChanged(int volume))
    MULTI_CS_SIGNAL_2(volumeChanged,volume)
    MULTI_CS_SIGNAL_1(Public, void mutedChanged(bool muted))
    MULTI_CS_SIGNAL_2(mutedChanged,muted)

    MULTI_CS_SIGNAL_1(Public, void error(QRadioTuner::Error error))
    MULTI_CS_SIGNAL_OVERLOAD(error, (QRadioTuner::Error), error)

    MULTI_CS_SIGNAL_1(Public, void stationFound(int frequency,QString stationId))
    MULTI_CS_SIGNAL_2(stationFound,frequency,stationId)

    MULTI_CS_SIGNAL_1(Public, void antennaConnectedChanged(bool connectionStatus))
    MULTI_CS_SIGNAL_2(antennaConnectedChanged,connectionStatus)

protected:
    explicit QRadioTunerControl(QObject *parent = nullptr);
};

#define QRadioTunerControl_iid "com.copperspice.CS.radioTunerControl/1.0"
CS_DECLARE_INTERFACE(QRadioTunerControl, QRadioTunerControl_iid)

#endif
