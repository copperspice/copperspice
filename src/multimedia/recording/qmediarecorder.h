/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QMEDIARECORDER_H
#define QMEDIARECORDER_H

#include <qpair.h>
#include <qurl.h>

#include <qmultimedia.h>
#include <qmediaobject.h>
#include <qmediaencodersettings.h>
#include <qmediabindableinterface.h>
#include <qmediaenumdebug.h>

class QSize;
class QAudioFormat;
class QMediaRecorderService;
class QAudioEncoderSettings;
class QVideoEncoderSettings;
class QMediaRecorderPrivate;

class Q_MULTIMEDIA_EXPORT QMediaRecorder : public QObject, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QMediaRecorder, QObject)

   CS_INTERFACES(QMediaBindableInterface)

   MULTI_CS_ENUM(State)
   MULTI_CS_ENUM(Status)
   MULTI_CS_ENUM(Error)

   MULTI_CS_PROPERTY_READ(state, state)
   MULTI_CS_PROPERTY_NOTIFY(state, stateChanged)

   MULTI_CS_PROPERTY_READ(status, status)
   MULTI_CS_PROPERTY_NOTIFY(status, statusChanged)

   MULTI_CS_PROPERTY_READ(duration, duration)
   MULTI_CS_PROPERTY_NOTIFY(duration, durationChanged)

   MULTI_CS_PROPERTY_READ(outputLocation, outputLocation)
   MULTI_CS_PROPERTY_WRITE(outputLocation, cs_setOutputLocation)

   MULTI_CS_PROPERTY_READ(actualLocation, actualLocation)
   MULTI_CS_PROPERTY_NOTIFY(actualLocation, actualLocationChanged)

   MULTI_CS_PROPERTY_READ(muted, isMuted)
   MULTI_CS_PROPERTY_WRITE(muted, setMuted)
   MULTI_CS_PROPERTY_NOTIFY(muted, mutedChanged)

   MULTI_CS_PROPERTY_READ(volume, volume)
   MULTI_CS_PROPERTY_WRITE(volume, setVolume)
   MULTI_CS_PROPERTY_NOTIFY(volume, volumeChanged)

   MULTI_CS_PROPERTY_READ(metaDataAvailable, isMetaDataAvailable)
   MULTI_CS_PROPERTY_NOTIFY(metaDataAvailable, metaDataAvailableChanged)

   MULTI_CS_PROPERTY_READ(metaDataWritable, isMetaDataWritable)
   MULTI_CS_PROPERTY_NOTIFY(metaDataWritable, metaDataWritableChanged)

 public:
   enum State {
      StoppedState,
      RecordingState,
      PausedState
   };

   enum Status {
      UnavailableStatus,
      UnloadedStatus,
      LoadingStatus,
      LoadedStatus,
      StartingStatus,
      RecordingStatus,
      PausedStatus,
      FinalizingStatus
   };

   enum Error {
      NoError,
      ResourceError,
      FormatError,
      OutOfSpaceError
   };

   explicit QMediaRecorder(QMediaObject *mediaObject, QObject *parent = nullptr);
   ~QMediaRecorder();

   QMediaObject *mediaObject() const;

   bool isAvailable() const;
   QMultimedia::AvailabilityStatus availability() const;

   QUrl outputLocation() const;
   bool setOutputLocation(const QUrl &location);


   QUrl actualLocation() const;

   State state() const;
   Status status() const;

   Error error() const;
   QString errorString() const;

   qint64 duration() const;

   bool isMuted() const;
   qreal volume() const;

   QStringList supportedContainers() const;
   QString containerDescription(const QString &format) const;

   QStringList supportedAudioCodecs() const;
   QString audioCodecDescription(const QString &codecName) const;

   QList<int> supportedAudioSampleRates(const QAudioEncoderSettings &settings = QAudioEncoderSettings(),
      bool *continuous = nullptr) const;

   QStringList supportedVideoCodecs() const;
   QString videoCodecDescription(const QString &codecName) const;

   QList<QSize> supportedResolutions(const QVideoEncoderSettings &settings = QVideoEncoderSettings(),
      bool *continuous = nullptr) const;

   QList<qreal> supportedFrameRates(const QVideoEncoderSettings &settings = QVideoEncoderSettings(),
      bool *continuous = nullptr) const;

   QAudioEncoderSettings audioSettings() const;
   QVideoEncoderSettings videoSettings() const;
   QString containerFormat() const;

   void setAudioSettings(const QAudioEncoderSettings &audioSettings);
   void setVideoSettings(const QVideoEncoderSettings &videoSettings);
   void setContainerFormat(const QString &container);

   void setEncodingSettings(const QAudioEncoderSettings &audioSettings,
      const QVideoEncoderSettings &videoSettings = QVideoEncoderSettings(),
      const QString &containerMimeType = QString());

   bool isMetaDataAvailable() const;
   bool isMetaDataWritable() const;

   QVariant metaData(const QString &key) const;
   void setMetaData(const QString &key, const QVariant &value);
   QStringList availableMetaData() const;

   CS_SLOT_1(Public, void record())
   CS_SLOT_2(record)
   CS_SLOT_1(Public, void pause())
   CS_SLOT_2(pause)
   CS_SLOT_1(Public, void stop())
   CS_SLOT_2(stop)
   CS_SLOT_1(Public, void setMuted(bool muted))
   CS_SLOT_2(setMuted)
   CS_SLOT_1(Public, void setVolume(qreal volume))
   CS_SLOT_2(setVolume)

   CS_SIGNAL_1(Public, void stateChanged(QMediaRecorder::State state))
   CS_SIGNAL_2(stateChanged, state)
   CS_SIGNAL_1(Public, void statusChanged(QMediaRecorder::Status status))
   CS_SIGNAL_2(statusChanged, status)
   CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   CS_SIGNAL_2(durationChanged, duration)
   CS_SIGNAL_1(Public, void mutedChanged(bool muted))
   CS_SIGNAL_2(mutedChanged, muted)
   CS_SIGNAL_1(Public, void volumeChanged(qreal volume))
   CS_SIGNAL_2(volumeChanged, volume)
   CS_SIGNAL_1(Public, void actualLocationChanged(const QUrl &location))
   CS_SIGNAL_2(actualLocationChanged, location)

   CS_SIGNAL_1(Public, void error(QMediaRecorder::Error error))
   CS_SIGNAL_OVERLOAD(error, (QMediaRecorder::Error), error)

   CS_SIGNAL_1(Public, void metaDataAvailableChanged(bool available))
   CS_SIGNAL_2(metaDataAvailableChanged, available)

   CS_SIGNAL_1(Public, void metaDataWritableChanged(bool writable))
   CS_SIGNAL_2(metaDataWritableChanged, writable)

   CS_SIGNAL_1(Public, void metaDataChanged())
   CS_SIGNAL_OVERLOAD(metaDataChanged, ())

   CS_SIGNAL_1(Public, void metaDataChanged(const QString &key, const QVariant &value))
   CS_SIGNAL_OVERLOAD(metaDataChanged, (const QString &, const QVariant  &), key, value)

   CS_SIGNAL_1(Public, void availabilityChanged(bool available))
   CS_SIGNAL_OVERLOAD(availabilityChanged, (bool), available)

   CS_SIGNAL_1(Public, void availabilityChanged(QMultimedia::AvailabilityStatus availability))
   CS_SIGNAL_OVERLOAD(availabilityChanged, (QMultimedia::AvailabilityStatus), availability)

 protected:
   QMediaRecorder(QMediaRecorderPrivate &dd, QMediaObject *mediaObject, QObject *parent = nullptr);
   bool setMediaObject(QMediaObject *object);

   QMediaRecorderPrivate *d_ptr;

 private:
   Q_DISABLE_COPY(QMediaRecorder)
   Q_DECLARE_PRIVATE(QMediaRecorder)

   // wrapper
   void cs_setOutputLocation(const QUrl &location) {
      setOutputLocation(location);
   }

   CS_SLOT_1(Private, void _q_stateChanged(QMediaRecorder::State un_named_arg1))
   CS_SLOT_2(_q_stateChanged)

   CS_SLOT_1(Private, void _q_error(int un_named_arg1, const QString &un_named_arg2))
   CS_SLOT_2(_q_error)

   CS_SLOT_1(Private, void _q_serviceDestroyed())
   CS_SLOT_2(_q_serviceDestroyed)

   CS_SLOT_1(Private, void _q_notify())
   CS_SLOT_2(_q_notify)

   CS_SLOT_1(Private, void _q_updateActualLocation(const QUrl &un_named_arg1))
   CS_SLOT_2(_q_updateActualLocation)

   CS_SLOT_1(Private, void _q_updateNotifyInterval(int un_named_arg1))
   CS_SLOT_2(_q_updateNotifyInterval)

   CS_SLOT_1(Private, void _q_applySettings())
   CS_SLOT_2(_q_applySettings)

   CS_SLOT_1(Private, void _q_availabilityChanged(QMultimedia::AvailabilityStatus un_named_arg1))
   CS_SLOT_2(_q_availabilityChanged)

};

Q_DECLARE_METATYPE(QMediaRecorder::State)
Q_DECLARE_METATYPE(QMediaRecorder::Status)
Q_DECLARE_METATYPE(QMediaRecorder::Error)

Q_MEDIA_ENUM_DEBUG(QMediaRecorder, State)
Q_MEDIA_ENUM_DEBUG(QMediaRecorder, Status)
Q_MEDIA_ENUM_DEBUG(QMediaRecorder, Error)

#endif

