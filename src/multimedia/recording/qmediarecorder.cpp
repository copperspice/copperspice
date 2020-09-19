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

#include <qmediarecorder.h>

// emerald  #include <qcamera.h>
// emerald  #include <qcameracontrol.h>
#include <qmediarecordercontrol.h>
#include <qmediaservice.h>
#include <qmediaserviceprovider_p.h>
#include <qmetadatawritercontrol.h>
#include <qaudioencodersettingscontrol.h>
#include <qvideoencodersettingscontrol.h>
#include <qmediacontainercontrol.h>
#include <qmediaavailabilitycontrol.h>
#include <qdebug.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qmetaobject.h>
#include <qaudioformat.h>

#include <qmediaobject_p.h>
#include <qmediarecorder_p.h>

#define ENUM_NAME(c,e,v) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(e)).valueToKey((v)))

QMediaRecorderPrivate::QMediaRecorderPrivate()
   : mediaObject(0), control(0), formatControl(0), audioControl(0),
     videoControl(0), metaDataControl(0), availabilityControl(0), settingsChanged(false),
     notifyTimer(0), state(QMediaRecorder::StoppedState), error(QMediaRecorder::NoError)
{
}

void QMediaRecorderPrivate::_q_stateChanged(QMediaRecorder::State ps)
{
   Q_Q(QMediaRecorder);

   if (ps == QMediaRecorder::RecordingState) {
      notifyTimer->start();
   } else {
      notifyTimer->stop();
   }

   //  qDebug() << "Recorder state changed:" << ENUM_NAME(QMediaRecorder,"State",ps);
   if (state != ps) {
      emit q->stateChanged(ps);
   }

   state = ps;
}

void QMediaRecorderPrivate::_q_error(int error, const QString &errorString)
{
   Q_Q(QMediaRecorder);

   this->error = QMediaRecorder::Error(error);
   this->errorString = errorString;

   emit q->error(this->error);
}

void QMediaRecorderPrivate::_q_serviceDestroyed()
{
   mediaObject     = 0;
   control         = 0;
   formatControl   = 0;
   audioControl    = 0;
   videoControl    = 0;
   metaDataControl = 0;
   availabilityControl = 0;
   settingsChanged = true;
}

void QMediaRecorderPrivate::_q_updateActualLocation(const QUrl &location)
{
   if (actualLocation != location) {
      actualLocation = location;
      emit q_func()->actualLocationChanged(actualLocation);
   }
}

void QMediaRecorderPrivate::_q_notify()
{
   emit q_func()->durationChanged(q_func()->duration());
}

void QMediaRecorderPrivate::_q_updateNotifyInterval(int ms)
{
   notifyTimer->setInterval(ms);
}

void QMediaRecorderPrivate::applySettingsLater()
{
   if (control && !settingsChanged) {
      settingsChanged = true;
      QMetaObject::invokeMethod(q_func(), "_q_applySettings", Qt::QueuedConnection);
   }
}

void QMediaRecorderPrivate::_q_applySettings()
{
   if (control && settingsChanged) {
      settingsChanged = false;
      control->applySettings();
   }
}

void QMediaRecorderPrivate::_q_availabilityChanged(QMultimedia::AvailabilityStatus availability)
{
   (void) availability;

   Q_Q(QMediaRecorder);

   // should not always emit, but we can not tell from here (isAvailable may not have changed,
   // or the mediaobject's overridden availability() may not have changed).

   q->availabilityChanged(q->availability());
   q->availabilityChanged(q->isAvailable());
}

void QMediaRecorderPrivate::restartCamera()
{

/*  emerald

   //restart camera if it can't apply new settings in the Active state
   QCamera *camera = qobject_cast<QCamera *>(mediaObject);

   if (camera && camera->captureMode() == QCamera::CaptureVideo) {
      QMetaObject::invokeMethod(camera,
         "_q_preparePropertyChange",
         Qt::DirectConnection,
         Q_ARG(int, QCameraControl::VideoEncodingSettings));
   }
*/

}

