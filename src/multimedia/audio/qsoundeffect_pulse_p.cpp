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

#include <qsoundeffect_pulse_p.h>

#include <qaudioformat.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qtime.h>
#include <qtimer.h>

#include <qaudiohelpers_p.h>
#include <qmediaresourcepolicy_p.h>
#include <qmediaresourceset_p.h>

#include <unistd.h>

#ifndef QTM_PULSEAUDIO_DEFAULTBUFFER
#define QT_PA_STREAM_BUFFER_SIZE_MAX (1024 * 64)  //64KB is a trade-off for balancing control latency and uploading overhead
#endif

namespace
{
inline pa_sample_spec audioFormatToSampleSpec(const QAudioFormat &format)
{
    pa_sample_spec  spec;

    spec.rate = format.sampleRate();
    spec.channels = format.channelCount();

    if (format.sampleSize() == 8)
        spec.format = PA_SAMPLE_U8;

    else if (format.sampleSize() == 16) {
        switch (format.byteOrder()) {
            case QAudioFormat::BigEndian: spec.format = PA_SAMPLE_S16BE; break;
            case QAudioFormat::LittleEndian: spec.format = PA_SAMPLE_S16LE; break;
        }
    }

    else if (format.sampleSize() == 32) {
        switch (format.byteOrder()) {
            case QAudioFormat::BigEndian: spec.format = PA_SAMPLE_S32BE; break;
            case QAudioFormat::LittleEndian: spec.format = PA_SAMPLE_S32LE; break;
        }
    }

    return spec;
}

class PulseDaemon : public QObject
{
    MULTI_CS_OBJECT(PulseDaemon)

 public:
    PulseDaemon(): m_prepared(false) {
        prepare();
    }

    ~PulseDaemon() {
        if (m_prepared)
            release();
    }

    void lock() {
        if (m_mainLoop)
            pa_threaded_mainloop_lock(m_mainLoop);
    }

    void unlock() {
        if (m_mainLoop)
            pa_threaded_mainloop_unlock(m_mainLoop);
    }

    pa_context *context() const {
        return m_context;
    }

    MULTI_CS_SIGNAL_1(Public, void contextReady())
    MULTI_CS_SIGNAL_2(contextReady)

    MULTI_CS_SIGNAL_1(Public, void contextFailed())
    MULTI_CS_SIGNAL_2(contextFailed)

 private:
    MULTI_CS_SLOT_1(Private, void onContextFailed())
    MULTI_CS_SLOT_2(onContextFailed)

    MULTI_CS_SLOT_1(Private, void prepare())
    MULTI_CS_SLOT_2(prepare)

    void release() {
        if (!m_prepared) {
            return;
        }

        if (m_context) {
            pa_context_unref(m_context);
            m_context = nullptr;
        }

        if (m_mainLoop) {
            pa_threaded_mainloop_stop(m_mainLoop);
            pa_threaded_mainloop_free(m_mainLoop);
            m_mainLoop = nullptr;
        }

        m_prepared = false;
    }

    static void context_state_callback(pa_context *c, void *userdata)
    {
        PulseDaemon *self = reinterpret_cast<PulseDaemon*>(userdata);

        switch (pa_context_get_state(c)) {
            case PA_CONTEXT_CONNECTING:
            case PA_CONTEXT_AUTHORIZING:
            case PA_CONTEXT_SETTING_NAME:
                break;

            case PA_CONTEXT_READY:
                QMetaObject::invokeMethod(self, "contextReady", Qt::QueuedConnection);
                break;

            case PA_CONTEXT_FAILED:
                QMetaObject::invokeMethod(self, "onContextFailed", Qt::QueuedConnection);
                break;

            default:
                break;
        }
    }

    bool m_prepared;
    pa_context *m_context;
    pa_threaded_mainloop *m_mainLoop;
    pa_mainloop_api *m_mainLoopApi;
};

void PulseDaemon::onContextFailed()
{
  release();

  // Try to reconnect later
  QTimer::singleShot(30000, this, SLOT(prepare()));

  emit contextFailed();
}

void PulseDaemon::prepare()
{
  m_context  = nullptr;
  m_mainLoop = pa_threaded_mainloop_new();

  if (m_mainLoop == nullptr) {
      qWarning("PulseAudioService::prepare() Unable to create pulseaudio mainloop");
      return;
  }

  if (pa_threaded_mainloop_start(m_mainLoop) != 0) {
      qWarning("PulseAudioService::prepare() Unable to start pulseaudio mainloop");
      pa_threaded_mainloop_free(m_mainLoop);
      return;
  }

  m_mainLoopApi = pa_threaded_mainloop_get_api(m_mainLoop);

  lock();
  m_context = pa_context_new(m_mainLoopApi, QString("QtPulseAudio:%1").formatArg(::getpid()).toLatin1().constData());

  if (m_context == nullptr) {
      qWarning("PulseAudioService::prepare() Unable to create new pulseaudio context");
      pa_threaded_mainloop_unlock(m_mainLoop);
      pa_threaded_mainloop_free(m_mainLoop);
      m_mainLoop = nullptr;
      onContextFailed();

      return;
  }

  pa_context_set_state_callback(m_context, context_state_callback, this);

  if (pa_context_connect(m_context, nullptr, (pa_context_flags_t)0, nullptr) < 0) {
      qWarning("PulseAudioService::prepare() Unable to connect to pulseaudio context");
      pa_context_unref(m_context);
      pa_threaded_mainloop_unlock(m_mainLoop);
      pa_threaded_mainloop_free(m_mainLoop);
      m_mainLoop = nullptr;
      m_context  = nullptr;

      return;
  }

  unlock();

  m_prepared = true;
}

} // namespace

static PulseDaemon *pulseDaemon()
{
   static PulseDaemon retval;
   return &retval;
}

static QSampleCache *sampleCache()
{
   static QSampleCache retval;
   return &retval;
}

namespace {

class PulseDaemonLocker
{
 public:
    PulseDaemonLocker()
    {
        pulseDaemon()->lock();
    }

    ~PulseDaemonLocker()
    {
        pulseDaemon()->unlock();
    }
};

}

class QSoundEffectRef
{
 public:
    QSoundEffectRef(QSoundEffectPrivate *target)
        : m_ref(1), m_target(target)
    {

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
        qDebug() << "QSoundEffectRef(" << this << ") ctor";
#endif

    }

    QSoundEffectRef *getRef()
    {

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
        qDebug() << "QSoundEffectRef(" << this << ") getRef";
#endif

        QMutexLocker locker(&m_mutex);
        ++m_ref;

        return this;
    }

    void release()
    {

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
        qDebug() << "QSoundEffectRef(" << this << ") Release";
#endif

        m_mutex.lock();
        --m_ref;

        if (m_ref == 0) {
            m_mutex.unlock();

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
            qDebug() << "QSoundEffectRef(" << this << ") deleted";
#endif
            delete this;

            return;
        }

        m_mutex.unlock();
    }

    QSoundEffectPrivate* soundEffect() const
    {
        QMutexLocker locker(&m_mutex);
        return m_target;
    }

    void notifyDeleted()
    {
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
        qDebug() << "QSoundEffectRef(" << this << ") notifyDeleted";
#endif

        QMutexLocker locker(&m_mutex);
        m_target = nullptr;
    }

 private:
    int m_ref;
    mutable QMutex m_mutex;
    QSoundEffectPrivate *m_target;
};

QSoundEffectPrivate::QSoundEffectPrivate(QObject* parent)
   : QObject(parent), m_pulseStream(nullptr), m_sinkInputId(-1), m_emptying(false), m_sampleReady(false),
     m_playing(false), m_status(QSoundEffect::Null), m_muted(false), m_playQueued(false), m_stopping(false),
     m_volume(1.0), m_loopCount(1), m_runningCount(0), m_reloadCategory(false), m_sample(nullptr),
     m_position(0), m_resourcesAvailable(false)
{
    m_ref = new QSoundEffectRef(this);
    pa_sample_spec_init(&m_pulseSpec);

    m_resources = QMediaResourcePolicy::createResourceSet<QMediaPlayerResourceSetInterface>();
    Q_ASSERT(m_resources);

    m_resourcesAvailable = m_resources->isAvailable();

    connect(m_resources, &QMediaPlayerResourceSetInterface::availabilityChanged, this,
          &QSoundEffectPrivate::handleAvailabilityChanged);
}

void QSoundEffectPrivate::handleAvailabilityChanged(bool available)
{
    m_resourcesAvailable = available;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << Q_FUNC_INFO << "Resource availability changed " << m_resourcesAvailable;
#endif

    if (! m_resourcesAvailable) {
        stop();
    }
}

void QSoundEffectPrivate::release()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "release";
#endif

    m_ref->notifyDeleted();
    unloadPulseStream();

    if (m_sample) {
        m_sample->release();
        m_sample = nullptr;
    }

    this->deleteLater();
}

QString QSoundEffectPrivate::category() const
{
    return m_category;
}

void QSoundEffectPrivate::setCategory(const QString &category)
{
    if (m_category != category) {
        m_category = category;

        if (m_playing || m_playQueued) {
            // Currently playing, we need to disconnect when
            // playback stops
            m_reloadCategory = true;

        } else if (m_pulseStream) {
            // We have to disconnect and reconnect
            unloadPulseStream();
            createPulseStream();

        } else {
            // Well, next time we create the pulse stream
            // it should be set
        }

        emit categoryChanged();
    }
}

QSoundEffectPrivate::~QSoundEffectPrivate()
{
    QMediaResourcePolicy::destroyResourceSet(m_resources);
    m_resources = nullptr;
    m_ref->release();
}

QStringList QSoundEffectPrivate::supportedMimeTypes()
{
    QStringList supportedTypes;
    supportedTypes << "audio/x-wav" << "audio/vnd.wave";

    return supportedTypes;
}

QUrl QSoundEffectPrivate::source() const
{
    return m_source;
}

void QSoundEffectPrivate::setSource(const QUrl &url)
{
    Q_ASSERT(m_source != url);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "setSource =" << url;
#endif

    // Make sure the stream is empty before loading a new source (otherwise whatever is there will
    // be played before the new source)
    emptyStream();

    stop();

    if (m_sample) {
        if (! m_sampleReady) {
            disconnect(m_sample, &QSample::error, this, &QSoundEffectPrivate::decoderError);
            disconnect(m_sample, &QSample::ready, this, &QSoundEffectPrivate::sampleReady);
        }

        m_sample->release();
        m_sample = nullptr;
    }

    m_source = url;
    m_sampleReady = false;

    PulseDaemonLocker locker;
    setLoopsRemaining(0);

    if (m_pulseStream && ! pa_stream_is_corked(m_pulseStream)) {
        pa_stream_set_write_callback(m_pulseStream, nullptr, nullptr);
        pa_stream_set_underflow_callback(m_pulseStream, nullptr, nullptr);
        pa_operation *op = pa_stream_cork(m_pulseStream, 1, nullptr, nullptr);

        if (op) {
            pa_operation_unref(op);
        } else {
            qWarning("QSoundEffect::setSource() Failed to cork stream");
        }
    }

    setPlaying(false);

    if (url.isEmpty()) {
        setStatus(QSoundEffect::Null);
        return;
    }

    setStatus(QSoundEffect::Loading);
    m_sample = sampleCache()->requestSample(url);

    connect(m_sample, &QSample::error, this, &QSoundEffectPrivate::decoderError);
    connect(m_sample, &QSample::ready, this, &QSoundEffectPrivate::sampleReady);

    switch(m_sample->state()) {
       case QSample::Ready:
           sampleReady();
           break;

       case QSample::Error:
           decoderError();
           break;

       default:
           break;
    }
}

int QSoundEffectPrivate::loopCount() const
{
    return m_loopCount;
}

int QSoundEffectPrivate::loopsRemaining() const
{
    return m_runningCount;
}

void QSoundEffectPrivate::setLoopCount(int loopCount)
{
    if (loopCount == 0) {
        loopCount = 1;
    }

    m_loopCount = loopCount;

    if (m_playing) {
        setLoopsRemaining(loopCount);
    }
}

qreal QSoundEffectPrivate::volume() const
{
    QReadLocker locker(&m_volumeLock);
    return m_volume;
}

void QSoundEffectPrivate::setVolume(qreal volume)
{
    QWriteLocker locker(&m_volumeLock);

    if (qFuzzyCompare(m_volume, volume))
        return;

    m_volume = qBound(qreal(0), volume, qreal(1));
    emit volumeChanged();
}

bool QSoundEffectPrivate::isMuted() const
{
    QReadLocker locker(&m_volumeLock);
    return m_muted;
}

void QSoundEffectPrivate::setMuted(bool muted)
{
    QWriteLocker locker(&m_volumeLock);
    m_muted = muted;
    emit mutedChanged();
}

bool QSoundEffectPrivate::isLoaded() const
{
    return m_status == QSoundEffect::Ready;
}

bool QSoundEffectPrivate::isPlaying() const
{
    return m_playing;
}

QSoundEffect::Status QSoundEffectPrivate::status() const
{
    return m_status;
}

void QSoundEffectPrivate::setPlaying(bool playing)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "setPlaying(" << playing << ")";
#endif

    if (m_playing == playing) {
        return;
    }

    if (! playing) {
        m_playQueued = false;
    }

    m_playing = playing;
    emit playingChanged();
}

