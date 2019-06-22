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

#include <qaudiodecoder.h>

#include <qcoreevent.h>
#include <qmetaobject.h>
#include <qtimer.h>
#include <qdebug.h>
#include <qpointer.h>

#include <qmediaservice.h>

#include <qmediaobject_p.h>
#include <qaudiodecodercontrol.h>
#include <qmediaserviceprovider_p.h>

static int qRegisterAudioDecoderMetaTypes()
{
   qRegisterMetaType<QAudioDecoder::State>("QAudioDecoder::State");
   qRegisterMetaType<QAudioDecoder::Error>("QAudioDecoder::Error");

   return 0;
}

Q_CONSTRUCTOR_FUNCTION(qRegisterAudioDecoderMetaTypes)

class QAudioDecoderPrivate : public QMediaObjectPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QAudioDecoder)

 public:
   QAudioDecoderPrivate()
      : provider(0), control(0), state(QAudioDecoder::StoppedState), error(QAudioDecoder::NoError)
   {}

   QMediaServiceProvider *provider;
   QAudioDecoderControl *control;
   QAudioDecoder::State state;
   QAudioDecoder::Error error;
   QString errorString;

   void _q_stateChanged(QAudioDecoder::State state);
   void _q_error(int error, const QString &errorString);
};

void QAudioDecoderPrivate::_q_stateChanged(QAudioDecoder::State ps)
{
   Q_Q(QAudioDecoder);

   if (ps != state) {
      state = ps;

      emit q->stateChanged(ps);
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
      d->control = qobject_cast<QAudioDecoderControl *>(d->service->requestControl(QAudioDecoderControl_iid));

      if (d->control != 0) {
         connect(d->control, SIGNAL(stateChanged(QAudioDecoder::State)), this, SLOT(_q_stateChanged(QAudioDecoder::State)));
         connect(d->control, &QAudioDecoderControl::error,               this, &QAudioDecoder::_q_error);

         connect(d->control, SIGNAL(formatChanged(QAudioFormat)),  this, SLOT(formatChanged(QAudioFormat)));
         connect(d->control, SIGNAL(sourceChanged()),              this, SLOT(sourceChanged()));
         connect(d->control, SIGNAL(bufferReady()),                this, SLOT(bufferReady()));
         connect(d->control, SIGNAL(bufferAvailableChanged(bool)), this, SLOT(bufferAvailableChanged(bool)));
         connect(d->control, SIGNAL(finished()),                   this, SLOT(finished()));
         connect(d->control, SIGNAL(positionChanged(qint64)),      this, SLOT(positionChanged(qint64)));
         connect(d->control, SIGNAL(durationChanged(qint64)),      this, SLOT(durationChanged(qint64)));
      }
   }
   if (!d->control) {
      d->error = ServiceMissingError;
      d->errorString = tr("The QAudioDecoder object does not have a valid service");
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
   return d_func()->state;
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

   if (d->control == 0) {
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

   if (d->control != 0) {
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

   if (d->control != 0) {
      d_func()->control->setSourceFilename(fileName);
   }
}

QIODevice *QAudioDecoder::sourceDevice() const
{
   Q_D(const QAudioDecoder);
   if (d->control) {
      return d->control->sourceDevice();
   }
   return 0;
}

void QAudioDecoder::setSourceDevice(QIODevice *device)
{
   Q_D(QAudioDecoder);

   if (d->control != 0) {
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

   if (d->control != 0) {
      d_func()->control->setAudioFormat(format);
   }
}

bool QAudioDecoder::bind(QObject *obj)
{
   return QMediaObject::bind(obj);
}

void QAudioDecoder::unbind(QObject *obj)
{
   QMediaObject::unbind(obj);
}

QMultimedia::SupportEstimate QAudioDecoder::hasSupport(const QString &mimeType,
   const QStringList &codecs)
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

void QAudioDecoder::_q_stateChanged(QAudioDecoder::State un_named_arg1)
{
   Q_D(QAudioDecoder);
   d->_q_stateChanged(un_named_arg1);
}

void QAudioDecoder::_q_error(int un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QAudioDecoder);
   d->_q_error(un_named_arg1, un_named_arg2);
}