QMediaRecorder::QMediaRecorder(QMediaObject *mediaObject, QObject *parent)
   : QObject(parent), d_ptr(new QMediaRecorderPrivate)
{
   Q_D(QMediaRecorder);
   d->q_ptr = this;

   d->notifyTimer = new QTimer(this);
   connect(d->notifyTimer, &QTimer::timeout, this, &QMediaRecorder::_q_notify);

   setMediaObject(mediaObject);
}

/*!
    \internal
*/
QMediaRecorder::QMediaRecorder(QMediaRecorderPrivate &dd, QMediaObject *mediaObject, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   Q_D(QMediaRecorder);

   d->q_ptr = this;

   d->notifyTimer = new QTimer(this);
   connect(d->notifyTimer, &QTimer::timeout, this, &QMediaRecorder::_q_notify);

   setMediaObject(mediaObject);
}

QMediaRecorder::~QMediaRecorder()
{
   delete d_ptr;
}

QMediaObject *QMediaRecorder::mediaObject() const
{
   return d_func()->mediaObject;
}

/*!
    \internal
*/
bool QMediaRecorder::setMediaObject(QMediaObject *object)
{
   Q_D(QMediaRecorder);

   if (object == d->mediaObject) {
      return true;
   }

   if (d->mediaObject) {
      if (d->control) {
         disconnect(d->control, &QMediaRecorderControl::stateChanged,          this, &QMediaRecorder::_q_stateChanged);
         disconnect(d->control, &QMediaRecorderControl::statusChanged,         this, &QMediaRecorder::statusChanged);
         disconnect(d->control, &QMediaRecorderControl::mutedChanged,          this, &QMediaRecorder::mutedChanged);
         disconnect(d->control, &QMediaRecorderControl::volumeChanged,         this, &QMediaRecorder::volumeChanged);
         disconnect(d->control, &QMediaRecorderControl::durationChanged,       this, &QMediaRecorder::durationChanged);
         disconnect(d->control, &QMediaRecorderControl::actualLocationChanged, this, &QMediaRecorder::_q_updateActualLocation);
         disconnect(d->control, &QMediaRecorderControl::error,                 this, &QMediaRecorder::_q_error);
      }

      disconnect(d->mediaObject, &QMediaObject::notifyIntervalChanged, this, &QMediaRecorder::_q_updateNotifyInterval);

      QMediaService *service = d->mediaObject->service();

      if (service) {
         disconnect(service, &QMediaService::destroyed, this, &QMediaRecorder::_q_serviceDestroyed);

         if (d->control) {
            service->releaseControl(d->control);
         }

         if (d->formatControl) {
            service->releaseControl(d->formatControl);
         }

         if (d->audioControl) {
            service->releaseControl(d->audioControl);
         }

         if (d->videoControl) {
            service->releaseControl(d->videoControl);
         }

         if (d->metaDataControl) {

            disconnect(d->metaDataControl, static_cast<void (QMetaDataWriterControl::*)()>
                  (&QMetaDataWriterControl::metaDataChanged), this,
                  static_cast<void (QMediaRecorder::*)()>(&QMediaRecorder::metaDataChanged));

            disconnect(d->metaDataControl, static_cast<void (QMetaDataWriterControl::*)(const QString &, const QVariant &)>
                  (&QMetaDataWriterControl::metaDataChanged), this,
                  static_cast<void (QMediaRecorder::*)(const QString &, const QVariant &)>(&QMediaRecorder::metaDataChanged));

            disconnect(d->metaDataControl, &QMetaDataWriterControl::metaDataAvailableChanged, this, &QMediaRecorder::metaDataAvailableChanged);
            disconnect(d->metaDataControl, &QMetaDataWriterControl::writableChanged,          this, &QMediaRecorder::metaDataWritableChanged);

            service->releaseControl(d->metaDataControl);
         }
         if (d->availabilityControl) {
            disconnect(d->availabilityControl, &QMediaAvailabilityControl::availabilityChanged, this, &QMediaRecorder::_q_availabilityChanged);

            service->releaseControl(d->availabilityControl);
         }
      }
   }

   d->control = 0;
   d->formatControl = 0;
   d->audioControl = 0;
   d->videoControl = 0;
   d->metaDataControl = 0;
   d->availabilityControl = 0;

   d->mediaObject = object;

   if (d->mediaObject) {
      QMediaService *service = d->mediaObject->service();

      d->notifyTimer->setInterval(d->mediaObject->notifyInterval());
      connect(d->mediaObject, &QMediaObject::notifyIntervalChanged, this, &QMediaRecorder::_q_updateNotifyInterval);

      if (service) {
         d->control = qobject_cast<QMediaRecorderControl *>(service->requestControl(QMediaRecorderControl_iid));

         if (d->control) {
            d->formatControl = qobject_cast<QMediaContainerControl *>(service->requestControl(QMediaContainerControl_iid));
            d->audioControl  = qobject_cast<QAudioEncoderSettingsControl *>(service->requestControl(QAudioEncoderSettingsControl_iid));
            d->videoControl  = qobject_cast<QVideoEncoderSettingsControl *>(service->requestControl(QVideoEncoderSettingsControl_iid));

            QMediaControl *control = service->requestControl(QMetaDataWriterControl_iid);
            if (control) {
               d->metaDataControl = qobject_cast<QMetaDataWriterControl *>(control);

               if (!d->metaDataControl) {
                  service->releaseControl(control);
               } else {
                  connect(d->metaDataControl, static_cast<void (QMetaDataWriterControl::*)()>
                        (&QMetaDataWriterControl::metaDataChanged), this,
                        static_cast<void (QMediaRecorder::*)()>(&QMediaRecorder::metaDataChanged));

                  connect(d->metaDataControl, static_cast<void (QMetaDataWriterControl::*)(const QString &, const QVariant &)>
                        (&QMetaDataWriterControl::metaDataChanged), this,
                        static_cast<void (QMediaRecorder::*)(const QString &, const QVariant &)>(&QMediaRecorder::metaDataChanged));


                  connect(d->metaDataControl, &QMetaDataWriterControl::metaDataAvailableChanged, this, &QMediaRecorder::metaDataAvailableChanged);
                  connect(d->metaDataControl, &QMetaDataWriterControl::writableChanged,          this, &QMediaRecorder::metaDataWritableChanged);
               }
            }

            d->availabilityControl = service->requestControl<QMediaAvailabilityControl *>();

            if (d->availabilityControl) {
               connect(d->availabilityControl, &QMediaAvailabilityControl::availabilityChanged, this, &QMediaRecorder::_q_availabilityChanged);
            }

            connect(d->control, &QMediaRecorderControl::stateChanged,    this, &QMediaRecorder::_q_stateChanged);
            connect(d->control, &QMediaRecorderControl::statusChanged,   this, &QMediaRecorder::statusChanged);
            connect(d->control, &QMediaRecorderControl::mutedChanged,    this, &QMediaRecorder::mutedChanged);
            connect(d->control, &QMediaRecorderControl::volumeChanged,   this, &QMediaRecorder::volumeChanged);
            connect(d->control, &QMediaRecorderControl::durationChanged, this, &QMediaRecorder::durationChanged);

            connect(d->control, &QMediaRecorderControl::actualLocationChanged, this, &QMediaRecorder::_q_updateActualLocation);
            connect(d->control, &QMediaRecorderControl::error,                 this, &QMediaRecorder::_q_error);
            connect(service,    &QMediaService::destroyed,                     this, &QMediaRecorder::_q_serviceDestroyed);

            d->applySettingsLater();

            return true;
         }
      }

      d->mediaObject = 0;
      return false;
   }

   return true;
}

