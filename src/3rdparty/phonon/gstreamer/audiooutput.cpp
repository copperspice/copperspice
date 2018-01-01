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

#include "common.h"
#include "audiooutput.h"
#include "backend.h"
#include "mediaobject.h"
#include "gsthelper.h"
#include <phonon/audiooutput.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{
AudioOutput::AudioOutput(Backend *backend, QObject *parent)
        : QObject(parent)
        , MediaNode(backend, AudioSink)
        , m_volumeLevel(1.0)
        , m_device(0) // ### get from backend
        , m_volumeElement(0)
        , m_audioBin(0)
        , m_audioSink(0)
        , m_conv(0)
{
    static int count = 0;
    m_name = "AudioOutput" + QString::number(count++);
    if (m_backend->isValid()) {
        m_audioBin = gst_bin_new (NULL);
        gst_object_ref (GST_OBJECT (m_audioBin));
        gst_object_sink (GST_OBJECT (m_audioBin));     

        m_conv = gst_element_factory_make ("audioconvert", NULL);

        // Get category from parent
        Phonon::Category category = Phonon::NoCategory;
        if (Phonon::AudioOutput *audioOutput = qobject_cast<Phonon::AudioOutput *>(parent))
            category = audioOutput->category();
    
        m_audioSink = m_backend->deviceManager()->createAudioSink(category);
        m_volumeElement = gst_element_factory_make ("volume", NULL);
        GstElement *queue = gst_element_factory_make ("queue", NULL);
        GstElement *audioresample = gst_element_factory_make ("audioresample", NULL);
    
        if (queue && m_audioBin && m_conv && audioresample && m_audioSink && m_volumeElement) {
            gst_bin_add_many (GST_BIN (m_audioBin), queue, m_conv, audioresample, m_volumeElement, m_audioSink, (const char*)NULL);
    
            if (gst_element_link_many (queue, m_conv, audioresample, m_volumeElement, m_audioSink, (const char*)NULL)) {
                // Add ghost sink for audiobin
                GstPad *audiopad = gst_element_get_pad (queue, "sink");
                gst_element_add_pad (m_audioBin, gst_ghost_pad_new ("sink", audiopad));
                gst_object_unref (audiopad);
                m_isValid = true; // Initialization ok, accept input
            }
        }
    }
}

void AudioOutput::mediaNodeEvent(const MediaNodeEvent *event)
{
    if (!m_audioBin)
        return;

    switch (event->type()) {

    default:
        break;
    }
}


AudioOutput::~AudioOutput()
{
    if (m_audioBin) {
        gst_element_set_state (m_audioBin, GST_STATE_NULL);
        gst_object_unref (m_audioBin);
    }
}

qreal AudioOutput::volume() const
{
    return m_volumeLevel;
}

int AudioOutput::outputDevice() const
{
    return m_device;
}

void AudioOutput::setVolume(qreal newVolume)
{
    if (newVolume > 2.0 )
        newVolume = 2.0;
    else if (newVolume < 0.0)
        newVolume = 0.0;

    if (newVolume == m_volumeLevel)
        return;

    m_volumeLevel = newVolume;

    if (m_volumeElement) {
        g_object_set(G_OBJECT(m_volumeElement), "volume", newVolume, (const char*)NULL);
    }

    emit volumeChanged(newVolume);
}

