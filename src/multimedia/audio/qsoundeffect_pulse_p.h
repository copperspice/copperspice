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

#ifndef QSOUNDEFFECT_PULSE_H
#define QSOUNDEFFECT_PULSE_H

#include <qsoundeffect.h>

#include <qobject.h>
#include <qdatetime.h>
#include <qreadwritelock.h>
#include <qmediaplayer.h>

#include <qsamplecache_p.h>
#include <qmediaresourcepolicy_p.h>
#include <qmediaresourceset_p.h>

#include <pulse/pulseaudio.h>

class QSoundEffectRef;

class QSoundEffectPrivate : public QObject
{
    MULTI_CS_OBJECT(QSoundEffectPrivate)

public:
    explicit QSoundEffectPrivate(QObject* parent);
    ~QSoundEffectPrivate();

    static QStringList supportedMimeTypes();

    QUrl source() const;
    void setSource(const QUrl &url);
    int loopCount() const;
    int loopsRemaining() const;
    void setLoopCount(int loopCount);
    qreal volume() const;
    void setVolume(qreal volume);
    bool isMuted() const;
    void setMuted(bool muted);
    bool isLoaded() const;
    bool isPlaying() const;
    QSoundEffect::Status status() const;

    void release();

    QString category() const;
    void setCategory(const QString &category);

    MULTI_CS_SLOT_1(Public, void play())
    MULTI_CS_SLOT_2(play)
    MULTI_CS_SLOT_1(Public, void stop())
    MULTI_CS_SLOT_2(stop)

    MULTI_CS_SIGNAL_1(Public, void loopsRemainingChanged())
    MULTI_CS_SIGNAL_2(loopsRemainingChanged)
    MULTI_CS_SIGNAL_1(Public, void volumeChanged())
    MULTI_CS_SIGNAL_2(volumeChanged)
    MULTI_CS_SIGNAL_1(Public, void mutedChanged())
    MULTI_CS_SIGNAL_2(mutedChanged)
    MULTI_CS_SIGNAL_1(Public, void loadedChanged())
    MULTI_CS_SIGNAL_2(loadedChanged)
    MULTI_CS_SIGNAL_1(Public, void playingChanged())
    MULTI_CS_SIGNAL_2(playingChanged)
    MULTI_CS_SIGNAL_1(Public, void statusChanged())
    MULTI_CS_SIGNAL_2(statusChanged)
    MULTI_CS_SIGNAL_1(Public, void categoryChanged())
    MULTI_CS_SIGNAL_2(categoryChanged)

 private:
    enum EmptyStreamOption {
        ReloadSampleWhenDone = 0x1
    };

    MULTI_CS_SLOT_1(Private, void decoderError())
    MULTI_CS_SLOT_2(decoderError)

    MULTI_CS_SLOT_1(Private, void sampleReady())
    MULTI_CS_SLOT_2(sampleReady)

    MULTI_CS_SLOT_1(Private, void uploadSample())
    MULTI_CS_SLOT_2(uploadSample)

    MULTI_CS_SLOT_1(Private, void contextReady())
    MULTI_CS_SLOT_2(contextReady)

    MULTI_CS_SLOT_1(Private, void contextFailed())
    MULTI_CS_SLOT_2(contextFailed)

    MULTI_CS_SLOT_1(Private, void underRun())
    MULTI_CS_SLOT_2(underRun)

    MULTI_CS_SLOT_1(Private, void prepare())
    MULTI_CS_SLOT_2(prepare)

    MULTI_CS_SLOT_1(Private, void streamReady())
    MULTI_CS_SLOT_2(streamReady)

    MULTI_CS_SLOT_1(Private, void emptyComplete(void * stream,bool reload))
    MULTI_CS_SLOT_2(emptyComplete)

    MULTI_CS_SLOT_1(Private, void handleAvailabilityChanged(bool available))
    MULTI_CS_SLOT_2(handleAvailabilityChanged)

    void playAvailable();
    void playSample();

    using EmptyStreamOptions = QFlags<EmptyStreamOption>;
    void emptyStream(EmptyStreamOptions options = EmptyStreamOptions());

    void createPulseStream();
    void unloadPulseStream();

    int writeToStream(const void *data, int size);

    void setPlaying(bool playing);
    void setStatus(QSoundEffect::Status status);
    void setLoopsRemaining(int loopsRemaining);

    static void stream_write_callback(pa_stream *s, size_t length, void *userdata);
    static void stream_state_callback(pa_stream *s, void *userdata);
    static void stream_underrun_callback(pa_stream *s, void *userdata);
    static void stream_cork_callback(pa_stream *s, int success, void *userdata);
    static void stream_flush_callback(pa_stream *s, int success, void *userdata);
    static void stream_flush_reload_callback(pa_stream *s, int success, void *userdata);
    static void stream_write_done_callback(void *p);
    static void stream_adjust_prebuffer_callback(pa_stream *s, int success, void *userdata);
    static void stream_reset_buffer_callback(pa_stream *s, int success, void *userdata);

    pa_stream *m_pulseStream;
    int m_sinkInputId;

    pa_sample_spec m_pulseSpec;
    int m_pulseBufferSize;

    bool m_emptying;
    bool m_sampleReady;
    bool m_playing;

    QSoundEffect::Status  m_status;

    bool m_muted;
    bool m_playQueued;
    bool m_stopping;
    qreal m_volume;
    int m_loopCount;
    int m_runningCount;

    QUrl m_source;
    QByteArray m_name;
    QString m_category;

    bool m_reloadCategory;

    QSample *m_sample;
    int m_position;
    QSoundEffectRef *m_ref;

    bool m_resourcesAvailable;

    mutable QReadWriteLock m_volumeLock;

    QMediaPlayerResourceSetInterface *m_resources;
};

#endif