bool QMediaRecorder::isAvailable() const
{
   return availability() == QMultimedia::Available;
}

QMultimedia::AvailabilityStatus QMediaRecorder::availability() const
{
   if (d_func()->control == NULL) {
      return QMultimedia::ServiceMissing;
   }

   if (d_func()->availabilityControl) {
      return d_func()->availabilityControl->availability();
   }

   return QMultimedia::Available;
}

QUrl QMediaRecorder::outputLocation() const
{
   return d_func()->control ? d_func()->control->outputLocation() : QUrl();
}

bool QMediaRecorder::setOutputLocation(const QUrl &location)
{
   Q_D(QMediaRecorder);
   d->actualLocation.clear();
   return d->control ? d->control->setOutputLocation(location) : false;
}

QUrl QMediaRecorder::actualLocation() const
{
   return d_func()->actualLocation;
}

QMediaRecorder::State QMediaRecorder::state() const
{
   return d_func()->control ? QMediaRecorder::State(d_func()->control->state()) : StoppedState;
}

QMediaRecorder::Status QMediaRecorder::status() const
{
   return d_func()->control ? QMediaRecorder::Status(d_func()->control->status()) : UnavailableStatus;
}

QMediaRecorder::Error QMediaRecorder::error() const
{
   return d_func()->error;
}