bool AudioOutput::setOutputDevice(int newDevice)
{
    m_backend->logMessage(Q_FUNC_INFO + QString::number(newDevice), Backend::Info, this);

    if (newDevice == m_device)
        return true;

    if (root()) {
        root()->saveState();
        if (gst_element_set_state(root()->pipeline(), GST_STATE_READY) == GST_STATE_CHANGE_FAILURE)
            return false;
    }

    bool success = false;
    if (m_audioSink &&  newDevice >= 0) {
        // Save previous state
        GstState oldState = GST_STATE(m_audioSink);
        const QByteArray oldDeviceValue = GstHelper::property(m_audioSink, "device");
        const QByteArray deviceId = m_backend->deviceManager()->gstId(newDevice);
        m_device = newDevice;

        // We test if the device can be opened by checking if it can go from NULL to READY state
        gst_element_set_state(m_audioSink, GST_STATE_NULL);
        success = GstHelper::setProperty(m_audioSink, "device", deviceId);
        if (success) {
            success = (gst_element_set_state(m_audioSink, oldState) == GST_STATE_CHANGE_SUCCESS);
        }
        if (!success) { // Revert state
            m_backend->logMessage(Q_FUNC_INFO +
                                  QLatin1String(" Failed to change device ") +
                                  deviceId, Backend::Info, this);

            GstHelper::setProperty(m_audioSink, "device", oldDeviceValue);
            gst_element_set_state(m_audioSink, oldState);
        } else {
            m_backend->logMessage(Q_FUNC_INFO +
                                  QLatin1String(" Successfully changed device ") +
                                  deviceId, Backend::Info, this);
        }

        // Note the stopped state should not really be necessary, but seems to be required to
        // properly reset after changing the audio state
        if (root()) {
            QMetaObject::invokeMethod(root(), "setState", Qt::QueuedConnection, Q_ARG(State, StoppedState));
            root()->resumeState();
        }
    }
    return success;
}

#if (PHONON_VERSION >= PHONON_VERSION_CHECK(4, 2, 0))
bool AudioOutput::setOutputDevice(const AudioOutputDevice &newDevice)
{
    m_backend->logMessage(Q_FUNC_INFO, Backend::Info, this);
    if (!m_audioSink || !newDevice.isValid()) {
        return false;
    }
    const QVariant driver = newDevice.property("driver");
    if (!driver.isValid()) {
        return setOutputDevice(newDevice.index());
    }
    if (newDevice.index() == m_device) {
        return true;
    }

    if (root()) {
        root()->saveState();
        if (gst_element_set_state(root()->pipeline(), GST_STATE_READY) == GST_STATE_CHANGE_FAILURE)
            return false;
    }

    // Save previous state
    const GstState oldState = GST_STATE(m_audioSink);
    const QByteArray oldDeviceValue = GstHelper::property(m_audioSink, "device");

    const QByteArray sinkName = GstHelper::property(m_audioSink, "name");
    if (sinkName == "alsasink" || sinkName == "alsasink2") {
        if (driver.toByteArray() != "alsa") {
            return false;
        }
    }

    const QVariant deviceIdsProperty = newDevice.property("deviceIds");
    QStringList deviceIds;
    if (deviceIdsProperty.type() == QVariant::StringList) {
        deviceIds = deviceIdsProperty.toStringList();
    } else if (deviceIdsProperty.type() == QVariant::String) {
        deviceIds += deviceIdsProperty.toString();
    }

    // We test if the device can be opened by checking if it can go from NULL to READY state
    for (const QString &deviceId : deviceIds) {
        gst_element_set_state(m_audioSink, GST_STATE_NULL);
        if (GstHelper::setProperty(m_audioSink, "device", deviceId.toUtf8())) {
            m_backend->logMessage(Q_FUNC_INFO + QLatin1String("setProperty(device,") +
                                  deviceId + QLatin1String(") succeeded"), Backend::Info, this);
            if (gst_element_set_state(m_audioSink, oldState) == GST_STATE_CHANGE_SUCCESS) {
                m_backend->logMessage(Q_FUNC_INFO + QLatin1String("go to old state on device") +
                                      deviceId + QLatin1String(" succeeded"), Backend::Info, this);
                m_device = newDevice.index();
                if (root()) {
                    QMetaObject::invokeMethod(root(), "setState", Qt::QueuedConnection, Q_ARG(State, StoppedState));
                    root()->resumeState();
                }
                return true;
            } else {
                m_backend->logMessage(Q_FUNC_INFO + QLatin1String("go to old state on device") +
                                      deviceId + QLatin1String(" failed"), Backend::Info, this);
            }
        } else {
            m_backend->logMessage(Q_FUNC_INFO + QLatin1String("setProperty(device,") +
                                  deviceId + QLatin1String(") failed"), Backend::Info, this);
        }
    }
    // Revert state
    GstHelper::setProperty(m_audioSink, "device", oldDeviceValue);
    gst_element_set_state(m_audioSink, oldState);

    if (root()) {
        QMetaObject::invokeMethod(root(), "setState", Qt::QueuedConnection, Q_ARG(State, StoppedState));
        root()->resumeState();
    }

    return false;
}
#endif

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
