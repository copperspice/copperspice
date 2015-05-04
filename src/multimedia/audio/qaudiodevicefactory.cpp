/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QtCore/qdebug.h>
#include <QtMultimedia/qaudioengine.h>
#include <QtMultimedia/qaudioengineplugin.h>
#include <qfactoryloader_p.h>
#include <qaudiodevicefactory_p.h>

#ifndef QT_NO_AUDIO_BACKEND
#if defined(Q_OS_WIN)
#include <qaudiodeviceinfo_win32_p.h>
#include <qaudiooutput_win32_p.h>
#include <qaudioinput_win32_p.h>

#elif defined(Q_OS_MAC)
#include <qaudiodeviceinfo_mac_p.h>
#include <qaudiooutput_mac_p.h>
#include <qaudioinput_mac_p.h>

#elif defined(HAS_ALSA)
#include <qaudiodeviceinfo_alsa_p.h>
#include <qaudiooutput_alsa_p.h>
#include <qaudioinput_alsa_p.h>
#endif
#endif

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_SETTINGS)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QAudioEngineFactoryInterface_iid, QLatin1String("/audio"), Qt::CaseInsensitive))
#endif

class QNullDeviceInfo : public QAbstractAudioDeviceInfo
{
 public:
   QAudioFormat preferredFormat() const {
      qWarning() << "using null deviceinfo, none available";
      return QAudioFormat();
   }
   bool isFormatSupported(const QAudioFormat &) const {
      return false;
   }
   QAudioFormat nearestFormat(const QAudioFormat &) const {
      return QAudioFormat();
   }
   QString deviceName() const {
      return QString();
   }
   QStringList codecList() {
      return QStringList();
   }
   QList<int> frequencyList()  {
      return QList<int>();
   }
   QList<int> channelsList() {
      return QList<int>();
   }
   QList<int> sampleSizeList() {
      return QList<int>();
   }
   QList<QAudioFormat::Endian> byteOrderList() {
      return QList<QAudioFormat::Endian>();
   }
   QList<QAudioFormat::SampleType> sampleTypeList() {
      return QList<QAudioFormat::SampleType>();
   }
};

class QNullInputDevice : public QAbstractAudioInput
{
 public:
   QIODevice *start(QIODevice *) {
      qWarning() << "using null input device, none available";
      return 0;
   }
   void stop() {}
   void reset() {}
   void suspend() {}
   void resume() {}
   int bytesReady() const {
      return 0;
   }
   int periodSize() const {
      return 0;
   }
   void setBufferSize(int ) {}
   int bufferSize() const  {
      return 0;
   }
   void setNotifyInterval(int ) {}
   int notifyInterval() const {
      return 0;
   }
   qint64 processedUSecs() const {
      return 0;
   }
   qint64 elapsedUSecs() const {
      return 0;
   }
   QAudio::Error error() const {
      return QAudio::OpenError;
   }
   QAudio::State state() const {
      return QAudio::StoppedState;
   }
   QAudioFormat format() const {
      return QAudioFormat();
   }
};

class QNullOutputDevice : public QAbstractAudioOutput
{
 public:
   QIODevice *start(QIODevice *) {
      qWarning() << "using null output device, none available";
      return 0;
   }
   void stop() {}
   void reset() {}
   void suspend() {}
   void resume() {}
   int bytesFree() const {
      return 0;
   }
   int periodSize() const {
      return 0;
   }
   void setBufferSize(int ) {}
   int bufferSize() const  {
      return 0;
   }
   void setNotifyInterval(int ) {}
   int notifyInterval() const {
      return 0;
   }
   qint64 processedUSecs() const {
      return 0;
   }
   qint64 elapsedUSecs() const {
      return 0;
   }
   QAudio::Error error() const {
      return QAudio::OpenError;
   }
   QAudio::State state() const {
      return QAudio::StoppedState;
   }
   QAudioFormat format() const {
      return QAudioFormat();
   }
};

QList<QAudioDeviceInfo> QAudioDeviceFactory::availableDevices(QAudio::Mode mode)
{
   QList<QAudioDeviceInfo> devices;

#ifndef QT_NO_AUDIO_BACKEND
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(HAS_ALSA))
   foreach (const QByteArray & handle, QAudioDeviceInfoInternal::availableDevices(mode))
   devices << QAudioDeviceInfo(QLatin1String("builtin"), handle, mode);
#endif
#endif

#if !defined(QT_NO_SETTINGS)
   QFactoryLoader *l = loader();

   foreach (QString const & key, l->keys()) {
      QAudioEngineFactoryInterface *plugin = qobject_cast<QAudioEngineFactoryInterface *>(l->instance(key));
      if (plugin) {
         foreach (QByteArray const & handle, plugin->availableDevices(mode))
         devices << QAudioDeviceInfo(key, handle, mode);
      }

      delete plugin;
   }
#endif
   return devices;
}

QAudioDeviceInfo QAudioDeviceFactory::defaultInputDevice()
{
#if !defined(QT_NO_SETTINGS)
   QAudioEngineFactoryInterface *plugin = qobject_cast<QAudioEngineFactoryInterface *>(loader()->instance(
         QLatin1String("default")));

   if (plugin) {
      QList<QByteArray> list = plugin->availableDevices(QAudio::AudioInput);
      if (list.size() > 0) {
         return QAudioDeviceInfo(QLatin1String("default"), list.at(0), QAudio::AudioInput);
      }
   }
#endif

#ifndef QT_NO_AUDIO_BACKEND
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(HAS_ALSA))
   return QAudioDeviceInfo(QLatin1String("builtin"), QAudioDeviceInfoInternal::defaultInputDevice(), QAudio::AudioInput);
#endif
#endif
   return QAudioDeviceInfo();
}

QAudioDeviceInfo QAudioDeviceFactory::defaultOutputDevice()
{
#if !defined(QT_NO_SETTINGS)
   QAudioEngineFactoryInterface *plugin = qobject_cast<QAudioEngineFactoryInterface *>(loader()->instance(
         QLatin1String("default")));

   if (plugin) {
      QList<QByteArray> list = plugin->availableDevices(QAudio::AudioOutput);
      if (list.size() > 0) {
         return QAudioDeviceInfo(QLatin1String("default"), list.at(0), QAudio::AudioOutput);
      }
   }
#endif

#ifndef QT_NO_AUDIO_BACKEND
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(HAS_ALSA))
   return QAudioDeviceInfo(QLatin1String("builtin"), QAudioDeviceInfoInternal::defaultOutputDevice(), QAudio::AudioOutput);
#endif
#endif

   return QAudioDeviceInfo();
}

QAbstractAudioDeviceInfo *QAudioDeviceFactory::audioDeviceInfo(const QString &realm, const QByteArray &handle,
      QAudio::Mode mode)
{
   QAbstractAudioDeviceInfo *rc = 0;

#ifndef QT_NO_AUDIO_BACKEND
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(HAS_ALSA))
   if (realm == QLatin1String("builtin")) {
      return new QAudioDeviceInfoInternal(handle, mode);
   }
#endif
#endif
#if !defined(QT_NO_SETTINGS)
   QAudioEngineFactoryInterface *plugin =
      qobject_cast<QAudioEngineFactoryInterface *>(loader()->instance(realm));

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

#ifndef QT_NO_AUDIO_BACKEND
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(HAS_ALSA))
   if (deviceInfo.realm() == QLatin1String("builtin")) {
      return new QAudioInputPrivate(deviceInfo.handle(), format);
   }
#endif
#endif

#if ! defined(QT_NO_SETTINGS)
   QAudioEngineFactoryInterface *plugin =
      qobject_cast<QAudioEngineFactoryInterface *>(loader()->instance(deviceInfo.realm()));

   if (plugin) {
      return plugin->createInput(deviceInfo.handle(), format);
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

#ifndef QT_NO_AUDIO_BACKEND
#if (defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(HAS_ALSA))
   if (deviceInfo.realm() == QLatin1String("builtin")) {
      return new QAudioOutputPrivate(deviceInfo.handle(), format);
   }
#endif
#endif

#if  !defined(QT_NO_SETTINGS)
   QAudioEngineFactoryInterface *plugin =
      qobject_cast<QAudioEngineFactoryInterface *>(loader()->instance(deviceInfo.realm()));

   if (plugin) {
      return plugin->createOutput(deviceInfo.handle(), format);
   }
#endif
   return new QNullOutputDevice();
}

QT_END_NAMESPACE

