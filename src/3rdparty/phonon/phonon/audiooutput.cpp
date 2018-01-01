/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include "audiooutput.h"
#include "audiooutput_p.h"
#include "factory_p.h"
#include "objectdescription.h"
#include "audiooutputadaptor_p.h"
#include "globalconfig.h"
#include "audiooutputinterface.h"
#include "phononnamespace_p.h"
#include "platform_p.h"
#include "pulsesupport.h"

#include <QtCore/qmath.h>
#include <QtCore/quuid.h>

#define PHONON_CLASSNAME AudioOutput
#define IFACES2 AudioOutputInterface42
#define IFACES1 IFACES2
#define IFACES0 AudioOutputInterface40, IFACES1
#define PHONON_INTERFACENAME IFACES0

QT_BEGIN_NAMESPACE

namespace Phonon
{

static inline bool callSetOutputDevice(AudioOutputPrivate *const d, int index)
{
    PulseSupport *pulse = PulseSupport::getInstance();

    if (pulse->isActive())
        return pulse->setOutputDevice(d->getStreamUuid(), index);

    Iface<IFACES2> iface(d);

    if (iface) {
       return iface->setOutputDevice(AudioOutputDevice::fromIndex(index));
    }

    return Iface<IFACES0>::cast(d)->setOutputDevice(index);
}

static inline bool callSetOutputDevice(AudioOutputPrivate *const d, const AudioOutputDevice &dev)
{
    PulseSupport *pulse = PulseSupport::getInstance();

    if (pulse->isActive()) {
      return pulse->setOutputDevice(d->getStreamUuid(), dev.index());
    }

    Iface<IFACES2> iface(d);

    if (iface) {
      return iface->setOutputDevice(dev);
    }

    return Iface<IFACES0>::cast(d)->setOutputDevice(dev.index());
}

AudioOutput::AudioOutput(Phonon::Category category, QObject *parent)
   : AbstractAudioOutput(* new AudioOutputPrivate, parent)
{
    K_D(AudioOutput);
    d->init(category);
}

AudioOutput::AudioOutput(QObject *parent) 
    : AbstractAudioOutput(* new AudioOutputPrivate, parent)
{
    K_D(AudioOutput);
    d->init(NoCategory);
}

void AudioOutputPrivate::init(Phonon::Category c)
{
    Q_Q(AudioOutput);

#ifndef QT_NO_DBUS
    adaptor = new AudioOutputAdaptor(q);
    static unsigned int number = 0;

    const QString &path = QLatin1String("/AudioOutputs/") + QString::number(number++);
    QDBusConnection con = QDBusConnection::sessionBus();
    con.registerObject(path, q);

    emit adaptor->newOutputAvailable(con.baseService(), path);

    q->connect(q, SIGNAL(volumeChanged(qreal)), adaptor, SLOT(volumeChanged(qreal)));
    q->connect(q, SIGNAL(mutedChanged(bool)),   adaptor, SLOT(mutedChanged(bool)));
#endif

    category = c;
    streamUuid = QUuid::createUuid().toString();
    PulseSupport *pulse = PulseSupport::getInstance();
    pulse->setStreamPropList(category, streamUuid);

    q->connect(pulse, SIGNAL(usingDevice(QString,int)), q, SLOT(_k_deviceChanged(QString,int)));

    createBackendObject();

    // may not want to do this connection if the createBackendObject failed
    q->connect(Factory::sender(), SIGNAL(availableAudioOutputDevicesChanged()), q, SLOT(_k_deviceListChanged()));
}

QString AudioOutputPrivate::getStreamUuid()
{
    return streamUuid;
}

void AudioOutputPrivate::createBackendObject()
{
    if (m_backendObject) {
        return;
    }

    Q_Q(AudioOutput);
    m_backendObject = Factory::createAudioOutput(q);

    if (m_backendObject) {

      device = AudioOutputDevice::fromIndex(GlobalConfig().audioOutputDeviceFor(category, 
                  GlobalConfig::AdvancedDevicesFromSettings | GlobalConfig::HideUnavailableDevices));

      setupBackendObject();
    }
}

QString AudioOutput::name() const
{
    K_D(const AudioOutput);
    return d->name;
}

void AudioOutput::setName(const QString &newName)
{
    K_D(AudioOutput);
    if (d->name == newName) {
        return;
    }
    d->name = newName;
    setVolume(Platform::loadVolume(newName));
#ifndef QT_NO_DBUS
    if (d->adaptor) {
        emit d->adaptor->nameChanged(newName);
    }
#endif
}

static const qreal LOUDNESS_TO_VOLTAGE_EXPONENT = qreal(0.67);
static const qreal VOLTAGE_TO_LOUDNESS_EXPONENT = qreal(1.0/LOUDNESS_TO_VOLTAGE_EXPONENT);

void AudioOutput::setVolume(qreal volume)
{
    K_D(AudioOutput);
    d->volume = volume;
    if (k_ptr->backendObject() && !d->muted) {
        // using Stevens' power law loudness is proportional to (sound pressure)^0.67
        // sound pressure is proportional to voltage:
        // p² \prop P \prop V²
        // => if a factor for loudness of x is requested
        INTERFACE_CALL(setVolume(pow(volume, VOLTAGE_TO_LOUDNESS_EXPONENT)));
    } else {
        emit volumeChanged(volume);
    }
    Platform::saveVolume(d->name, volume);
}

qreal AudioOutput::volume() const
{
    K_D(const AudioOutput);
    if (d->muted || !d->m_backendObject) {
        return d->volume;
    }
    return pow(INTERFACE_CALL(volume()), LOUDNESS_TO_VOLTAGE_EXPONENT);
}

#ifndef PHONON_LOG10OVER20
#define PHONON_LOG10OVER20
static const qreal log10over20 = qreal(0.1151292546497022842); // ln(10) / 20
#endif // PHONON_LOG10OVER20

qreal AudioOutput::volumeDecibel() const
{
    K_D(const AudioOutput);
    if (d->muted || !d->m_backendObject) {
        return log(d->volume) / log10over20;
    }
    return 0.67 * log(INTERFACE_CALL(volume())) / log10over20;
}

void AudioOutput::setVolumeDecibel(qreal newVolumeDecibel)
{
    setVolume(exp(newVolumeDecibel * log10over20));
}

bool AudioOutput::isMuted() const
{
    K_D(const AudioOutput);
    return d->muted;
}

void AudioOutput::setMuted(bool mute)
{
    K_D(AudioOutput);
    if (d->muted != mute) {
        if (mute) {
            d->muted = mute;
            if (k_ptr->backendObject()) {
                INTERFACE_CALL(setVolume(0.0));
            }
        } else {
            if (k_ptr->backendObject()) {
                INTERFACE_CALL(setVolume(pow(d->volume, VOLTAGE_TO_LOUDNESS_EXPONENT)));
            }
            d->muted = mute;
        }
        emit mutedChanged(mute);
    }
}

Category AudioOutput::category() const
{
    K_D(const AudioOutput);
    return d->category;
}

AudioOutputDevice AudioOutput::outputDevice() const
{
    K_D(const AudioOutput);
    return d->device;
}

bool AudioOutput::setOutputDevice(const AudioOutputDevice &newAudioOutputDevice)
{
    K_D(AudioOutput);
    if (!newAudioOutputDevice.isValid()) {
        d->outputDeviceOverridden = d->forceMove = false;
        const int newIndex = GlobalConfig().audioOutputDeviceFor(d->category);
        if (newIndex == d->device.index()) {
            return true;
        }
        d->device = AudioOutputDevice::fromIndex(newIndex);
    } else {
        d->outputDeviceOverridden = d->forceMove = true;
        if (d->device == newAudioOutputDevice) {
            return true;
        }
        d->device = newAudioOutputDevice;
    }
    if (k_ptr->backendObject()) {
        return callSetOutputDevice(d, d->device.index());
    }
    return true;
}

bool AudioOutputPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject) {
        volume = pINTERFACE_CALL(volume());
    }
    return AbstractAudioOutputPrivate::aboutToDeleteBackendObject();
}