QString QMediaRecorder::errorString() const
{
   return d_func()->errorString;
}

qint64 QMediaRecorder::duration() const
{
   return d_func()->control ? d_func()->control->duration() : 0;
}

bool QMediaRecorder::isMuted() const
{
   return d_func()->control ? d_func()->control->isMuted() : 0;
}

void QMediaRecorder::setMuted(bool muted)
{
   Q_D(QMediaRecorder);

   if (d->control) {
      d->control->setMuted(muted);
   }
}

qreal QMediaRecorder::volume() const
{
   return d_func()->control ? d_func()->control->volume() : 1.0;
}

void QMediaRecorder::setVolume(qreal volume)
{
   Q_D(QMediaRecorder);

   if (d->control) {
      volume = qMax(qreal(0.0), volume);
      d->control->setVolume(volume);
   }
}

QStringList QMediaRecorder::supportedContainers() const
{
   return d_func()->formatControl ?
      d_func()->formatControl->supportedContainers() : QStringList();
}

QString QMediaRecorder::containerDescription(const QString &format) const
{
   return d_func()->formatControl ?
      d_func()->formatControl->containerDescription(format) : QString();
}

QString QMediaRecorder::containerFormat() const
{
   return d_func()->formatControl ?
      d_func()->formatControl->containerFormat() : QString();
}

QStringList QMediaRecorder::supportedAudioCodecs() const
{
   return d_func()->audioControl ?
      d_func()->audioControl->supportedAudioCodecs() : QStringList();
}

QString QMediaRecorder::audioCodecDescription(const QString &codec) const
{
   return d_func()->audioControl ?
      d_func()->audioControl->codecDescription(codec) : QString();
}

QList<int> QMediaRecorder::supportedAudioSampleRates(const QAudioEncoderSettings &settings, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   return d_func()->audioControl ?
      d_func()->audioControl->supportedSampleRates(settings, continuous) : QList<int>();
}

QList<QSize> QMediaRecorder::supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   return d_func()->videoControl ?
      d_func()->videoControl->supportedResolutions(settings, continuous) : QList<QSize>();
}

QList<qreal> QMediaRecorder::supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   return d_func()->videoControl ?
      d_func()->videoControl->supportedFrameRates(settings, continuous) : QList<qreal>();
}

QStringList QMediaRecorder::supportedVideoCodecs() const
{
   return d_func()->videoControl ?
      d_func()->videoControl->supportedVideoCodecs() : QStringList();
}

QString QMediaRecorder::videoCodecDescription(const QString &codec) const
{
   return d_func()->videoControl ?
      d_func()->videoControl->videoCodecDescription(codec) : QString();
}

QAudioEncoderSettings QMediaRecorder::audioSettings() const
{
   return d_func()->audioControl ?
      d_func()->audioControl->audioSettings() : QAudioEncoderSettings();
}

QVideoEncoderSettings QMediaRecorder::videoSettings() const
{
   return d_func()->videoControl ?
      d_func()->videoControl->videoSettings() : QVideoEncoderSettings();
}

void QMediaRecorder::setAudioSettings(const QAudioEncoderSettings &settings)
{
   (void) settings;

/*  emerald
   Q_D(QMediaRecorder);

   //restart camera if it can't apply new settings in the Active state
   d->restartCamera();

   if (d->audioControl) {
      d->audioControl->setAudioSettings(settings);
      d->applySettingsLater();
   }
*/

}

