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

#ifndef QMEDIA_RECORDER_H
#define QMEDIA_RECORDER_H

#include <qmediabindableinterface.h>
#include <qmediaencodersettings.h>
#include <qmediaobject.h>
#include <qmultimedia.h>
#include <qpair.h>
#include <qurl.h>

class QAudioEncoderSettings;
class QAudioFormat;
class QMediaRecorderPrivate;
class QMediaRecorderService;
class QSize;
class QVideoEncoderSettings;

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

   QMediaRecorder(const QMediaRecorder &) = delete;
   QMediaRecorder &operator=(const QMediaRecorder &) = delete;

   ~QMediaRecorder();

   QMediaObject *mediaObject() const override;

   bool isAvailable() const;
   QMultimedia::AvailabilityStatus availability() const;

   QUrl outputLocation() const;
   bool setOutputLocation(const QUrl &location);

   QUrl actualLocation() const;

   Error error() const;
   QString errorString() const;

   qint64 duration() const;

   bool isMuted() const;
   qreal volume() const;

   State state() const;
   Status status() const;

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
   void setContainerFormat(const QString &format);

   void setEncodingSettings(const QAudioEncoderSettings &audioSettings,
      const QVideoEncoderSettings &videoSettings = QVideoEncoderSettings(), const QString &mimeType = QString());

   bool isMetaDataAvailable() const;
   bool isMetaDataWritable() const;

   QVariant metaData(const QString &key) const;
   void setMetaData(const QString &key, const QVariant &value);
   QStringList availableMetaData() const;

   MULTI_CS_SLOT_1(Public, void record())
   MULTI_CS_SLOT_2(record)

   MULTI_CS_SLOT_1(Public, void pause())
   MULTI_CS_SLOT_2(pause)

   MULTI_CS_SLOT_1(Public, void stop())
   MULTI_CS_SLOT_2(stop)

   MULTI_CS_SLOT_1(Public, void setMuted(bool muted))
   MULTI_CS_SLOT_2(setMuted)

   MULTI_CS_SLOT_1(Public, void setVolume(qreal volume))
   MULTI_CS_SLOT_2(setVolume)

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QMediaRecorder::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void statusChanged(QMediaRecorder::Status status))
   MULTI_CS_SIGNAL_2(statusChanged, status)

   MULTI_CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   MULTI_CS_SIGNAL_2(durationChanged, duration)

   MULTI_CS_SIGNAL_1(Public, void mutedChanged(bool muted))
   MULTI_CS_SIGNAL_2(mutedChanged, muted)

   MULTI_CS_SIGNAL_1(Public, void volumeChanged(qreal volume))
   MULTI_CS_SIGNAL_2(volumeChanged, volume)

   MULTI_CS_SIGNAL_1(Public, void actualLocationChanged(const QUrl &location))
   MULTI_CS_SIGNAL_2(actualLocationChanged, location)

   MULTI_CS_SIGNAL_1(Public, void error(QMediaRecorder::Error error))
   MULTI_CS_SIGNAL_OVERLOAD(error, (QMediaRecorder::Error), error)

   MULTI_CS_SIGNAL_1(Public, void metaDataAvailableChanged(bool available))
   MULTI_CS_SIGNAL_2(metaDataAvailableChanged, available)

   MULTI_CS_SIGNAL_1(Public, void metaDataWritableChanged(bool writable))
   MULTI_CS_SIGNAL_2(metaDataWritableChanged, writable)

   MULTI_CS_SIGNAL_1(Public, void metaDataChanged())
   MULTI_CS_SIGNAL_OVERLOAD(metaDataChanged, ())

   MULTI_CS_SIGNAL_1(Public, void metaDataChanged(const QString &key, const QVariant &value))
   MULTI_CS_SIGNAL_OVERLOAD(metaDataChanged, (const QString &, const QVariant  &), key, value)

   MULTI_CS_SIGNAL_1(Public, void availabilityChanged(bool available))
   MULTI_CS_SIGNAL_OVERLOAD(availabilityChanged, (bool), available)

   MULTI_CS_SIGNAL_1(Public, void availabilityChanged(QMultimedia::AvailabilityStatus availability))
   MULTI_CS_SIGNAL_OVERLOAD(availabilityChanged, (QMultimedia::AvailabilityStatus), availability)

 protected:
   QMediaRecorder(QMediaRecorderPrivate &dd, QMediaObject *mediaObject, QObject *parent = nullptr);
   bool setMediaObject(QMediaObject *object) override;

   QMediaRecorderPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QMediaRecorder)

   // wrapper
   void cs_setOutputLocation(const QUrl &location) {
      setOutputLocation(location);
   }

   MULTI_CS_SLOT_1(Private, void _q_stateChanged(QMediaRecorder::State state))
   MULTI_CS_SLOT_2(_q_stateChanged)

   MULTI_CS_SLOT_1(Private, void _q_error(int error, const QString &errorString))
   MULTI_CS_SLOT_2(_q_error)

   MULTI_CS_SLOT_1(Private, void _q_serviceDestroyed())
   MULTI_CS_SLOT_2(_q_serviceDestroyed)

   MULTI_CS_SLOT_1(Private, void _q_notify())
   MULTI_CS_SLOT_2(_q_notify)

   MULTI_CS_SLOT_1(Private, void _q_updateActualLocation(const QUrl &url))
   MULTI_CS_SLOT_2(_q_updateActualLocation)

   MULTI_CS_SLOT_1(Private, void _q_updateNotifyInterval(int interval))
   MULTI_CS_SLOT_2(_q_updateNotifyInterval)

   MULTI_CS_SLOT_1(Private, void _q_applySettings())
   MULTI_CS_SLOT_2(_q_applySettings)

   MULTI_CS_SLOT_1(Private, void _q_availabilityChanged(QMultimedia::AvailabilityStatus availStatus))
   MULTI_CS_SLOT_2(_q_availabilityChanged)
};

#endif