void AudioOutputPrivate::setupBackendObject()
{
    Q_Q(AudioOutput);
    Q_ASSERT(m_backendObject);

    AbstractAudioOutputPrivate::setupBackendObject();

    QObject::connect(m_backendObject, SIGNAL(volumeChanged(qreal)), q, SLOT(_k_volumeChanged(qreal)));
    QObject::connect(m_backendObject, SIGNAL(audioDeviceFailed()),  q, SLOT(_k_audioDeviceFailed()));

    // set up attributes
    pINTERFACE_CALL(setVolume(pow(volume, VOLTAGE_TO_LOUDNESS_EXPONENT)));

#ifndef QT_NO_PHONON_SETTINGSGROUP
    // if the output device is not available and the device was not explicitly set 
    // There is no need to set the output device initially if PA is used as as we
    // know it will not work (stream doesn't exist yet) and that this will be handled by _k_deviceChanged()

    if (! PulseSupport::getInstance()->isActive() && ! callSetOutputDevice(this, device) && ! outputDeviceOverridden) {
        // fall back in the preference list of output devices
        QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category, 
                  GlobalConfig::AdvancedDevicesFromSettings | GlobalConfig::HideUnavailableDevices);

        if (deviceList.isEmpty()) {
            return;
        }

        for (int i = 0; i < deviceList.count(); ++i) {
            const AudioOutputDevice &dev = AudioOutputDevice::fromIndex(deviceList.at(i));
            if (callSetOutputDevice(this, dev)) {
                handleAutomaticDeviceChange(dev, AudioOutputPrivate::FallbackChange);

                return; // found one that works
            }
        }

        // if we get here there is no working output device. Tell the backend.
        const AudioOutputDevice none;
        callSetOutputDevice(this, none);
        handleAutomaticDeviceChange(none, FallbackChange);
    }

