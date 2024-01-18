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

#include <qaudiodecoder.h>

#include <qcoreevent.h>
#include <qmetaobject.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qpointer.h>
#include <qmediaservice.h>
#include <qaudiodecodercontrol.h>

#include <qmediaobject_p.h>
#include <qmediaserviceprovider_p.h>

class QAudioDecoderPrivate : public QMediaObjectPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QAudioDecoder)

 public:
   QAudioDecoderPrivate()
      : provider(nullptr), control(nullptr), m_state(QAudioDecoder::StoppedState), error(QAudioDecoder::NoError)
   {
   }

   QMediaServiceProvider *provider;
   QAudioDecoderControl *control;
   QAudioDecoder::State m_state;
   QAudioDecoder::Error error;
   QString errorString;

   void _q_stateChanged(QAudioDecoder::State state);
   void _q_error(int error, const QString &errorString);
};

void QAudioDecoderPrivate::_q_stateChanged(QAudioDecoder::State newState)
{
   Q_Q(QAudioDecoder);

   if (m_state != newState) {
      m_state = newState;

      emit q->stateChanged(newState);
   }
}

void QAudioDecoderPrivate::_q_error(int error, const QString &errorString)
{
   Q_Q(QAudioDecoder);

   this->error = QAudioDecoder::Error(error);
   this->errorString = errorString;

   emit q->error(this->error);
}

QAudioDecoder::QAudioDecoder(QObject *parent)
   : QMediaObject(*new QAudioDecoderPrivate, parent,
        QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_AUDIODECODER))
{
   Q_D(QAudioDecoder);

   d->provider = QMediaServiceProvider::defaultServiceProvider();

   if (d->service) {
      d->control = dynamic_cast<QAudioDecoderControl *>(d->service->requestControl(QAudioDecoderControl_Key));

      if (d->control != nullptr) {
         connect(d->control, &QAudioDecoderControl::stateChanged,           this, &QAudioDecoder::_q_stateChanged);
         connect(d->control, &QAudioDecoderControl::error,                  this, &QAudioDecoder::_q_error);

         connect(d->control, &QAudioDecoderControl::formatChanged,          this, &QAudioDecoder::formatChanged);
         connect(d->control, &QAudioDecoderControl::sourceChanged,          this, &QAudioDecoder::sourceChanged);
         connect(d->control, &QAudioDecoderControl::bufferReady,            this, &QAudioDecoder::bufferReady);
         connect(d->control, &QAudioDecoderControl::bufferAvailableChanged, this, &QAudioDecoder::bufferAvailableChanged);
         connect(d->control, &QAudioDecoderControl::finished,               this, &QAudioDecoder::finished);
         connect(d->control, &QAudioDecoderControl::positionChanged,        this, &QAudioDecoder::positionChanged);
         connect(d->control, &QAudioDecoderControl::durationChanged,        this, &QAudioDecoder::durationChanged);
      }
   }

   if (! d->control) {
      d->error = ServiceMissingError;
      d->errorString = tr("QAudioDecoder unable to find a valid service");
   }
}

QAudioDecoder::~QAudioDecoder()
{
   Q_D(QAudioDecoder);

   if (d->service) {
      if (d->control) {
         d->service->releaseControl(d->control);
      }

      d->provider->releaseService(d->service);
   }
}

QAudioDecoder::State QAudioDecoder::state() const
{
   return d_func()->m_state;
}

QAudioDecoder::Error QAudioDecoder::error() const
{
   return d_func()->error;
}

QString QAudioDecoder::errorString() const
{
   return d_func()->errorString;
}

void QAudioDecoder::start()
{
   Q_D(QAudioDecoder);

   if (d->control == nullptr) {
      QMetaObject::invokeMethod(this, "_q_error", Qt::QueuedConnection,
         Q_ARG(int, QAudioDecoder::ServiceMissingError),
         Q_ARG(const QString &, tr("QAudioDecoder object does not have a valid service")));
      return;
   }

   // Reset error conditions
   d->error = NoError;
   d->errorString.clear();

   d->control->start();
}

void QAudioDecoder::stop()
{
   Q_D(QAudioDecoder);

   if (d->control != nullptr) {
      d->control->stop();
   }
}

QString QAudioDecoder::sourceFilename() const
{
   Q_D(const QAudioDecoder);
   if (d->control) {
      return d->control->sourceFilename();
   }

   return QString();
}

void QAudioDecoder::setSourceFilename(const QString &fileName)
{
   Q_D(QAudioDecoder);

   if (d->control != nullptr) {
      d_func()->control->setSourceFilename(fileName);
   }
}

QIODevice *QAudioDecoder::sourceDevice() const
{
   Q_D(const QAudioDecoder);

   if (d->control) {
      return d->control->sourceDevice();
   }

   return nullptr;
}

void QAudioDecoder::setSourceDevice(QIODevice *device)
{
   Q_D(QAudioDecoder);

   if (d->control != nullptr) {
      d_func()->control->setSourceDevice(device);
   }
}

QAudioFormat QAudioDecoder::audioFormat() const
{
   Q_D(const QAudioDecoder);

   if (d->control) {
      return d->control->audioFormat();
   }

   return QAudioFormat();
}

void QAudioDecoder::setAudioFormat(const QAudioFormat &format)
{
   Q_D(QAudioDecoder);

   if (state() != QAudioDecoder::StoppedState) {
      return;
   }

   if (d->control != nullptr) {
      d_func()->control->setAudioFormat(format);
   }
}

bool QAudioDecoder::bind(QObject *object)
{
   return QMediaObject::bind(object);
}

void QAudioDecoder::unbind(QObject *object)
{
   QMediaObject::unbind(object);
}

QMultimedia::SupportEstimate QAudioDecoder::hasSupport(const QString &mimeType, const QStringList &codecs)
{
   return QMediaServiceProvider::defaultServiceProvider()->hasSupport(QByteArray
         (Q_MEDIASERVICE_AUDIODECODER), mimeType, codecs);
}

bool QAudioDecoder::bufferAvailable() const
{
   Q_D(const QAudioDecoder);

   if (d->control) {
      return d->control->bufferAvailable();
   }

   return false;
}

qint64 QAudioDecoder::position() const
{
   Q_D(const QAudioDecoder);

   if (d->control) {
      return d->control->position();
   }

   return -1;
}

qint64 QAudioDecoder::duration() const
{
   Q_D(const QAudioDecoder);

   if (d->control) {
      return d->control->duration();
   }

   return -1;
}

QAudioBuffer QAudioDecoder::read() const
{
   Q_D(const QAudioDecoder);

   if (d->control) {
      return d->control->read();
   } else {
      return QAudioBuffer();
   }
}

void QAudioDecoder::_q_stateChanged(QAudioDecoder::State state)
{
   Q_D(QAudioDecoder);
   d->_q_stateChanged(state);
}

void QAudioDecoder::_q_error(int error, const QString &errorString)
{
   Q_D(QAudioDecoder);
   d->_q_error(error, errorString);
}