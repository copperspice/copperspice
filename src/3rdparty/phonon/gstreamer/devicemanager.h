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

#ifndef GSTREAMER_DEVICEMANAGER_H
#define GSTREAMER_DEVICEMANAGER_H

#include "common.h"
#include <phonon/audiooutputinterface.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <gst/gst.h>

QT_BEGIN_NAMESPACE

namespace Phonon {
namespace Gstreamer {
class Backend;
class DeviceManager;
class AbstractRenderer;
class VideoWidget;

class AudioDevice {
public :
    AudioDevice(DeviceManager *s, const QByteArray &deviceId);
    int id;
    QByteArray gstId;
    QByteArray description;
    QString icon;
};

class DeviceManager : public QObject {
    GSTRM_CS_OBJECT(DeviceManager)

public:
    DeviceManager(Backend *parent);
    virtual ~DeviceManager();
    const QList<AudioDevice> audioOutputDevices() const;
    GstPad *requestPad(int device) const;
    int allocateDeviceId();
    int deviceId(const QByteArray &gstId) const;
    const QByteArray gstId(int id);

    AudioDevice* audioDevice(int id);
    GstElement *createGNOMEAudioSink(Category category);
    GstElement *createAudioSink(Category category = NoCategory);
    AbstractRenderer *createVideoRenderer(VideoWidget *parent);

    GSTRM_CS_SIGNAL_1(Public, void deviceAdded(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(deviceAdded,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void deviceRemoved(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(deviceRemoved,un_named_arg1) 

    GSTRM_CS_SLOT_1(Public, void updateDeviceList())
    GSTRM_CS_SLOT_2(updateDeviceList) 

private:
    bool canOpenDevice(GstElement *element) const;
    Backend *m_backend;
    QList <AudioDevice> m_audioDeviceList;
    int m_audioDeviceCounter;
    QTimer m_devicePollTimer;
    QByteArray m_audioSink;
    QByteArray m_videoSinkWidget;
};
}
} // namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif