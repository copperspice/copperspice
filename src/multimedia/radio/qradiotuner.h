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

#ifndef QRADIOTUNER_H
#define QRADIOTUNER_H

#include <qmediaobject.h>
#include <qobject.h>
#include <qpair.h>
#include <qradiodata.h>

class QRadioTunerPrivate;

class Q_MULTIMEDIA_EXPORT QRadioTuner : public QMediaObject
{
   MULTI_CS_OBJECT(QRadioTuner)

   MULTI_CS_PROPERTY_READ(state, state)
   MULTI_CS_PROPERTY_NOTIFY(state, stateChanged)

   MULTI_CS_PROPERTY_READ(band, band)
   MULTI_CS_PROPERTY_WRITE(band, setBand)
   MULTI_CS_PROPERTY_NOTIFY(band, bandChanged)

   MULTI_CS_PROPERTY_READ(frequency, frequency)
   MULTI_CS_PROPERTY_WRITE(frequency, setFrequency)
   MULTI_CS_PROPERTY_NOTIFY(frequency, frequencyChanged)

   MULTI_CS_PROPERTY_READ(stereo, isStereo)
   MULTI_CS_PROPERTY_NOTIFY(stereo, stereoStatusChanged)

   MULTI_CS_PROPERTY_READ(stereoMode, stereoMode)
   MULTI_CS_PROPERTY_WRITE(stereoMode, setStereoMode)

   MULTI_CS_PROPERTY_READ(signalStrength, signalStrength)
   MULTI_CS_PROPERTY_NOTIFY(signalStrength, signalStrengthChanged)

   MULTI_CS_PROPERTY_READ(volume, volume)
   MULTI_CS_PROPERTY_WRITE(volume, setVolume)
   MULTI_CS_PROPERTY_NOTIFY(volume, volumeChanged)

   MULTI_CS_PROPERTY_READ(muted, isMuted)
   MULTI_CS_PROPERTY_WRITE(muted, setMuted)
   MULTI_CS_PROPERTY_NOTIFY(muted, mutedChanged)

   MULTI_CS_PROPERTY_READ(searching, isSearching)
   MULTI_CS_PROPERTY_NOTIFY(searching, searchingChanged)

   MULTI_CS_PROPERTY_READ(antennaConnected, isAntennaConnected)
   MULTI_CS_PROPERTY_NOTIFY(antennaConnected, antennaConnectedChanged)

   MULTI_CS_PROPERTY_READ(radioData, radioData)
   MULTI_CS_PROPERTY_CONSTANT(radioData)

   MULTI_CS_ENUM(State)
   MULTI_CS_ENUM(Band)
   MULTI_CS_ENUM(Error)
   MULTI_CS_ENUM(StereoMode)
   MULTI_CS_ENUM(SearchMode)

 public:
   enum State {
      ActiveState,
      StoppedState
   };

   enum Band {
      AM,
      FM,
      SW,
      LW,
      FM2
   };

   enum Error {
      NoError,
      ResourceError,
      OpenError,
      OutOfRangeError
   };

   enum StereoMode {
      ForceStereo,
      ForceMono,
      Auto
   };

   enum SearchMode {
      SearchFast,
      SearchGetStationId
   };

    explicit QRadioTuner(QObject *parent = nullptr);

    QRadioTuner(const QRadioTuner &) = delete;
    QRadioTuner &operator=(const QRadioTuner &) = delete;

    ~QRadioTuner();

    QMultimedia::AvailabilityStatus availability() const override;

    State state() const;

    Band band() const;

    bool isBandSupported(Band band) const;

    int frequency() const;
    int frequencyStep(Band band) const;
    QPair<int,int> frequencyRange(Band band) const;

    bool isStereo() const;
    void setStereoMode(QRadioTuner::StereoMode mode);
    StereoMode stereoMode() const;

    int signalStrength() const;

    int volume() const;
    bool isMuted() const;

    bool isSearching() const;

    bool isAntennaConnected() const;

    Error error() const;
    QString errorString() const;

    QRadioData *radioData() const;

    MULTI_CS_SLOT_1(Public, void searchForward())
    MULTI_CS_SLOT_2(searchForward)
    MULTI_CS_SLOT_1(Public, void searchBackward())
    MULTI_CS_SLOT_2(searchBackward)
    MULTI_CS_SLOT_1(Public, void searchAllStations(QRadioTuner::SearchMode searchMode = QRadioTuner::SearchFast))
    MULTI_CS_SLOT_2(searchAllStations)
    MULTI_CS_SLOT_1(Public, void cancelSearch())
    MULTI_CS_SLOT_2(cancelSearch)

    MULTI_CS_SLOT_1(Public, void setBand(Band band))
    MULTI_CS_SLOT_2(setBand)
    MULTI_CS_SLOT_1(Public, void setFrequency(int frequency))
    MULTI_CS_SLOT_2(setFrequency)

    MULTI_CS_SLOT_1(Public, void setVolume(int volume))
    MULTI_CS_SLOT_2(setVolume)
    MULTI_CS_SLOT_1(Public, void setMuted(bool muted))
    MULTI_CS_SLOT_2(setMuted)

    MULTI_CS_SLOT_1(Public, void start())
    MULTI_CS_SLOT_2(start)
    MULTI_CS_SLOT_1(Public, void stop())
    MULTI_CS_SLOT_2(stop)

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
    MULTI_CS_SIGNAL_1(Public, void stationFound(int frequency,QString stationId))
    MULTI_CS_SIGNAL_2(stationFound,frequency,stationId)

    MULTI_CS_SIGNAL_1(Public, void antennaConnectedChanged(bool connectionStatus))
    MULTI_CS_SIGNAL_2(antennaConnectedChanged,connectionStatus)

    MULTI_CS_SIGNAL_1(Public, void error(QRadioTuner::Error error))
    MULTI_CS_SIGNAL_OVERLOAD(error, (QRadioTuner::Error), error)

 private:
    Q_DECLARE_PRIVATE(QRadioTuner)
};

#endif