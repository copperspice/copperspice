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

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#include <gst/interfaces/propertyprobe.h>
#include "devicemanager.h"
#include "backend.h"
#include "gsthelper.h"
#include "videowidget.h"
#include "glrenderer.h"
#include "widgetrenderer.h"

#ifdef Q_WS_X11
#include "x11renderer.h"
#endif

#include "artssink.h"
#include "pulsesupport.h"

#ifdef USE_ALSASINK2
#include "alsasink2.h"
#endif

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{

AudioDevice::AudioDevice(DeviceManager *manager, const QByteArray &gstId)
        : gstId(gstId)
{
    // This should never be called when PulseAudio is active.
    Q_ASSERT(!PulseSupport::getInstance()->isActive());

    id = manager->allocateDeviceId();
    icon = "audio-card";

    //get name from device
    if (gstId == "default") {
        description = "Default audio device";
    } else {
        GstElement *aSink= manager->createAudioSink();

        if (aSink) {
            gchar *deviceDescription = NULL;

            if (GST_IS_PROPERTY_PROBE(aSink) && gst_property_probe_get_property( GST_PROPERTY_PROBE(aSink), "device" ) ) {
                g_object_set (G_OBJECT(aSink), "device", gstId.constData(), (const char*)NULL);
                g_object_get (G_OBJECT(aSink), "device-name", &deviceDescription, (const char*)NULL);
                description = QByteArray(deviceDescription);
                g_free (deviceDescription);
                gst_element_set_state(aSink, GST_STATE_NULL);
                gst_object_unref (aSink);
            }
        }
    }
}

DeviceManager::DeviceManager(Backend *backend)
        : QObject(backend)
        , m_backend(backend)
        , m_audioDeviceCounter(0)
{
    QSettings settings(QLatin1String("CopperSpice"));
    settings.beginGroup(QLatin1String("CS"));

    PulseSupport *pulse = PulseSupport::getInstance();
    m_audioSink = qgetenv("PHONON_GST_AUDIOSINK");

    if (m_audioSink.isEmpty()) {
        m_audioSink = settings.value(QLatin1String("audiosink"), QString("Auto")).toByteArray().toLower();
        if (m_audioSink == "auto" && pulse->isActive())
            m_audioSink = "pulsesink";
    }
    if ("pulsesink" != m_audioSink)
        pulse->enable(false);

    m_videoSinkWidget = qgetenv("PHONON_GST_VIDEOMODE");
    if (m_videoSinkWidget.isEmpty()) {
        m_videoSinkWidget = settings.value(QLatin1String("videomode"), QString("Auto")).toByteArray().toLower();
    }

    if (m_backend->isValid())
        updateDeviceList();
}

DeviceManager::~DeviceManager()
{
    m_audioDeviceList.clear();
}

/***
* Returns a Gst Audiosink based on GNOME configuration settings,
* or 0 if the element is not available.
*/
GstElement *DeviceManager::createGNOMEAudioSink(Category category)
{
    GstElement *sink = gst_element_factory_make ("gconfaudiosink", NULL);

    if (sink) {

        // set profile property on the gconfaudiosink to "music and movies"
        if (g_object_class_find_property (G_OBJECT_GET_CLASS (sink), "profile")) {
            switch (category) {
            case NotificationCategory:
                g_object_set (G_OBJECT (sink), "profile", 0, (const char*)NULL); // 0 = 'sounds'
                break;
            case CommunicationCategory:
                g_object_set (G_OBJECT (sink), "profile", 2, (const char*)NULL); // 2 = 'chat'
                break;
            default:
                g_object_set (G_OBJECT (sink), "profile", 1, (const char*)NULL); // 1 = 'music and movies'
                break;
            }
        }
    }
    return sink;
}


bool DeviceManager::canOpenDevice(GstElement *element) const
{
    if (!element)
        return false;

    if (gst_element_set_state(element, GST_STATE_READY) == GST_STATE_CHANGE_SUCCESS)
        return true;

    const QList<QByteArray> &list = GstHelper::extractProperties(element, "device");
    for (const QByteArray &gstId : list) {
        GstHelper::setProperty(element, "device", gstId);
        if (gst_element_set_state(element, GST_STATE_READY) == GST_STATE_CHANGE_SUCCESS) {
            return true;
        }
    }
    // FIXME: the above can still fail for a valid alsasink because list only contains entries of
    // the form "hw:X,Y". Would be better to use "default:X" or "dmix:X,Y"

    gst_element_set_state(element, GST_STATE_NULL);
    return false;
}

/*
*
* Returns a GstElement with a valid audio sink
* based on the current value of PHONON_GSTREAMER_DRIVER
*
* Allowed values are auto (default), alsa, oss, arts and ess
* does not exist
*
* If no real sound sink is available a fakesink will be returned
*/
GstElement *DeviceManager::createAudioSink(Category category)
{
    GstElement *sink = 0;

    if (m_backend && m_backend->isValid())
    {
        if (m_audioSink == "auto") //this is the default value
        {
            //### TODO : get equivalent KDE settings here

            if (!qgetenv("GNOME_DESKTOP_SESSION_ID").isEmpty()) {
                sink = createGNOMEAudioSink(category);
                if (canOpenDevice(sink))
                    m_backend->logMessage("AudioOutput using gconf audio sink");
                else if (sink) {
                    gst_object_unref(sink);
                    sink = 0;
                }
            }

#ifdef USE_ALSASINK2
            if (!sink) {
                sink = gst_element_factory_make ("_k_alsasink", NULL);
                if (canOpenDevice(sink))
                    m_backend->logMessage("AudioOutput using alsa2 audio sink");
                else if (sink) {
                    gst_object_unref(sink);
                    sink = 0;
                }
            }
#endif

            if (!sink) {
                sink = gst_element_factory_make ("alsasink", NULL);
                if (canOpenDevice(sink))
                    m_backend->logMessage("AudioOutput using alsa audio sink");
                else if (sink) {
                    gst_object_unref(sink);
                    sink = 0;
                }
            }

            if (!sink) {
                sink = gst_element_factory_make ("autoaudiosink", NULL);
                if (canOpenDevice(sink))
                    m_backend->logMessage("AudioOutput using auto audio sink");
                else if (sink) {
                    gst_object_unref(sink);
                    sink = 0;
                }
            }

            if (!sink) {
                sink = gst_element_factory_make ("osssink", NULL);
                if (canOpenDevice(sink))
                    m_backend->logMessage("AudioOutput using oss audio sink");
                else if (sink) {
                    gst_object_unref(sink);
                    sink = 0;
                }
            }
        } else if (m_audioSink == "fake") {
            //do nothing as a fakesink will be created by default
        } else if (m_audioSink == "artssink") {
            sink = GST_ELEMENT(g_object_new(arts_sink_get_type(), NULL));

        } else if (!m_audioSink.isEmpty()) { //Use a custom sink
            sink = gst_element_factory_make (m_audioSink.constData(), NULL);

            if (canOpenDevice(sink))
                m_backend->logMessage(QString("AudioOutput using %0").formatArg(QString::fromUtf8(m_audioSink)));

            else if (sink) {
                gst_object_unref(sink);
                sink = 0;
            }
        }
    }

    if (!sink) { //no suitable sink found so we'll make a fake one
        sink = gst_element_factory_make("fakesink", NULL);
        if (sink) {
            m_backend->logMessage("AudioOutput Using fake audio sink");
            //without sync the sink will pull the pipeline as fast as the CPU allows
            g_object_set (G_OBJECT (sink), "sync", TRUE, (const char*)NULL);
        }
    }
    Q_ASSERT(sink);
    return sink;
}

#ifndef QT_NO_PHONON_VIDEO
AbstractRenderer *DeviceManager::createVideoRenderer(VideoWidget *parent)
{
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES)
    if (m_videoSinkWidget == "opengl") {
        return new GLRenderer(parent);
    } else
#endif
    if (m_videoSinkWidget == "software") {
        return new WidgetRenderer(parent);
    }
#ifdef Q_WS_X11
    else if (m_videoSinkWidget == "xwindow") {
        return new X11Renderer(parent);
    } else {
        GstElementFactory *srcfactory = gst_element_factory_find("ximagesink");
        if (srcfactory) {
            return new X11Renderer(parent);
        }
    }
#endif
    return new WidgetRenderer(parent);
}
#endif //QT_NO_PHONON_VIDEO