void QMediaRecorder::setVideoSettings(const QVideoEncoderSettings &settings)
{
   (void) settings;

/*  emerald
   Q_D(QMediaRecorder);
   d->restartCamera();

   if (d->videoControl) {
      d->videoControl->setVideoSettings(settings);
      d->applySettingsLater();
   }
*/

}

void QMediaRecorder::setContainerFormat(const QString &container)
{
   (void) container;

/*  emerald
   Q_D(QMediaRecorder);
   d->restartCamera();

   if (d->formatControl) {
      d->formatControl->setContainerFormat(container);
      d->applySettingsLater();
   }
*/

}

void QMediaRecorder::setEncodingSettings(const QAudioEncoderSettings &audio,
         const QVideoEncoderSettings &video, const QString &container)
{
   (void) audio;
   (void) video;
   (void) container;

/*  emerald

   Q_D(QMediaRecorder);
   d->restartCamera();

   if (d->audioControl) {
      d->audioControl->setAudioSettings(audio);
   }

   if (d->videoControl) {
      d->videoControl->setVideoSettings(video);
   }

   if (d->formatControl) {
      d->formatControl->setContainerFormat(container);
   }

   d->applySettingsLater();
*/


}

void QMediaRecorder::record()
{
   Q_D(QMediaRecorder);

   d->actualLocation.clear();

   if (d->settingsChanged) {
      d->_q_applySettings();
   }

   // reset error
   d->error = NoError;
   d->errorString = QString();

   if (d->control) {
      d->control->setState(RecordingState);
   }
}

void QMediaRecorder::pause()
{
   Q_D(QMediaRecorder);
   if (d->control) {
      d->control->setState(PausedState);
   }
}

void QMediaRecorder::stop()
{
   Q_D(QMediaRecorder);
   if (d->control) {
      d->control->setState(StoppedState);
   }
}

bool QMediaRecorder::isMetaDataAvailable() const
{
   Q_D(const QMediaRecorder);

   return d->metaDataControl
      ? d->metaDataControl->isMetaDataAvailable()
      : false;
}

bool QMediaRecorder::isMetaDataWritable() const
{
   Q_D(const QMediaRecorder);

   return d->metaDataControl
      ? d->metaDataControl->isWritable()
      : false;
}

QVariant QMediaRecorder::metaData(const QString &key) const
{
   Q_D(const QMediaRecorder);

   return d->metaDataControl
      ? d->metaDataControl->metaData(key)
      : QVariant();
}

void QMediaRecorder::setMetaData(const QString &key, const QVariant &value)
{
   Q_D(QMediaRecorder);

   if (d->metaDataControl) {
      d->metaDataControl->setMetaData(key, value);
   }
}

QStringList QMediaRecorder::availableMetaData() const
{
   Q_D(const QMediaRecorder);

   return d->metaDataControl
      ? d->metaDataControl->availableMetaData()
      : QStringList();
}

void QMediaRecorder::_q_stateChanged(QMediaRecorder::State un_named_arg1)
{
   Q_D(QMediaRecorder);
   d->_q_stateChanged(un_named_arg1);
}

void QMediaRecorder::_q_error(int un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QMediaRecorder);
   d->_q_error(un_named_arg1, un_named_arg2);
}

void QMediaRecorder::_q_serviceDestroyed()
{
   Q_D(QMediaRecorder);
   d->_q_serviceDestroyed();
}

void QMediaRecorder::_q_notify()
{
   Q_D(QMediaRecorder);
   d->_q_notify();
}

void QMediaRecorder::_q_updateActualLocation(const QUrl &un_named_arg1)
{
   Q_D(QMediaRecorder);
   d->_q_updateActualLocation(un_named_arg1);
}

void QMediaRecorder::_q_updateNotifyInterval(int un_named_arg1)
{
   Q_D(QMediaRecorder);
   d->_q_updateNotifyInterval(un_named_arg1);
}

void QMediaRecorder::_q_applySettings()
{
   Q_D(QMediaRecorder);
   d->_q_applySettings();
}

void QMediaRecorder::_q_availabilityChanged(QMultimedia::AvailabilityStatus un_named_arg1)
{
   Q_D(QMediaRecorder);
   d->_q_availabilityChanged(un_named_arg1);
}