void QSoundEffectPrivate::setStatus(QSoundEffect::Status status)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "setStatus" << status;
#endif

    if (m_status == status) {
        return;
    }

    bool oldLoaded = isLoaded();
    m_status = status;

    emit statusChanged();

    if (oldLoaded != isLoaded()) {
        emit loadedChanged();
   }
}

void QSoundEffectPrivate::setLoopsRemaining(int loopsRemaining)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "setLoopsRemaining " << loopsRemaining;
#endif

    if (m_runningCount == loopsRemaining) {
        return;
    }

    m_runningCount = loopsRemaining;
    emit loopsRemainingChanged();
}

void QSoundEffectPrivate::play()
{
    if (! m_resourcesAvailable) {
        return;
    }

    playAvailable();
}

void QSoundEffectPrivate::playAvailable()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "play";
#endif

    if (m_status == QSoundEffect::Null || m_status == QSoundEffect::Error || m_playQueued) {
        return;
    }

    PulseDaemonLocker locker;

    if (! m_pulseStream || m_status != QSoundEffect::Ready || m_stopping || m_emptying) {

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
        qDebug() << this << "play deferred";
#endif
        m_playQueued = true;

    } else {
        if (m_playing) {
          //restart playing from the beginning

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
           qDebug() << this << "restart playing";
#endif

            setLoopsRemaining(0);
            m_playQueued = true;
            Q_ASSERT(m_pulseStream);
            emptyStream(ReloadSampleWhenDone);

            return;
        }

        setLoopsRemaining(m_loopCount);
        playSample();
    }

    setPlaying(true);
}

void QSoundEffectPrivate::emptyStream(EmptyStreamOptions options)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "emptyStream";
#endif

    if (! m_pulseStream || m_emptying) {
        return;
    }

    const bool reloadSample = options.testFlag(ReloadSampleWhenDone);
    pa_stream_success_cb_t flushCompleteCb = reloadSample ? stream_flush_reload_callback  : stream_flush_callback;

    m_emptying = true;
    pa_stream_set_write_callback(m_pulseStream, nullptr, nullptr);
    pa_stream_set_underflow_callback(m_pulseStream, nullptr, nullptr);
    pa_operation *op = pa_stream_flush(m_pulseStream, flushCompleteCb, m_ref->getRef());

    if (op) {
        pa_operation_unref(op);
    } else {
        qWarning("QSoundEffect::emptyStream() Failed to flush stream");
    }
}

void QSoundEffectPrivate::emptyComplete(void *stream, bool reload)
{
    PulseDaemonLocker locker;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "emptyComplete";
#endif

    m_emptying = false;

    if ((pa_stream *)stream == m_pulseStream) {
        pa_operation *op = pa_stream_cork(m_pulseStream, 1, reload ?
                  stream_cork_callback : nullptr, m_ref->getRef());

        if (op) {
            pa_operation_unref(op);
        } else {
            qWarning("QSoundEffect::emptyComplete() Failed to cork stream");
        }
    }
}

void QSoundEffectPrivate::sampleReady()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "sampleReady";
#endif

    disconnect(m_sample, &QSample::error, this, &QSoundEffectPrivate::decoderError);
    disconnect(m_sample, &QSample::ready, this, &QSoundEffectPrivate::sampleReady);

    pa_sample_spec newFormatSpec = audioFormatToSampleSpec(m_sample->format());

    if (m_pulseStream && ! pa_sample_spec_equal(&m_pulseSpec, &newFormatSpec)) {
        unloadPulseStream();
    }
    m_pulseSpec = newFormatSpec;

    m_sampleReady = true;
    m_position = 0;

    if (m_name.isNull()) {
        m_name = QString("QtPulseSample-%1-%2").formatArg(::getpid()).formatArg(quintptr(this)).toUtf8();
    }

    PulseDaemonLocker locker;

    if (m_pulseStream) {

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
        qDebug() << this << "reuse existing pulsestream";
#endif

#ifdef QTM_PULSEAUDIO_DEFAULTBUFFER
        const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(m_pulseStream);

        if (bufferAttr->prebuf > uint32_t(m_sample->data().size())) {
            pa_buffer_attr newBufferAttr;
            newBufferAttr = *bufferAttr;
            newBufferAttr.prebuf = m_sample->data().size();

            pa_operation *op = pa_stream_set_buffer_attr(m_pulseStream, &newBufferAttr,
                  stream_adjust_prebuffer_callback, m_ref->getRef());

            if (op) {
                pa_operation_unref(op);
            } else {
                qWarning("QSoundEffect::sampleReady() Failed to adjust pre-buffer attribute");
            }

        } else {
            streamReady();
        }
#else
        const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(m_pulseStream);

        if (bufferAttr->tlength < m_sample->data().size() && bufferAttr->tlength < QT_PA_STREAM_BUFFER_SIZE_MAX) {
            pa_buffer_attr newBufferAttr;
            newBufferAttr.maxlength = -1;
            newBufferAttr.tlength = qMin(m_sample->data().size(), QT_PA_STREAM_BUFFER_SIZE_MAX);
            newBufferAttr.minreq = bufferAttr->tlength / 2;
            newBufferAttr.prebuf = -1;
            newBufferAttr.fragsize = -1;

            pa_operation *op = pa_stream_set_buffer_attr(m_pulseStream, &newBufferAttr,
                  stream_reset_buffer_callback, m_ref->getRef());

            if (op) {
                pa_operation_unref(op);
            } else {
                qWarning("QSoundEffect::sampleReady() Failed to adjust pre-buffer attribute");
            }

        } else if (bufferAttr->prebuf > uint32_t(m_sample->data().size())) {
            pa_buffer_attr newBufferAttr;
            newBufferAttr = *bufferAttr;
            newBufferAttr.prebuf = m_sample->data().size();

            pa_operation *op = pa_stream_set_buffer_attr(m_pulseStream, &newBufferAttr,
                  stream_adjust_prebuffer_callback, m_ref->getRef());

            if (op) {
                pa_operation_unref(op);
            } else {
                qWarning("QSoundEffect::sampleReady() Failed to adjust pre-buffer attribute");
            }

        } else {
            streamReady();
        }
#endif

    } else {
        if (! pulseDaemon()->context() || pa_context_get_state(pulseDaemon()->context()) != PA_CONTEXT_READY) {
            connect(pulseDaemon(), &PulseDaemon::contextReady, this, &QSoundEffectPrivate::contextReady);
            return;
        }

        createPulseStream();
    }
}

void QSoundEffectPrivate::decoderError()
{
    qWarning("QSoundEffect::decoderError() Error decoding source");
    disconnect(m_sample, &QSample::error, this, &QSoundEffectPrivate::decoderError);

    bool playingDirty = false;

    if (m_playing) {
        m_playing = false;
        playingDirty = true;
    }

    setStatus(QSoundEffect::Error);

    if (playingDirty) {
        emit playingChanged();
    }
}

void QSoundEffectPrivate::unloadPulseStream()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "unloadPulseStream";
#endif

    m_sinkInputId = -1;
    PulseDaemonLocker locker;

    if (m_pulseStream) {
        pa_stream_set_state_callback(m_pulseStream, nullptr, nullptr);
        pa_stream_set_write_callback(m_pulseStream, nullptr, nullptr);
        pa_stream_set_underflow_callback(m_pulseStream, nullptr, nullptr);
        pa_stream_disconnect(m_pulseStream);
        pa_stream_unref(m_pulseStream);

        disconnect(pulseDaemon(), &PulseDaemon::contextFailed, this, &QSoundEffectPrivate::contextFailed);

        m_pulseStream = nullptr;
        m_reloadCategory = false; // category will be reloaded when we connect anyway
    }
}

void QSoundEffectPrivate::prepare()
{
    if (! m_pulseStream || !m_sampleReady) {
        return;
    }

    PulseDaemonLocker locker;

    if (pa_stream_get_state(m_pulseStream) != PA_STREAM_READY) {
        return;
    }

    pa_stream_set_write_callback(m_pulseStream, stream_write_callback, this);
    pa_stream_set_underflow_callback(m_pulseStream, stream_underrun_callback, this);

    m_stopping = false;
    size_t writeBytes = size_t(qMin(m_pulseBufferSize, m_sample->data().size()));

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "prepare(): writable size =" << pa_stream_writable_size(m_pulseStream)
             << "actual writeBytes =" << writeBytes
             << "m_playQueued =" << m_playQueued;
#endif

    m_position = writeToStream(m_sample->data().data(), writeBytes);

    if (m_playQueued) {
        m_playQueued = false;
        setLoopsRemaining(m_loopCount);
        playSample();
    }
}

void QSoundEffectPrivate::uploadSample()
{
    if (m_runningCount == 0) {

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "uploadSample: return due to 0 m_runningCount";
#endif
        return;
    }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "uploadSample: m_runningCount =" << m_runningCount;
#endif

    if (m_position == m_sample->data().size()) {
        m_position = 0;

        if (m_runningCount > 0) {
            setLoopsRemaining(m_runningCount - 1);
        }

        if (m_runningCount == 0) {
            return;
        }
    }

    int writableSize    = int(pa_stream_writable_size(m_pulseStream));
    int firstPartLength = qMin(m_sample->data().size() - m_position, writableSize);
    int writtenBytes    = writeToStream(m_sample->data().data() + m_position, firstPartLength);

    m_position += writtenBytes;
    if (m_position == m_sample->data().size()) {
        m_position = 0;

        if (m_runningCount > 0) {
            setLoopsRemaining(m_runningCount - 1);
        }

        if (m_runningCount != 0 && firstPartLength < writableSize) {
            while (writtenBytes < writableSize) {
                int writeSize = qMin(writableSize - writtenBytes, m_sample->data().size());
                writtenBytes += writeToStream(m_sample->data().data(), writeSize);

                if (writeSize < m_sample->data().size()) {
                    m_position = writeSize;
                    break;
                }

                if (m_runningCount > 0) {
                    setLoopsRemaining(m_runningCount - 1);
                }

                if (m_runningCount == 0) {
                    break;
                }
            }
        }
    }
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "uploadSample: use direct write, writeable size =" << writableSize
             << "actual writtenBytes =" << writtenBytes;
#endif
}

int QSoundEffectPrivate::writeToStream(const void *data, int size)
{
    m_volumeLock.lockForRead();
    qreal volume = m_muted ? 0 : m_volume;
    m_volumeLock.unlock();
    pa_free_cb_t writeDoneCb = stream_write_done_callback;

    if (volume < 1.0f) {
        // Don't use PulseAudio volume, as it might affect all other streams of the same category
        // or even affect the system volume if flat volumes are enabled
        void *dest    = nullptr;
        size_t nbytes = size;

        if (pa_stream_begin_write(m_pulseStream, &dest, &nbytes) < 0) {
            qWarning("QSoundEffect::writeToStream() Pulseaudio stream begin write error = %s",
                  pa_strerror(pa_context_errno(pulseDaemon()->context())));

            return 0;
        }

        size = int(nbytes);
        QAudioHelperInternal::qMultiplySamples(volume, m_sample->format(), data, dest, size);
        data = dest;
        writeDoneCb = nullptr;
    }

    if (pa_stream_write(m_pulseStream, data, size, writeDoneCb, 0, PA_SEEK_RELATIVE) < 0) {
        qWarning("QSoundEffect::writeToStream() Pulseaudio stream write error = %s",
              pa_strerror(pa_context_errno(pulseDaemon()->context())));

        return 0;
    }

    return size;
}

void QSoundEffectPrivate::playSample()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "playSample";
#endif

    Q_ASSERT(m_pulseStream);
    Q_ASSERT(pa_stream_get_state(m_pulseStream) == PA_STREAM_READY);

    pa_operation *o = pa_stream_cork(m_pulseStream, 0, nullptr, nullptr);

    if (o) {
        pa_operation_unref(o);
   }
}

void QSoundEffectPrivate::stop()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "stop";
#endif

    if (! m_playing) {
        return;
    }

    setPlaying(false);
    PulseDaemonLocker locker;
    m_stopping = true;

    if (m_pulseStream) {
        emptyStream(ReloadSampleWhenDone);

        if (m_reloadCategory) {
            // upon play we reconnect anyway
            unloadPulseStream();
        }
    }

    setLoopsRemaining(0);
    m_position = 0;
    m_playQueued = false;
    m_reloadCategory = false;
}

void QSoundEffectPrivate::underRun()
{
    stop();
}

void QSoundEffectPrivate::streamReady()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "streamReady";
#endif

    PulseDaemonLocker locker;
    m_sinkInputId =  pa_stream_get_index(m_pulseStream);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    const pa_buffer_attr *realBufAttr = pa_stream_get_buffer_attr(m_pulseStream);

    qDebug() << this << "m_sinkInputId =" << m_sinkInputId
             << "tlength =" << realBufAttr->tlength << "maxlength =" << realBufAttr->maxlength
             << "minreq = " << realBufAttr->minreq << "prebuf =" << realBufAttr->prebuf;
#endif

    prepare();
    setStatus(QSoundEffect::Ready);
}

void QSoundEffectPrivate::createPulseStream()
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << this << "createPulseStream";
#endif

    if (! pulseDaemon()->context())
        return;

    pa_proplist *propList = pa_proplist_new();

    if (! m_category.isEmpty()) {
        pa_proplist_sets(propList, PA_PROP_MEDIA_ROLE, m_category.toLatin1().constData());
   }

    pa_stream *stream = pa_stream_new_with_proplist(pulseDaemon()->context(), m_name.constData(),
          &m_pulseSpec, nullptr, propList);

    pa_proplist_free(propList);

    connect(pulseDaemon(), &PulseDaemon::contextFailed, this, &QSoundEffectPrivate::contextFailed);

    if (stream == nullptr) {
        qWarning("QSoundEffect::createPulseStream() Failed to create stream");
        m_pulseStream = nullptr;
        setStatus(QSoundEffect::Error);
        setPlaying(false);
        return;

    } else {
        pa_stream_set_state_callback(stream, stream_state_callback, this);
        pa_stream_set_write_callback(stream, stream_write_callback, this);
        pa_stream_set_underflow_callback(stream, stream_underrun_callback, this);
    }

    m_pulseStream = stream;

#ifndef QTM_PULSEAUDIO_DEFAULTBUFFER
    pa_buffer_attr bufferAttr;
    bufferAttr.tlength = qMin(m_sample->data().size(), QT_PA_STREAM_BUFFER_SIZE_MAX);
    bufferAttr.maxlength = -1;
    bufferAttr.minreq = bufferAttr.tlength / 2;
    bufferAttr.prebuf = -1;
    bufferAttr.fragsize = -1;

    if (pa_stream_connect_playback(m_pulseStream, nullptr, &bufferAttr, PA_STREAM_START_CORKED, nullptr, nullptr) < 0) {

#else
    if (pa_stream_connect_playback(m_pulseStream, nullptr, nullptr, PA_STREAM_START_CORKED, nullptr, nullptr) < 0) {

#endif
        qWarning("QSoundEffect::createPulseStream() Failed to connect stream, error = %s",
              pa_strerror(pa_context_errno(pulseDaemon()->context())));
    }
}

void QSoundEffectPrivate::contextReady()
{
    disconnect(pulseDaemon(), &PulseDaemon::contextReady, this, &QSoundEffectPrivate::contextReady);
    PulseDaemonLocker locker;
    createPulseStream();
}

void QSoundEffectPrivate::contextFailed()
{
    unloadPulseStream();
    connect(pulseDaemon(), &PulseDaemon::contextReady, this, &QSoundEffectPrivate::contextReady);
}

void QSoundEffectPrivate::stream_write_callback(pa_stream *s, size_t length, void *userdata)
{
    (void) length;
    (void) s;

    QSoundEffectPrivate *self = reinterpret_cast<QSoundEffectPrivate*>(userdata);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << self << "stream_write_callback";
#endif

    self->uploadSample();
}

void QSoundEffectPrivate::stream_state_callback(pa_stream *s, void *userdata)
{
    QSoundEffectPrivate *self = reinterpret_cast<QSoundEffectPrivate*>(userdata);

    switch (pa_stream_get_state(s)) {
        case PA_STREAM_READY:
        {
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
            qDebug() << self << "pulse stream ready";
#endif
            const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(self->m_pulseStream);
            self->m_pulseBufferSize = bufferAttr->tlength;

            if (bufferAttr->prebuf > uint32_t(self->m_sample->data().size())) {
                pa_buffer_attr newBufferAttr;
                newBufferAttr = *bufferAttr;
                newBufferAttr.prebuf = self->m_sample->data().size();

                pa_operation *op = pa_stream_set_buffer_attr(self->m_pulseStream, &newBufferAttr,
                     stream_adjust_prebuffer_callback, self->m_ref->getRef());

                if (op) {
                    pa_operation_unref(op);
                } else {
                    qWarning("QSoundEffect::createPulseStream() Failed to adjust pre-buffer attribute");
                }

            } else {
                QMetaObject::invokeMethod(self, "streamReady", Qt::QueuedConnection);
            }
            break;
        }

        case PA_STREAM_CREATING:
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
            qDebug() << self << "pulse stream creating";
#endif
            break;

        case PA_STREAM_TERMINATED:
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
            qDebug() << self << "pulse stream terminated";
#endif
            break;

        case PA_STREAM_FAILED:
        default:
            qWarning("QSoundEffect::createPulseStream() Error in pulse audio stream");
            break;
    }
}