/**
 * Allocate a device id for a new audio device
 */
int DeviceManager::allocateDeviceId()
{
    return m_audioDeviceCounter++;
}


/**
 * Returns a positive device id or -1 if device does not exist
 *
 * The gstId is typically in the format hw:1,0
 */
int DeviceManager::deviceId(const QByteArray &gstId) const
{
    for (int i = 0 ; i < m_audioDeviceList.size() ; ++i) {
        if (m_audioDeviceList[i].gstId == gstId) {
            return m_audioDeviceList[i].id;
        }
    }
    return -1;
}

/**
 * Returns a gstId or "default" if device does not exist
 *
 * The gstId is typically in the format hw:1,0
 */
const QByteArray DeviceManager::gstId(int deviceId)
{
    if (!PulseSupport::getInstance()->isActive()) {
        AudioDevice *ad = audioDevice(deviceId);
        if (ad)
            return QByteArray(ad->gstId);
    }
    return QByteArray("default");
}

/**
* Get the AudioDevice for a given device id
*/
AudioDevice* DeviceManager::audioDevice(int id)
{
    for (int i = 0 ; i < m_audioDeviceList.size() ; ++i) {
        if (m_audioDeviceList[i].id == id)
            return &m_audioDeviceList[i];
    }
    return NULL;
}

/**
 * Updates the current list of active devices
 */
void DeviceManager::updateDeviceList()
{
    //fetch list of current devices
    GstElement *audioSink= createAudioSink();

    QList<QByteArray> list;

    if (audioSink) {
        if (!PulseSupport::getInstance()->isActive()) {
            // If we're using pulse, the PulseSupport class takes care of things for us.
            list = GstHelper::extractProperties(audioSink, "device");
            list.prepend("default");
        }

        for (int i = 0 ; i < list.size() ; ++i) {
            QByteArray gstId = list.at(i);
            if (deviceId(gstId) == -1) {
                // This is a new device, add it
                m_audioDeviceList.append(AudioDevice(this, gstId));
                emit deviceAdded(deviceId(gstId));
                m_backend->logMessage(QString("Found new audio device %0").formatArg(QString::fromUtf8(gstId)), Backend::Debug, this);
            }
        }

        if (list.size() < m_audioDeviceList.size()) {
            //a device was removed
            for (int i = m_audioDeviceList.size() -1 ; i >= 0 ; --i) {
                QByteArray currId = m_audioDeviceList[i].gstId;
                bool found = false;

                for (int k = list.size() -1  ; k >= 0 ; --k) {
                    if (currId == list[k]) {
                        found = true;
                        break;
                    }
                }
                if (! found) {
                    m_backend->logMessage(QString("Audio device lost %0").formatArg(QString::fromUtf8(currId)), Backend::Debug, this);
                    emit deviceRemoved(deviceId(currId));
                    m_audioDeviceList.removeAt(i);
                }
            }
        }
    }

    gst_element_set_state (audioSink, GST_STATE_NULL);
    gst_object_unref (audioSink);
}

/**
  * Returns a list of hardware id usable by gstreamer [i.e hw:1,0]
  */
const QList<AudioDevice> DeviceManager::audioOutputDevices() const
{
    return m_audioDeviceList;
}

}
}

QT_END_NAMESPACE
