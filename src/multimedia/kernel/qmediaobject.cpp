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

#include <qmediaobject_p.h>

#include <qmetaobject.h>
#include <qdebug.h>
#include <qmediaservice.h>
#include <qmetadatareadercontrol.h>
#include <qmediabindableinterface.h>
#include <qmediaavailabilitycontrol.h>

void QMediaObjectPrivate::_q_notify()
{
   for (const auto &callBack : notifyProperties) {
      callBack();
   }
}

void QMediaObjectPrivate::_q_availabilityChanged()
{
   Q_Q(QMediaObject);

   // Really this should not always emit, but we are unable to tell from here (isAvailable
   // may not have changed, or the mediaobject's overridden availability() may not have changed)

   q->availabilityChanged(q->availability());
   q->availabilityChanged(q->isAvailable());
}

QMediaObject::~QMediaObject()
{
   delete d_ptr;
}

QMultimedia::AvailabilityStatus QMediaObject::availability() const
{
   if (d_func()->service == nullptr) {
      return QMultimedia::ServiceMissing;
   }

   if (d_func()->availabilityControl) {
      return d_func()->availabilityControl->availability();
   }

   return QMultimedia::Available;
}

bool QMediaObject::isAvailable() const
{
   return availability() == QMultimedia::Available;
}

QMediaService *QMediaObject::service() const
{
   return d_func()->service;
}

int QMediaObject::notifyInterval() const
{
   return d_func()->notifyTimer->interval();
}

void QMediaObject::setNotifyInterval(int milliSeconds)
{
   Q_D(QMediaObject);

   if (d->notifyTimer->interval() != milliSeconds) {
      d->notifyTimer->setInterval(milliSeconds);

      emit notifyIntervalChanged(milliSeconds);
   }
}

bool QMediaObject::bind(QObject *object)
{
   QMediaBindableInterface *helper = dynamic_cast<QMediaBindableInterface *>(object);
   if (! helper) {
      return false;
   }

   QMediaObject *currentObject = helper->mediaObject();

   if (currentObject == this) {
      return true;
   }

   if (currentObject) {
      currentObject->unbind(object);
   }

   return helper->setMediaObject(this);
}

void QMediaObject::unbind(QObject *object)
{
   QMediaBindableInterface *helper = dynamic_cast<QMediaBindableInterface *>(object);

   if (helper && helper->mediaObject() == this) {
      helper->setMediaObject(nullptr);
   } else {
      qWarning() << "QMediaObject:unbind(): Trying to unbind a helper object which was never bound";
   }
}

QMediaObject::QMediaObject(QObject *parent, QMediaService *service)
   : QObject(parent), d_ptr(new QMediaObjectPrivate)
{
   Q_D(QMediaObject);

   d->q_ptr = this;

   d->notifyTimer = new QTimer(this);
   d->notifyTimer->setInterval(1000);

   connect(d->notifyTimer, &QTimer::timeout, this, &QMediaObject::_q_notify);

   d->service = service;

   setupControls();
}

QMediaObject::QMediaObject(QMediaObjectPrivate &dd, QObject *parent, QMediaService *service)
   : QObject(parent), d_ptr(&dd)
{
   Q_D(QMediaObject);
   d->q_ptr = this;

   d->notifyTimer = new QTimer(this);
   d->notifyTimer->setInterval(1000);

   connect(d->notifyTimer, &QTimer::timeout, this, &QMediaObject::_q_notify);

   d->service = service;

   setupControls();
}

void QMediaObject::cs_internal_addPropertyWatch(const QString &name, std::function<void ()> callBack)
{
   Q_D(QMediaObject);

   d->notifyProperties.insert(name, callBack);

   if (! d->notifyTimer->isActive()) {
      d->notifyTimer->start();
   }
}

void QMediaObject::removePropertyWatch(const QString &name)
{
   Q_D(QMediaObject);

   d->notifyProperties.remove(name);

   if (d->notifyProperties.isEmpty()) {
      d->notifyTimer->stop();
   }
}

bool QMediaObject::isMetaDataAvailable() const
{
   Q_D(const QMediaObject);

   return d->metaDataControl ? d->metaDataControl->isMetaDataAvailable() : false;
}

QVariant QMediaObject::metaData(const QString &key) const
{
   Q_D(const QMediaObject);

   return d->metaDataControl ? d->metaDataControl->metaData(key) : QVariant();
}

QStringList QMediaObject::availableMetaData() const
{
   Q_D(const QMediaObject);

   return d->metaDataControl ? d->metaDataControl->availableMetaData() : QStringList();
}

void QMediaObject::setupControls()
{
   Q_D(QMediaObject);

   if (d->service != nullptr) {
      d->metaDataControl = dynamic_cast<QMetaDataReaderControl *>(d->service->requestControl(QMetaDataReaderControl_iid));

      if (d->metaDataControl) {

         connect(d->metaDataControl, cs_mp_cast<>(&QMetaDataReaderControl::metaDataChanged),
               this, cs_mp_cast<>(&QMediaObject::metaDataChanged));

         connect(d->metaDataControl, cs_mp_cast<const QString &, const QVariant &>(&QMetaDataReaderControl::metaDataChanged),
               this, cs_mp_cast<const QString &, const QVariant &>(&QMediaObject::metaDataChanged));

         connect(d->metaDataControl, &QMetaDataReaderControl::metaDataAvailableChanged, this, &QMediaObject::metaDataAvailableChanged);
      }

      d->availabilityControl = d->service->requestControl<QMediaAvailabilityControl *>();

      if (d->availabilityControl) {
         connect(d->availabilityControl, &QMediaAvailabilityControl::availabilityChanged, this, &QMediaObject::_q_availabilityChanged);
      }
   }
}

void QMediaObject::_q_notify()
{
   Q_D(QMediaObject);
   d->_q_notify();
}

void QMediaObject::_q_availabilityChanged()
{
   Q_D(QMediaObject);
   d->_q_availabilityChanged();
}

