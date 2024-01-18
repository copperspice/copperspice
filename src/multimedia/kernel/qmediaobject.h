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

#ifndef QMEDIAOBJECT_H
#define QMEDIAOBJECT_H

#include <qobject.h>
#include <qstringlist.h>
#include <qmultimedia.h>

#include <functional>

class QMediaService;
class QMediaBindableInterface;
class QMediaObjectPrivate;

class Q_MULTIMEDIA_EXPORT QMediaObject : public QObject
{
   MULTI_CS_OBJECT(QMediaObject)

   MULTI_CS_PROPERTY_READ(notifyInterval,   notifyInterval)
   MULTI_CS_PROPERTY_WRITE(notifyInterval,  setNotifyInterval)
   MULTI_CS_PROPERTY_NOTIFY(notifyInterval, notifyIntervalChanged)

 public:
   ~QMediaObject();

   virtual bool isAvailable() const;
   virtual QMultimedia::AvailabilityStatus availability() const;

   virtual QMediaService *service() const;

   int notifyInterval() const;
   void setNotifyInterval(int milliSeconds);

   virtual bool bind(QObject *object);
   virtual void unbind(QObject *object);

   bool isMetaDataAvailable() const;

   QVariant metaData(const QString &key) const;
   QStringList availableMetaData() const;

   MULTI_CS_SIGNAL_1(Public, void notifyIntervalChanged(int milliSeconds))
   MULTI_CS_SIGNAL_2(notifyIntervalChanged, milliSeconds)

   MULTI_CS_SIGNAL_1(Public, void metaDataAvailableChanged(bool available))
   MULTI_CS_SIGNAL_2(metaDataAvailableChanged, available)

   MULTI_CS_SIGNAL_1(Public, void metaDataChanged())
   MULTI_CS_SIGNAL_OVERLOAD(metaDataChanged, ())

   MULTI_CS_SIGNAL_1(Public, void metaDataChanged(const QString &key, const QVariant &value))
   MULTI_CS_SIGNAL_OVERLOAD(metaDataChanged, (const QString &, const QVariant &), key, value )

   MULTI_CS_SIGNAL_1(Public, void availabilityChanged(bool available))
   MULTI_CS_SIGNAL_OVERLOAD(availabilityChanged, (bool), available)

   MULTI_CS_SIGNAL_1(Public, void availabilityChanged(QMultimedia::AvailabilityStatus availability))
   MULTI_CS_SIGNAL_OVERLOAD(availabilityChanged, (QMultimedia::AvailabilityStatus), availability)

 protected:
   QMediaObject(QObject *parent, QMediaService *service);
   QMediaObject(QMediaObjectPrivate &dd, QObject *parent, QMediaService *service);

   template <typename T>
   void addPropertyWatch(const QString &name);

   void removePropertyWatch(const QString &name);

   QMediaObjectPrivate *d_ptr;

 private:
   void setupControls();

   Q_DECLARE_PRIVATE(QMediaObject)

   void cs_internal_addPropertyWatch(const QString &name, std::function<void ()> callBack);

   MULTI_CS_SLOT_1(Private, void _q_notify())
   MULTI_CS_SLOT_2(_q_notify)

   MULTI_CS_SLOT_1(Private, void _q_availabilityChanged())
   MULTI_CS_SLOT_2(_q_availabilityChanged)
};

template <typename T>
void QMediaObject::addPropertyWatch(const QString &name)
{
   const QMetaObject *metaObj = metaObject();
   int propIndex = metaObj->indexOfProperty(name);

   if (propIndex!= -1 && metaObj->property(propIndex).hasNotifySignal()) {

      QMetaProperty metaProp   = metaObj->property(propIndex);
      QMetaMethod signalMethod = metaProp.notifySignal();

      auto callBack = [this, metaProp, signalMethod]()
         {
            QVariant data = metaProp.read(this);
            signalMethod.invoke(this, data.value<T>());
        };

      cs_internal_addPropertyWatch(name, callBack);
   }
}

#endif