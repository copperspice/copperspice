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

#include <qdebug.h>

#include <qaudiosystem.h>
#include <qaudiosystemplugin.h>

#include <qmediapluginloader_p.h>
#include <qaudiodevicefactory_p.h>
#include <qfactoryloader_p.h>

static QString defaultKey()
{
   return QString("default");
}

#if ! defined(QT_NO_SETTINGS)
Q_GLOBAL_STATIC_WITH_ARGS(QMediaPluginLoader, audioLoader,
        (QAudioSystemFactoryInterface_iid, "audio", Qt::CaseInsensitive))
#endif

class QNullDeviceInfo : public QAbstractAudioDeviceInfo
{
 public:
   QAudioFormat preferredFormat() const override {
      qWarning() << "using null deviceinfo, none available";
      return QAudioFormat();
   }

   bool isFormatSupported(const QAudioFormat &) const override {
      return false;
   }

   QString deviceName() const override {
      return QString();
   }

   QStringList supportedCodecs() override {
      return QStringList();
   }

   QList<int> supportedSampleRates() override {
      return QList<int>();
   }

   QList<int> supportedChannelCounts() override {
      return QList<int>();
   }

   QList<int> supportedSampleSizes() override {
      return QList<int>();
   }

   QList<QAudioFormat::Endian> supportedByteOrders() override {
      return QList<QAudioFormat::Endian>();
   }

   QList<QAudioFormat::SampleType> supportedSampleTypes()  override{
      return QList<QAudioFormat::SampleType>();
   }
};

class QNullInputDevice : public QAbstractAudioInput
{
 public:
   void start(QIODevice*) override {
      qWarning() << "using null input device, none available";
   }

   QIODevice *start() override {
      qWarning()<<"using null input device, none available";
      return nullptr;
   }

   void stop() override {}
   void reset() override {}
   void suspend() override {}
   void resume() override {}

   int bytesReady() const override {
      return 0;
   }

   int periodSize() const override {
      return 0;
   }

   void setBufferSize(int ) override {}
   int bufferSize() const  override {
      return 0;
   }

   void setNotifyInterval(int ) override {}
   int notifyInterval() const override {
      return 0;
   }

   qint64 processedUSecs() const override {
      return 0;
   }

   qint64 elapsedUSecs() const override {
      return 0;
   }

   QAudio::Error error() const override {
      return QAudio::OpenError;
   }

   QAudio::State state() const override {
      return QAudio::StoppedState;
   }

   void setFormat(const QAudioFormat&) override {
   }

   QAudioFormat format() const override {
      return QAudioFormat();
   }

   void setVolume(qreal) override {
   }

   qreal volume() const override {
      return 1.0f;
   }
};

class QNullOutputDevice : public QAbstractAudioOutput
{
 public:
   void start(QIODevice *)  override {
      qWarning() << "using null output device, none available";
   }

   QIODevice *start() override {
      qWarning() << "using null output device, none available";
      return nullptr;
   }

   void stop() override {}
   void reset() override {}
   void suspend() override {}
   void resume() override {}

   int bytesFree() const override {
      return 0;
   }

   int periodSize() const override {
      return 0;
   }

   void setBufferSize(int ) override {}
   int bufferSize() const   override {
      return 0;
   }

   void setNotifyInterval(int ) override {}
   int notifyInterval() const override {
      return 0;
   }

   qint64 processedUSecs() const override {
      return 0;
   }

   qint64 elapsedUSecs() const override {
      return 0;
   }

   QAudio::Error error() const override {
      return QAudio::OpenError;
   }

   QAudio::State state() const override {
      return QAudio::StoppedState;
   }

   void setFormat(const QAudioFormat&) override {
   }

   QAudioFormat format() const override {
      return QAudioFormat();
   }
};

QList<QAudioDeviceInfo> QAudioDeviceFactory::availableDevices(QAudio::Mode mode)
{
   QList<QAudioDeviceInfo> devices;

#if ! defined(QT_NO_SETTINGS)
    QMediaPluginLoader *loader = audioLoader();

    for (const QString &key : loader->keys()) {
        QAudioSystemFactoryInterface *plugin = qobject_cast<QAudioSystemFactoryInterface*>(loader->instanceForKey(key));

        if (plugin) {
            for (QString const& handle : plugin->availableDevices(mode)) {
                devices << QAudioDeviceInfo(key, handle, mode);
            }
        }
    }
#endif
   return devices;
}