#endif 
}

void AudioOutputPrivate::_k_volumeChanged(qreal newVolume)
{
    if (!muted) {
        Q_Q(AudioOutput);
        emit q->volumeChanged(pow(newVolume, qreal(0.67)));
    }
}

void AudioOutputPrivate::_k_revertFallback()
{
    if (deviceBeforeFallback == -1) {
        return;
    }
    device = AudioOutputDevice::fromIndex(deviceBeforeFallback);
    callSetOutputDevice(this, device);
    Q_Q(AudioOutput);
    emit q->outputDeviceChanged(device);
#ifndef QT_NO_DBUS
    emit adaptor->outputDeviceIndexChanged(device.index());
#endif
}

void AudioOutputPrivate::_k_audioDeviceFailed()
{
    if (PulseSupport::getInstance()->isActive())
        return;

#ifndef QT_NO_PHONON_SETTINGSGROUP

    pDebug() << Q_FUNC_INFO;
    // outputDeviceIndex identifies a failing device
    // fall back in the preference list of output devices
    const QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category, GlobalConfig::AdvancedDevicesFromSettings | GlobalConfig::HideUnavailableDevices);
    for (int i = 0; i < deviceList.count(); ++i) {
        const int devIndex = deviceList.at(i);
        // if it's the same device as the one that failed, ignore it
        if (device.index() != devIndex) {
            const AudioOutputDevice &info = AudioOutputDevice::fromIndex(devIndex);
            if (callSetOutputDevice(this, info)) {
                handleAutomaticDeviceChange(info, FallbackChange);
                return; // found one that works
            }
        }
    }
#endif //QT_NO_PHONON_SETTINGSGROUP
    // if we get here there is no working output device. Tell the backend.
    const AudioOutputDevice none;
    callSetOutputDevice(this, none);
    handleAutomaticDeviceChange(none, FallbackChange);
}

void AudioOutputPrivate::_k_deviceListChanged()
{
    if (PulseSupport::getInstance()->isActive())
        return;

#ifndef QT_NO_PHONON_SETTINGSGROUP
    pDebug() << Q_FUNC_INFO;
    // Check to see if we have an override and do not change to a higher priority device if the overridden device is still present.
    if (outputDeviceOverridden && device.property("available").toBool()) {
        return;
    }
    // let's see if there's a usable device higher in the preference list
    const QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category, GlobalConfig::AdvancedDevicesFromSettings);
    DeviceChangeType changeType = HigherPreferenceChange;
    for (int i = 0; i < deviceList.count(); ++i) {
        const int devIndex = deviceList.at(i);
        const AudioOutputDevice &info = AudioOutputDevice::fromIndex(devIndex);
        if (!info.property("available").toBool()) {
            if (device.index() == devIndex) {
                // we've reached the currently used device and it's not available anymore, so we
                // fallback to the next available device
                changeType = FallbackChange;
            }
            pDebug() << devIndex << "is not available";
            continue;
        }
        pDebug() << devIndex << "is available";
        if (device.index() == devIndex) {
            // we've reached the currently used device, nothing to change
            break;
        }
        if (callSetOutputDevice(this, info)) {
            handleAutomaticDeviceChange(info, changeType);
            break; // found one with higher preference that works
        }
    }
#endif //QT_NO_PHONON_SETTINGSGROUP
}

