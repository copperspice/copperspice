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

#ifndef QRADIODATA_H
#define QRADIODATA_H

#include <qmediabindableinterface.h>
#include <qmediaobject.h>
#include <qobject.h>

class QRadioDataPrivate;

class Q_MULTIMEDIA_EXPORT QRadioData : public QObject, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QRadioData, QObject)

   MULTI_CS_PROPERTY_READ(stationId, stationId)
   MULTI_CS_PROPERTY_NOTIFY(stationId, stationIdChanged)

   MULTI_CS_PROPERTY_READ(programType, programType)
   MULTI_CS_PROPERTY_NOTIFY(programType, programTypeChanged)

   MULTI_CS_PROPERTY_READ(programTypeName, programTypeName)
   MULTI_CS_PROPERTY_NOTIFY(programTypeName, programTypeNameChanged)

   MULTI_CS_PROPERTY_READ(stationName, stationName)
   MULTI_CS_PROPERTY_NOTIFY(stationName, stationNameChanged)

   MULTI_CS_PROPERTY_READ(radioText, radioText)
   MULTI_CS_PROPERTY_NOTIFY(radioText, radioTextChanged)

   MULTI_CS_PROPERTY_READ(alternativeFrequenciesEnabled, isAlternativeFrequenciesEnabled)
   MULTI_CS_PROPERTY_WRITE(alternativeFrequenciesEnabled, setAlternativeFrequenciesEnabled)
   MULTI_CS_PROPERTY_NOTIFY(alternativeFrequenciesEnabled, alternativeFrequenciesEnabledChanged)

   MULTI_CS_ENUM(Error)
   MULTI_CS_ENUM(ProgramType)

   CS_INTERFACES(QMediaBindableInterface)

public:
   enum Error {
      NoError,
      ResourceError,
      OpenError,
      OutOfRangeError
   };

   enum ProgramType {
      Undefined = 0,
      News,
      CurrentAffairs,
      Information,
      Sport,
      Education,
      Drama,
      Culture,
      Science,
      Varied,
      PopMusic,
      RockMusic,
      EasyListening,
      LightClassical,
      SeriousClassical,
      OtherMusic,
      Weather,
      Finance,
      ChildrensProgrammes,
      SocialAffairs,
      Religion,
      PhoneIn,
      Travel,
      Leisure,
      JazzMusic,
      CountryMusic,
      NationalMusic,
      OldiesMusic,
      FolkMusic,
      Documentary,
      AlarmTest,
      Alarm,
      Talk,
      ClassicRock,
      AdultHits,
      SoftRock,
      Top40,
      Soft,
      Nostalgia,
      Classical,
      RhythmAndBlues,
      SoftRhythmAndBlues,
      Language,
      ReligiousMusic,
      ReligiousTalk,
      Personality,
      Public,
      College
   };

   explicit QRadioData(QMediaObject *mediaObject, QObject *parent = nullptr);

   QRadioData(const QRadioData &) = delete;
   QRadioData &operator=(const QRadioData &) = delete;

   ~QRadioData();

   QMultimedia::AvailabilityStatus availability() const;

   QMediaObject *mediaObject() const override;

   QString stationId() const;
   ProgramType programType() const;
   QString programTypeName() const;
   QString stationName() const;
   QString radioText() const;
   bool isAlternativeFrequenciesEnabled() const;

   Error error() const;
   QString errorString() const;

   MULTI_CS_SLOT_1(Public, void setAlternativeFrequenciesEnabled(bool enabled))
   MULTI_CS_SLOT_2(setAlternativeFrequenciesEnabled)

   MULTI_CS_SIGNAL_1(Public, void stationIdChanged(QString stationId))
   MULTI_CS_SIGNAL_2(stationIdChanged,stationId)

   MULTI_CS_SIGNAL_1(Public, void programTypeChanged(QRadioData::ProgramType programType))
   MULTI_CS_SIGNAL_2(programTypeChanged,programType)

   MULTI_CS_SIGNAL_1(Public, void programTypeNameChanged(QString programTypeName))
   MULTI_CS_SIGNAL_2(programTypeNameChanged,programTypeName)

   MULTI_CS_SIGNAL_1(Public, void stationNameChanged(QString stationName))
   MULTI_CS_SIGNAL_2(stationNameChanged,stationName)

   MULTI_CS_SIGNAL_1(Public, void radioTextChanged(QString radioText))
   MULTI_CS_SIGNAL_2(radioTextChanged,radioText)

   MULTI_CS_SIGNAL_1(Public, void alternativeFrequenciesEnabledChanged(bool enabled))
   MULTI_CS_SIGNAL_2(alternativeFrequenciesEnabledChanged,enabled)

   MULTI_CS_SIGNAL_1(Public, void error(QRadioData::Error error))
   MULTI_CS_SIGNAL_OVERLOAD(error, (QRadioData::Error), error)

 protected:
   bool setMediaObject(QMediaObject *) override;

   QRadioDataPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QRadioData)

   MULTI_CS_SLOT_1(Private, void _q_serviceDestroyed())
   MULTI_CS_SLOT_2(_q_serviceDestroyed)
};

#endif