QAudioDeviceInfo QAudioDeviceFactory::defaultInputDevice()
{
#if ! defined(QT_NO_SETTINGS)
   QAudioSystemFactoryInterface* plugin = qobject_cast<QAudioSystemFactoryInterface*>(audioLoader()->instanceForKey(defaultKey()));

   if (plugin) {
      QList<QString> list = plugin->availableDevices(QAudio::AudioInput);

      if (list.size() > 0) {
         return QAudioDeviceInfo(defaultKey(), list.at(0), QAudio::AudioInput);
      }
   }

   // if no plugin is marked as default or if the default plugin doesn't have any input device,
   // return the first input available from other plugins.

   QList<QAudioDeviceInfo> inputDevices = availableDevices(QAudio::AudioInput);

   if (! inputDevices.isEmpty())
      return inputDevices.first();
#endif

   return QAudioDeviceInfo();
}

QAudioDeviceInfo QAudioDeviceFactory::defaultOutputDevice()
{
#if !defined(QT_NO_SETTINGS)
   QAudioSystemFactoryInterface *plugin = qobject_cast<QAudioSystemFactoryInterface*>(audioLoader()->instanceForKey(defaultKey()));

   if (plugin) {
      QList<QString> list = plugin->availableDevices(QAudio::AudioOutput);

      if (list.size() > 0) {
         return QAudioDeviceInfo(defaultKey(), list.at(0), QAudio::AudioOutput);
      }
   }

   // if no plugin is marked as default or if the default plugin doesn't have any output device,
   // return the first output available from other plugins
   QList<QAudioDeviceInfo> outputDevices = availableDevices(QAudio::AudioOutput);

   if (! outputDevices.isEmpty())
      return outputDevices.first();

#endif

   return QAudioDeviceInfo();
}

QAbstractAudioDeviceInfo *QAudioDeviceFactory::audioDeviceInfo(const QString &realm, const QString &handle,
      QAudio::Mode mode)
{
   QAbstractAudioDeviceInfo *rc = 0;

#if ! defined(QT_NO_SETTINGS)
    QAudioSystemFactoryInterface* plugin =
        qobject_cast<QAudioSystemFactoryInterface*>(audioLoader()->instanceForKey(realm));

   if (plugin) {
      rc = plugin->createDeviceInfo(handle, mode);
   }
#endif

   return rc == 0 ? new QNullDeviceInfo() : rc;
}

QAbstractAudioInput *QAudioDeviceFactory::createDefaultInputDevice(QAudioFormat const &format)
{
   return createInputDevice(defaultInputDevice(), format);
}

QAbstractAudioOutput *QAudioDeviceFactory::createDefaultOutputDevice(QAudioFormat const &format)
{
   return createOutputDevice(defaultOutputDevice(), format);
}

QAbstractAudioInput *QAudioDeviceFactory::createInputDevice(QAudioDeviceInfo const &deviceInfo,
      QAudioFormat const &format)
{
   if (deviceInfo.isNull()) {
      return new QNullInputDevice();
   }

#if ! defined(QT_NO_SETTINGS)
    QAudioSystemFactoryInterface* plugin =
        qobject_cast<QAudioSystemFactoryInterface*>(audioLoader()->instanceForKey(deviceInfo.realm()));

   if (plugin) {
     QAbstractAudioInput* p = plugin->createInput(deviceInfo.handle());
     if (p) p->setFormat(format);
        return p;
   }
#endif
   return new QNullInputDevice();
}

QAbstractAudioOutput *QAudioDeviceFactory::createOutputDevice(QAudioDeviceInfo const &deviceInfo,
      QAudioFormat const &format)
{
   if (deviceInfo.isNull()) {
      return new QNullOutputDevice();
   }

#if ! defined(QT_NO_SETTINGS)
    QAudioSystemFactoryInterface* plugin =
        qobject_cast<QAudioSystemFactoryInterface*>(audioLoader()->instanceForKey(deviceInfo.realm()));

    if (plugin) {
        QAbstractAudioOutput* p = plugin->createOutput(deviceInfo.handle());
        if (p) p->setFormat(format);
           return p;
    }
#endif

   return new QNullOutputDevice();
}

