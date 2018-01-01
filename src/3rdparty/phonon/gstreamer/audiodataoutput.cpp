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

/**************************************************************
** Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
** Copyright (C) 2009 Martin Sandsmark <sandsmark@samfundet.no>
**************************************************************/

#include "audiodataoutput.h"
#include "gsthelper.h"
#include "medianode.h"
#include "mediaobject.h"
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <phonon/audiooutput.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{
AudioDataOutput::AudioDataOutput(Backend *backend, QObject *parent)
    : QObject(parent), MediaNode(backend, AudioSink | AudioSource)
{
    static int count = 0;
    m_name = "AudioDataOutput" + QString::number(count++);

    m_queue = gst_element_factory_make ("identity", NULL);
    gst_object_ref(m_queue);
    m_isValid = true;
}

AudioDataOutput::~AudioDataOutput()
{
    gst_element_set_state(m_queue, GST_STATE_NULL);
    gst_object_unref(m_queue);
}

int AudioDataOutput::dataSize() const
{
    return m_dataSize;
}

int AudioDataOutput::sampleRate() const
{
    return 44100;
}

void AudioDataOutput::setDataSize(int size)
{
    m_dataSize = size;
}

typedef QMap<Phonon::AudioDataOutput::Channel, QVector<float> > FloatMap;
typedef QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > IntMap;

inline void AudioDataOutput::convertAndEmit(const QVector<qint16> &leftBuffer, const QVector<qint16> &rightBuffer)
{
    //TODO: Floats
    IntMap map;
    map.insert(Phonon::AudioDataOutput::LeftChannel, leftBuffer);
    map.insert(Phonon::AudioDataOutput::RightChannel, rightBuffer);
    emit dataReady(map);
}

void AudioDataOutput::processBuffer(GstPad*, GstBuffer* buffer, gpointer gThat)
{
    // TODO emit endOfMedia
    AudioDataOutput *that = reinterpret_cast<AudioDataOutput*>(gThat);

    // determine the number of channels
    GstStructure* structure = gst_caps_get_structure (GST_BUFFER_CAPS(buffer), 0);
    gst_structure_get_int (structure, "channels", &that->m_channels);

    if (that->m_channels > 2 || that->m_channels < 0) {
        qWarning() << Q_FUNC_INFO << ": Number of channels not supported: " << that->m_channels;
        return;
    }

    gint16 *data = reinterpret_cast<gint16*>(GST_BUFFER_DATA(buffer));
    guint size = GST_BUFFER_SIZE(buffer) / sizeof(gint16);

    that->m_pendingData.reserve(that->m_pendingData.size() + size);

    for (uint i=0; i<size; i++) {
        // 8 bit? interleaved? yay for lacking documentation!
        that->m_pendingData.append(data[i]);
    }

    while (that->m_pendingData.size() > that->m_dataSize * that->m_channels) {
        if (that->m_channels == 1) {
            QVector<qint16> intBuffer(that->m_dataSize);
            memcpy(intBuffer.data(), that->m_pendingData.constData(), that->m_dataSize * sizeof(qint16));

            that->convertAndEmit(intBuffer, intBuffer);
            int newSize = that->m_pendingData.size() - that->m_dataSize;
            memmove(that->m_pendingData.data(), that->m_pendingData.constData() + that->m_dataSize, newSize * sizeof(qint16));
            that->m_pendingData.resize(newSize);
        } else {
            QVector<qint16> left(that->m_dataSize), right(that->m_dataSize);
            for (int i=0; i<that->m_dataSize; i++) {
                left[i] = that->m_pendingData[i*2];
                right[i] = that->m_pendingData[i*2+1];
            }
            that->m_pendingData.resize(that->m_pendingData.size() - that->m_dataSize*2);
            that->convertAndEmit(left, right);
        }
    }
}

void AudioDataOutput::mediaNodeEvent(const MediaNodeEvent *event)
{
    if (event->type() == MediaNodeEvent::MediaObjectConnected && root()) {
        g_object_set(G_OBJECT(audioElement()), "sync", true, (const char*)NULL);
        GstPad *audiopad = gst_element_get_pad (audioElement(), "src");
        gst_pad_add_buffer_probe (audiopad, G_CALLBACK(processBuffer), this);
        gst_object_unref (audiopad);
        return;
    }

    MediaNode::mediaNodeEvent(event);
}

}} //namespace Phonon::Gstreamer

QT_END_NAMESPACE