void AudioOutputPrivate::_k_deviceChanged(QString inStreamUuid, int deviceIndex)
{
    // Note that this method is only used by PulseAudio at present.
    if (inStreamUuid == streamUuid) {
        // 1. Check to see if we are overridden. If we are, and devices do not match,
        //    then try and apply our own device as the output device.
        //    We only do this the first time
        if (outputDeviceOverridden && forceMove) {
            forceMove = false;
            const AudioOutputDevice &currentDevice = AudioOutputDevice::fromIndex(deviceIndex);
            if (currentDevice != device) {
                if (!callSetOutputDevice(this, device)) {
                    // What to do if we are overridden and cannot change to our preferred device?
                }
            }
        }
        // 2. If we are not overridden, then we need to update our perception of what
        //    device we are using. If the devices do not match, something lower in the
        //    stack is overriding our preferences (e.g. a per-application stream preference,
        //    specific application move, priority list changed etc. etc.)
        else if (!outputDeviceOverridden) {
            const AudioOutputDevice &currentDevice = AudioOutputDevice::fromIndex(deviceIndex);
            if (currentDevice != device) {
                // The device is not what we think it is, so lets say what is happening.
                handleAutomaticDeviceChange(currentDevice, SoundSystemChange);
            }
        }
    }
}

static struct
{
    int first;
    int second;
} g_lastFallback = { 0, 0 };

void AudioOutputPrivate::handleAutomaticDeviceChange(const AudioOutputDevice &device2, DeviceChangeType type)
{
    Q_Q(AudioOutput);

    deviceBeforeFallback = device.index();
    device = device2;

    emit q->outputDeviceChanged(device2);

#ifndef QT_NO_DBUS
    emit adaptor->outputDeviceIndexChanged(device.index());
#endif

    const AudioOutputDevice &device1 = AudioOutputDevice::fromIndex(deviceBeforeFallback);
    switch (type) {
    case FallbackChange:

        if (g_lastFallback.first != device1.index() || g_lastFallback.second != device2.index()) {

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
            const QString &text = AudioOutput::tr("<html>The audio playback device <b>%1</b> does not work.<br/>"
                        "Falling back to <b>%2</b>.</html>").arg(device1.name()).arg(device2.name()); 

            Platform::notification("AudioDeviceFallback", text);

#endif //QT_NO_PHONON_PLATFORMPLUGIN
            g_lastFallback.first = device1.index();
            g_lastFallback.second = device2.index();
        }
        break;

    case HigherPreferenceChange:
        {

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
        const QString text = AudioOutput::tr("<html>Switching to the audio playback device <b>%1</b><br/>"
                "which just became available and has higher preference.</html>").arg(device2.name());

        Platform::notification("AudioDeviceFallback", text,
                QStringList(AudioOutput::tr("Revert back to device '%1'").arg(device1.name())), q, SLOT(_k_revertFallback()));
#endif
        g_lastFallback.first = 0;
        g_lastFallback.second = 0;
        }
        break;

    case SoundSystemChange:
        {

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
        if (device1.property("available").toBool()) {

            const QString text = AudioOutput::tr("<html>Switching to the audio playback device <b>%1</b><br/>"
                    "which has higher preference or is specifically configured for this stream.</html>").arg(device2.name());

            Platform::notification("AudioDeviceFallback", text,
                    QStringList(AudioOutput::tr("Revert back to device '%1'").arg(device1.name())),
                    q, SLOT(_k_revertFallback()));
        } else {
            const QString &text = AudioOutput::tr("<html>The audio playback device <b>%1</b> does not work.<br/>"
                    "Falling back to <b>%2</b>.</html>").arg(device1.name()).arg(device2.name());

            Platform::notification("AudioDeviceFallback", text);
        }
#endif

        //outputDeviceOverridden = true;
        g_lastFallback.first = 0;
        g_lastFallback.second = 0;
        }
        break;
    }
}

AudioOutputPrivate::~AudioOutputPrivate()
{
    PulseSupport::getInstance()->clearStreamCache(streamUuid);
#ifndef QT_NO_DBUS
    if (adaptor) {
        emit adaptor->outputDestroyed();
    }
#endif
}

void AudioOutput::_k_volumeChanged(qreal un_named_arg1)
{
	K_D(AudioOutput);
	d->_k_volumeChanged(un_named_arg1);
}

void AudioOutput::_k_revertFallback()
{
	K_D(AudioOutput);
	d->_k_revertFallback();
}

void AudioOutput::_k_audioDeviceFailed()
{
	K_D(AudioOutput);
	d->_k_audioDeviceFailed();
}

void AudioOutput::_k_deviceListChanged()
{
	K_D(AudioOutput);
	d->_k_deviceListChanged();
}

void AudioOutput::_k_deviceChanged(QString streamUuid,int device)
{
	K_D(AudioOutput);
	d->_k_deviceChanged(streamUuid, device);
}

} //namespace Phonon

QT_END_NAMESPACE


template <>  
const char * cs_typeName_internal<Phonon::AudioOutputDevice, void>::typeName() 
{ 
   return "AudioOutputDevice"; 
} 

template const char *cs_typeName_internal<Phonon::AudioOutputDevice,void>::typeName();


#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
#undef IFACES2
#undef IFACES1
#undef IFACES0
