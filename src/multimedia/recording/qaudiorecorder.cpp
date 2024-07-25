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

#include <qaudiorecorder.h>

#include <qaudioformat.h>
#include <qaudioinputselectorcontrol.h>
#include <qdebug.h>
#include <qmediaservice.h>
#include <qmetaobject.h>
#include <qstringlist.h>
#include <qurl.h>

#include <qmediaobject_p.h>
#include <qmediarecorder_p.h>
#include <qmediaserviceprovider_p.h>

class QAudioRecorderObject : public QMediaObject
{
 public:
   QAudioRecorderObject(QObject *parent, QMediaService *service)
      : QMediaObject(parent, service) {
   }

   ~QAudioRecorderObject() {
   }
};

class QAudioRecorderPrivate : public QMediaRecorderPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QAudioRecorder)

 public:
   void initControls() {
      Q_Q(QAudioRecorder);

      audioInputSelector = nullptr;

      QMediaService *service = mediaObject ? mediaObject->service() : nullptr;

      if (service != nullptr) {
         audioInputSelector = dynamic_cast<QAudioInputSelectorControl *>(service->requestControl(QAudioInputSelectorControl_iid));
      }

      if (audioInputSelector) {
         q->connect(audioInputSelector, &QAudioInputSelectorControl::activeInputChanged, q,
                  &QAudioRecorder::audioInputChanged);

         q->connect(audioInputSelector, &QAudioInputSelectorControl::availableInputsChanged, q,
                  &QAudioRecorder::availableAudioInputsChanged);
      }
   }

   QAudioRecorderPrivate()
      : QMediaRecorderPrivate(), provider(nullptr), audioInputSelector(nullptr)
   {
   }

   QMediaServiceProvider *provider;
   QAudioInputSelectorControl *audioInputSelector;
};

QAudioRecorder::QAudioRecorder(QObject *parent):
   QMediaRecorder(*new QAudioRecorderPrivate, nullptr, parent)
{
   Q_D(QAudioRecorder);
   d->provider = QMediaServiceProvider::defaultServiceProvider();

   QMediaService *service = d->provider->requestService(Q_MEDIASERVICE_AUDIOSOURCE);
   setMediaObject(new QAudioRecorderObject(this, service));
   d->initControls();
}

QAudioRecorder::~QAudioRecorder()
{
   Q_D(QAudioRecorder);

   QMediaService *service = d->mediaObject ? d->mediaObject->service() : nullptr;
   QMediaObject *mediaObject = d->mediaObject;
   setMediaObject(nullptr);

   if (service && d->audioInputSelector) {
      service->releaseControl(d->audioInputSelector);
   }

   if (d->provider && service) {
      d->provider->releaseService(service);
   }

   delete mediaObject;
}

QStringList QAudioRecorder::audioInputs() const
{
   Q_D(const QAudioRecorder);

   if (d->audioInputSelector) {
      return d->audioInputSelector->availableInputs();
   } else {
      return QStringList();
   }
}

QString QAudioRecorder::audioInputDescription(const QString &name) const
{
   Q_D(const QAudioRecorder);

   if (d->audioInputSelector) {
      return d->audioInputSelector->inputDescription(name);
   } else {
      return QString();
   }
}

QString QAudioRecorder::defaultAudioInput() const
{
   Q_D(const QAudioRecorder);

   if (d->audioInputSelector) {
      return d->audioInputSelector->defaultInput();
   } else {
      return QString();
   }
}

QString QAudioRecorder::audioInput() const
{
   Q_D(const QAudioRecorder);

   if (d->audioInputSelector) {
      return d->audioInputSelector->activeInput();
   } else {
      return QString();
   }
}

void QAudioRecorder::setAudioInput(const QString &name)
{
   Q_D(const QAudioRecorder);

   if (d->audioInputSelector) {
      return d->audioInputSelector->setActiveInput(name);
   }
}