void QSoundEffectPrivate::stream_reset_buffer_callback(pa_stream *s, int success, void *userdata)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << "stream_reset_buffer_callback";
#endif

    (void) s;

    QSoundEffectRef *ref = reinterpret_cast<QSoundEffectRef*>(userdata);
    QSoundEffectPrivate *self = ref->soundEffect();

    ref->release();

    if (! self) {
        return;
    }

    if (! success)
        qWarning("QSoundEffect::createPulseStream() Failed to reset buffer attribute");

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << self << "stream_reset_buffer_callback";
#endif

    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(self->m_pulseStream);
    self->m_pulseBufferSize = bufferAttr->tlength;

    if (bufferAttr->prebuf > uint32_t(self->m_sample->data().size())) {
        pa_buffer_attr newBufferAttr;
        newBufferAttr = *bufferAttr;
        newBufferAttr.prebuf = self->m_sample->data().size();

        pa_operation *op = pa_stream_set_buffer_attr(self->m_pulseStream,
              &newBufferAttr, stream_adjust_prebuffer_callback, userdata);

        if (op) {
            pa_operation_unref(op);
        } else {
            qWarning("QSoundEffect::createPulseStream() Failed to adjust pre-buffer attribute");
        }

    } else {
        QMetaObject::invokeMethod(self, "streamReady", Qt::QueuedConnection);
    }
}

void QSoundEffectPrivate::stream_adjust_prebuffer_callback(pa_stream *s, int success, void *userdata)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << "stream_adjust_prebuffer_callback";
#endif

    (void) s;

    QSoundEffectRef *ref = reinterpret_cast<QSoundEffectRef*>(userdata);
    QSoundEffectPrivate *self = ref->soundEffect();
    ref->release();

    if (! self)  {
        return;
    }

    if (! success) {
        qWarning("QSoundEffect::stream_adjust_prebuffer_callback() Failed to adjust pre-buffer attribute");
    }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << self << "stream_adjust_prebuffer_callback";
#endif

    QMetaObject::invokeMethod(self, "streamReady", Qt::QueuedConnection);
}

void QSoundEffectPrivate::stream_underrun_callback(pa_stream *s, void *userdata)
{
   (void) s;

   QSoundEffectPrivate *self = reinterpret_cast<QSoundEffectPrivate*>(userdata);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << self << "stream_underrun_callback";
#endif

   if (self->m_runningCount == 0 && ! self->m_playQueued) {
      QMetaObject::invokeMethod(self, "underRun", Qt::QueuedConnection);
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   else {
      qDebug() << "underun corked =" << pa_stream_is_corked(s);
   }
#endif
}

void QSoundEffectPrivate::stream_cork_callback(pa_stream *s, int success, void *userdata)
{
   (void) s;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << "stream_cork_callback";
#endif

    QSoundEffectRef *ref = reinterpret_cast<QSoundEffectRef*>(userdata);
    QSoundEffectPrivate *self = ref->soundEffect();
    ref->release();

    if (! self) {
        return;
    }

    if (! success) {
        qWarning("QSoundEffect::stream_cork_callback() Failed to stop");
    }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << self << "stream_cork_callback";
#endif

    QMetaObject::invokeMethod(self, "prepare", Qt::QueuedConnection);
}

void QSoundEffectPrivate::stream_flush_callback(pa_stream *s, int success, void *userdata)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << "stream_flush_callback";
#endif

    QSoundEffectRef *ref = reinterpret_cast<QSoundEffectRef*>(userdata);
    QSoundEffectPrivate *self = ref->soundEffect();
    ref->release();

    if (! self) {
        return;
    }

    if (! success) {
        qWarning("QSoundEffect::stream_flush_callback() Failed to drain");
    }

    QMetaObject::invokeMethod(self, "emptyComplete", Qt::QueuedConnection, Q_ARG(void*, s), Q_ARG(bool, false));
}

void QSoundEffectPrivate::stream_flush_reload_callback(pa_stream *s, int success, void *userdata)
{
#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << "stream_flush_reload_callback";
#endif

    QSoundEffectRef *ref = reinterpret_cast<QSoundEffectRef*>(userdata);
    QSoundEffectPrivate *self = ref->soundEffect();

    ref->release();
    if (! self) {
        return;
    }

    if (! success) {
        qWarning("QSoundEffect::stream_flush_reload_callback() Failed to drain");
    }

    QMetaObject::invokeMethod(self, "emptyComplete", Qt::QueuedConnection, Q_ARG(void*, s), Q_ARG(bool, true));
}

void QSoundEffectPrivate::stream_write_done_callback(void *p)
{
   (void) p;

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
    qDebug() << "stream_write_done_callback";
#endif
}
