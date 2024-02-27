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

#include <dshow.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "directshowplayerservice.h"

#include "directshowaudioendpointcontrol.h"
#include "directshowmetadatacontrol.h"
#include "vmr9videowindowcontrol.h"
#include "directshowiosource.h"
#include "directshowplayercontrol.h"
#include "directshowvideorenderercontrol.h"

#if defined(HAVE_EVR)
#include "directshowevrvideowindowcontrol.h"
#endif

#if defined(QT_USE_WMSDK)
#include <wmsdk.h>
#endif

#include <qmediacontent.h>

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qthread.h>
#include <qvarlengtharray.h>

static DirectShowEventLoop *qt_directShowEventLoop()
{
   static DirectShowEventLoop retval;
   return &retval;
}

// QMediaPlayer uses millisecond time units, direct show uses 100 nanosecond units.
static const int qt_directShowTimeScale = 10000;

class DirectShowPlayerServiceThread : public QThread
{
 public:
   DirectShowPlayerServiceThread(DirectShowPlayerService *service)
      : m_service(service)
   { }

 protected:
   void run() override {
      m_service->run();
   }

 private:
   DirectShowPlayerService *m_service;
};

DirectShowPlayerService::DirectShowPlayerService(QObject *parent)
   : QMediaService(parent)
   , m_playerControl(nullptr)
   , m_metaDataControl(nullptr)
   , m_videoRendererControl(nullptr)
   , m_videoWindowControl(nullptr)
   , m_audioEndpointControl(nullptr)
   , m_taskThread(nullptr)
   , m_loop(qt_directShowEventLoop())
   , m_pendingTasks(0)
   , m_executingTask(0)
   , m_executedTasks(0)
   , m_taskHandle(::CreateEvent(nullptr, 0, 0, nullptr))
   , m_eventHandle(nullptr)
   , m_graphStatus(NoMedia)
   , m_stream(nullptr)
   , m_graph(nullptr)
   , m_source(nullptr)
   , m_audioOutput(nullptr)
   , m_videoOutput(nullptr)
   , m_rate(1.0)
   , m_position(0)
   , m_seekPosition(-1)
   , m_duration(0)
   , m_buffering(false)
   , m_seekable(false)
   , m_atEnd(false)
   , m_dontCacheNextSeekResult(false)
{
   m_playerControl        = new DirectShowPlayerControl(this);
   m_metaDataControl      = new DirectShowMetaDataControl(this);
   m_audioEndpointControl = new DirectShowAudioEndpointControl(this);

   m_taskThread = new DirectShowPlayerServiceThread(this);
   m_taskThread->start();
}

DirectShowPlayerService::~DirectShowPlayerService()
{
   {
      QMutexLocker locker(&m_mutex);
      releaseGraph();

      m_pendingTasks = Shutdown;
      ::SetEvent(m_taskHandle);
   }

   m_taskThread->wait();
   delete m_taskThread;

   if (m_audioOutput) {
      m_audioOutput->Release();
      m_audioOutput = nullptr;
   }

   if (m_videoOutput) {
      m_videoOutput->Release();
      m_videoOutput = nullptr;
   }

   delete m_playerControl;
   delete m_audioEndpointControl;
   delete m_metaDataControl;
   delete m_videoRendererControl;
   delete m_videoWindowControl;

   ::CloseHandle(m_taskHandle);
}

QMediaControl *DirectShowPlayerService::requestControl(const QString &name)
{
   if (name == QMediaPlayerControl_Key) {
      return m_playerControl;

   } else if (name == QAudioOutputSelectorControl_iid) {
      return m_audioEndpointControl;

   } else if (name == QMetaDataReaderControl_iid) {
      return m_metaDataControl;

   } else if (name == QVideoRendererControl_iid) {
      if (! m_videoRendererControl && ! m_videoWindowControl) {
         m_videoRendererControl = new DirectShowVideoRendererControl(m_loop);

         connect(m_videoRendererControl, SIGNAL(filterChanged()), this, SLOT(videoOutputChanged()));

         return m_videoRendererControl;
      }

   } else if (name == QVideoWindowControl_iid) {
      if (! m_videoRendererControl && ! m_videoWindowControl) {
         IBaseFilter *filter = nullptr;

#ifdef HAVE_EVR
         DirectShowEvrVideoWindowControl *evrControl = new DirectShowEvrVideoWindowControl;

         if ((filter = evrControl->filter())) {
            m_videoWindowControl = evrControl;
         } else {
            delete evrControl;
         }
#endif
         // Fall back to the VMR9 if the EVR is not available
         if (! m_videoWindowControl) {
            Vmr9VideoWindowControl *vmr9Control = new Vmr9VideoWindowControl;
            filter = vmr9Control->filter();
            m_videoWindowControl = vmr9Control;
         }

         setVideoOutput(filter);

         return m_videoWindowControl;
      }

   }

   return nullptr;
}

void DirectShowPlayerService::releaseControl(QMediaControl *control)
{
   if (! control) {
      qWarning("QMediaService::releaseControl(): Attempted release of null control");

   } else if (control == m_videoRendererControl) {
      setVideoOutput(nullptr);

      delete m_videoRendererControl;
      m_videoRendererControl = nullptr;

   } else if (control == m_videoWindowControl) {
      setVideoOutput(nullptr);

      delete m_videoWindowControl;
      m_videoWindowControl = nullptr;

   }
}

void DirectShowPlayerService::load(const QMediaContent &media, QIODevice *stream)
{
   QMutexLocker locker(&m_mutex);

   m_pendingTasks = 0;

   if (m_graph) {
      releaseGraph();
   }

   m_resources = media.resources();
   m_stream = stream;
   m_error = QMediaPlayer::NoError;
   m_errorString = QString();
   m_position = 0;
   m_seekPosition = -1;
   m_duration = 0;
   m_streamTypes = 0;
   m_executedTasks = 0;
   m_buffering = false;
   m_seekable = false;
   m_atEnd = false;
   m_dontCacheNextSeekResult = false;
   m_metaDataControl->reset();

   if (m_resources.isEmpty() && !stream) {
      m_pendingTasks = 0;
      m_graphStatus = NoMedia;

      m_url.clear();
   } else if (stream && (!stream->isReadable() || stream->isSequential())) {
      m_pendingTasks = 0;
      m_graphStatus = InvalidMedia;
      m_error = QMediaPlayer::ResourceError;
   } else {
      // {36b73882-c2c8-11cf-8b46-00805f6cef60}
      static const GUID iid_IFilterGraph2 = {
         0x36b73882, 0xc2c8, 0x11cf, {0x8b, 0x46, 0x00, 0x80, 0x5f, 0x6c, 0xef, 0x60}
      };
      m_graphStatus = Loading;

      m_graph = com_new<IFilterGraph2>(CLSID_FilterGraph, iid_IFilterGraph2);

      if (stream) {
         m_pendingTasks = SetStreamSource;
      } else {
         m_pendingTasks = SetUrlSource;
      }

      ::SetEvent(m_taskHandle);
   }

   m_playerControl->updateError(m_error, m_errorString);
   m_playerControl->updateMediaInfo(m_duration, m_streamTypes, m_seekable);
   m_playerControl->updateState(QMediaPlayer::StoppedState);
   m_playerControl->updatePosition(m_position);
   updateStatus();
}

void DirectShowPlayerService::doSetUrlSource(QMutexLocker *locker)
{
   IBaseFilter *source = nullptr;

   QMediaResource resource = m_resources.takeFirst();
   m_url = resource.url();

   HRESULT hr = E_FAIL;
   if (m_url.scheme() == QLatin1String("http") || m_url.scheme() == "https") {
      static const GUID clsid_WMAsfReader = {
         0x187463a0, 0x5bb7, 0x11d3, {0xac, 0xbe, 0x00, 0x80, 0xc7, 0x5e, 0x24, 0x6e}
      };

      // {56a868a6-0ad4-11ce-b03a-0020af0ba770}
      static const GUID iid_IFileSourceFilter = {
         0x56a868a6, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}
      };

      if (IFileSourceFilter *fileSource = com_new<IFileSourceFilter>(clsid_WMAsfReader, iid_IFileSourceFilter)) {
         locker->unlock();

         std::wstring tmp = m_url.toString().toStdWString();
         hr = fileSource->Load(tmp.data(), nullptr);

         if (SUCCEEDED(hr)) {
            source = com_cast<IBaseFilter>(fileSource, IID_IBaseFilter);

            if (! SUCCEEDED(hr = m_graph->AddFilter(source, L"Source")) && source) {
               source->Release();
               source = nullptr;
            }
         }

         fileSource->Release();
         locker->relock();
      }
   }

   if (!SUCCEEDED(hr)) {
      locker->unlock();
      const QString urlString = m_url.isLocalFile()
         ? QDir::toNativeSeparators(m_url.toLocalFile()) : m_url.toString();

      std::wstring tmp = urlString.toStdWString();

      hr = m_graph->AddSourceFilter(tmp.data(), L"Source", &source);
      locker->relock();
   }

   if (SUCCEEDED(hr)) {
      m_executedTasks = SetSource;
      m_pendingTasks |= Render;

      if (m_audioOutput) {
         m_pendingTasks |= SetAudioOutput;
      }

      if (m_videoOutput) {
         m_pendingTasks |= SetVideoOutput;
      }

      if (m_rate != 1.0) {
         m_pendingTasks |= SetRate;
      }

      m_source = source;

   } else if (!m_resources.isEmpty()) {
      m_pendingTasks |= SetUrlSource;

   } else {
      m_pendingTasks = 0;
      m_graphStatus = InvalidMedia;

      switch (hr) {
         case VFW_E_UNKNOWN_FILE_TYPE:
            m_error = QMediaPlayer::FormatError;
            m_errorString = QString();
            break;

         case E_FAIL:
         case E_OUTOFMEMORY:
         case VFW_E_CANNOT_LOAD_SOURCE_FILTER:
         case VFW_E_NOT_FOUND:
            m_error = QMediaPlayer::ResourceError;
            m_errorString = QString();
            break;

         default:
            m_error = QMediaPlayer::ResourceError;
            m_errorString = QString();
            qWarning("DirectShowPlayerService::doSetUrlSource: Unresolved error code %x", uint(hr));
            break;
      }

      QCoreApplication::postEvent(this, new QEvent(QEvent::Type(Error)));
   }
}

void DirectShowPlayerService::doSetStreamSource(QMutexLocker *locker)
{
   (void) locker;

   DirectShowIOSource *source = new DirectShowIOSource(m_loop);
   source->setDevice(m_stream);

   if (SUCCEEDED(m_graph->AddFilter(source, L"Source"))) {
      m_executedTasks = SetSource;
      m_pendingTasks |= Render;

      if (m_audioOutput) {
         m_pendingTasks |= SetAudioOutput;
      }
      if (m_videoOutput) {
         m_pendingTasks |= SetVideoOutput;
      }

      if (m_rate != 1.0) {
         m_pendingTasks |= SetRate;
      }

      m_source = source;
   } else {
      source->Release();

      m_pendingTasks = 0;
      m_graphStatus = InvalidMedia;

      m_error = QMediaPlayer::ResourceError;
      m_errorString = QString();

      QCoreApplication::postEvent(this, new QEvent(QEvent::Type(Error)));
   }
}

void DirectShowPlayerService::doRender(QMutexLocker *locker)
{
   m_pendingTasks |= m_executedTasks & (Play | Pause);

   if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
      control->Stop();
      control->Release();
   }

   if (m_pendingTasks & SetAudioOutput) {
      m_graph->AddFilter(m_audioOutput, L"AudioOutput");

      m_pendingTasks ^= SetAudioOutput;
      m_executedTasks |= SetAudioOutput;
   }
   if (m_pendingTasks & SetVideoOutput) {
      m_graph->AddFilter(m_videoOutput, L"VideoOutput");

      m_pendingTasks ^= SetVideoOutput;
      m_executedTasks |= SetVideoOutput;
   }

   IFilterGraph2 *graph = m_graph;
   graph->AddRef();

   QVarLengthArray<IBaseFilter *, 16> filters;
   m_source->AddRef();
   filters.append(m_source);

   bool rendered = false;

   HRESULT renderHr = S_OK;

   while (!filters.isEmpty()) {
      IEnumPins *pins = nullptr;
      IBaseFilter *filter = filters[filters.size() - 1];
      filters.removeLast();

      if (!(m_pendingTasks & ReleaseFilters) && SUCCEEDED(filter->EnumPins(&pins))) {
         int outputs = 0;

         for (IPin *pin = nullptr; pins->Next(1, &pin, nullptr) == S_OK; pin->Release()) {
            PIN_DIRECTION direction;
            if (pin->QueryDirection(&direction) == S_OK && direction == PINDIR_OUTPUT) {
               ++outputs;

               IPin *peer = nullptr;
               if (pin->ConnectedTo(&peer) == S_OK) {
                  PIN_INFO peerInfo;
                  if (SUCCEEDED(peer->QueryPinInfo(&peerInfo))) {
                     filters.append(peerInfo.pFilter);
                  }
                  peer->Release();

               } else {
                  locker->unlock();
                  HRESULT hr;
                  if (SUCCEEDED(hr = graph->RenderEx(pin, 1, nullptr))) {
                     rendered = true;

                  } else if (renderHr == S_OK || renderHr == VFW_E_NO_DECOMPRESSOR) {
                     renderHr = hr;
                  }

                  locker->relock();
               }
            }
         }

         pins->Release();

         if (outputs == 0) {
            rendered = true;
         }
      }

      filter->Release();
   }

   if (m_audioOutput && ! isConnected(m_audioOutput, PINDIR_INPUT)) {
      graph->RemoveFilter(m_audioOutput);

      m_executedTasks &= ~SetAudioOutput;
   }

   if (m_videoOutput && ! isConnected(m_videoOutput, PINDIR_INPUT)) {
      graph->RemoveFilter(m_videoOutput);

      m_executedTasks &= ~SetVideoOutput;
   }

   graph->Release();

   if (!(m_pendingTasks & ReleaseFilters)) {
      if (rendered) {
         if (!(m_executedTasks & FinalizeLoad)) {
            m_pendingTasks |= FinalizeLoad;
         }
      } else {
         m_pendingTasks = 0;

         m_graphStatus = InvalidMedia;

         if (!m_audioOutput && !m_videoOutput) {
            m_error = QMediaPlayer::ResourceError;
            m_errorString = QString();

         } else {
            switch (renderHr) {
               case VFW_E_UNSUPPORTED_AUDIO:
               case VFW_E_UNSUPPORTED_VIDEO:
               case VFW_E_UNSUPPORTED_STREAM:
                  m_error = QMediaPlayer::FormatError;
                  m_errorString = QString();
                  break;

               default:
                  m_error = QMediaPlayer::ResourceError;
                  m_errorString = QString();
                  qWarning("DirectShowPlayerService::doRender: Unresolved error code %x", uint(renderHr));
            }
         }

         QCoreApplication::postEvent(this, new QEvent(QEvent::Type(Error)));
      }

      m_executedTasks |= Render;
   }
}

void DirectShowPlayerService::doFinalizeLoad(QMutexLocker *locker)
{
   (void) locker;

   if (m_graphStatus != Loaded) {
      if (IMediaEvent *event = com_cast<IMediaEvent>(m_graph, IID_IMediaEvent)) {
         event->GetEventHandle(reinterpret_cast<OAEVENT *>(&m_eventHandle));
         event->Release();
      }

      if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
         LONGLONG duration = 0;
         seeking->GetDuration(&duration);
         m_duration = duration / qt_directShowTimeScale;

         DWORD capabilities = 0;
         seeking->GetCapabilities(&capabilities);
         m_seekable = capabilities & AM_SEEKING_CanSeekAbsolute;

         seeking->Release();
      }
   }

   if ((m_executedTasks & SetOutputs) == SetOutputs) {
      m_streamTypes = AudioStream | VideoStream;
   } else {
      m_streamTypes = findStreamTypes(m_source);
   }

   m_executedTasks |= FinalizeLoad;

   m_graphStatus = Loaded;

   QCoreApplication::postEvent(this, new QEvent(QEvent::Type(FinalizedLoad)));
}

void DirectShowPlayerService::releaseGraph()
{
   if (m_graph) {
      if (m_executingTask != 0) {
         // {8E1C39A1-DE53-11cf-AA63-0080C744528D}
         static const GUID iid_IAMOpenProgress = {
            0x8E1C39A1, 0xDE53, 0x11cf, {0xAA, 0x63, 0x00, 0x80, 0xC7, 0x44, 0x52, 0x8D}
         };

         if (IAMOpenProgress *progress = com_cast<IAMOpenProgress>(
                  m_graph, iid_IAMOpenProgress)) {
            progress->AbortOperation();
            progress->Release();
         }
         m_graph->Abort();
      }

      m_pendingTasks = ReleaseGraph;

      ::SetEvent(m_taskHandle);

      m_loop->wait(&m_mutex);
   }
}

void DirectShowPlayerService::doReleaseGraph(QMutexLocker *locker)
{
   (void) locker;

   if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
      control->Stop();
      control->Release();
   }

   if (m_source) {
      m_source->Release();
      m_source = nullptr;
   }

   m_eventHandle = nullptr;

   m_graph->Release();
   m_graph = nullptr;

   m_loop->wake();
}

int DirectShowPlayerService::findStreamTypes(IBaseFilter *source) const
{
   QVarLengthArray<IBaseFilter *, 16> filters;
   source->AddRef();
   filters.append(source);

   int streamTypes = 0;

   while (! filters.isEmpty()) {
      IEnumPins *pins = nullptr;
      IBaseFilter *filter = filters[filters.size() - 1];
      filters.removeLast();

      if (SUCCEEDED(filter->EnumPins(&pins))) {
         for (IPin *pin = nullptr; pins->Next(1, &pin, nullptr) == S_OK; pin->Release()) {
            PIN_DIRECTION direction;

            if (pin->QueryDirection(&direction) == S_OK && direction == PINDIR_OUTPUT) {
               AM_MEDIA_TYPE connectionType;

               if (SUCCEEDED(pin->ConnectionMediaType(&connectionType))) {
                  IPin *peer = nullptr;

                  if (connectionType.majortype == MEDIATYPE_Audio) {
                     streamTypes |= AudioStream;

                  } else if (connectionType.majortype == MEDIATYPE_Video) {
                     streamTypes |= VideoStream;

                  } else if (SUCCEEDED(pin->ConnectedTo(&peer))) {
                     PIN_INFO peerInfo;
                     if (SUCCEEDED(peer->QueryPinInfo(&peerInfo))) {
                        filters.append(peerInfo.pFilter);
                     }
                     peer->Release();
                  }

               } else {
                  streamTypes |= findStreamType(pin);
               }
            }
         }
         pins->Release();
      }
      filter->Release();
   }

   return streamTypes;
}

int DirectShowPlayerService::findStreamType(IPin *pin) const
{
   IEnumMediaTypes *types;

   if (SUCCEEDED(pin->EnumMediaTypes(&types))) {
      bool video = false;
      bool audio = false;
      bool other = false;

      for (AM_MEDIA_TYPE *type = nullptr; types->Next(1, &type, nullptr) == S_OK;
         DirectShowMediaType::deleteType(type)) {

         if (type->majortype == MEDIATYPE_Audio) {
            audio = true;

         } else if (type->majortype == MEDIATYPE_Video) {
            video = true;

         } else {
            other = true;
         }
      }
      types->Release();

      if (other) {
         return 0;

      } else if (audio && !video) {
         return AudioStream;

      } else if (!audio && video) {
         return VideoStream;

      } else {
         return 0;
      }

   } else {
      return 0;
   }
}

void DirectShowPlayerService::play()
{
   QMutexLocker locker(&m_mutex);

   m_pendingTasks &= ~Pause;
   m_pendingTasks |= Play;

   if (m_executedTasks & Render) {
      if (m_executedTasks & Stop) {
         m_atEnd = false;

         if (m_seekPosition == -1) {
            m_dontCacheNextSeekResult = true;
            m_seekPosition = 0;
            m_position = 0;
            m_pendingTasks |= Seek;
         }
         m_executedTasks ^= Stop;
      }

      ::SetEvent(m_taskHandle);
   }

   updateStatus();
}

void DirectShowPlayerService::doPlay(QMutexLocker *locker)
{
   if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
      locker->unlock();
      HRESULT hr = control->Run();
      locker->relock();

      control->Release();

      if (SUCCEEDED(hr)) {
         m_executedTasks |= Play;

         QCoreApplication::postEvent(this, new QEvent(QEvent::Type(StatusChange)));
      } else {
         m_error = QMediaPlayer::ResourceError;
         m_errorString = QString();
         qWarning("DirectShowPlayerService::doPlay: Unresolved error code %x", uint(hr));

         QCoreApplication::postEvent(this, new QEvent(QEvent::Type(Error)));
      }
   }
}

void DirectShowPlayerService::pause()
{
   QMutexLocker locker(&m_mutex);

   m_pendingTasks &= ~Play;
   m_pendingTasks |= Pause;

   if (m_executedTasks & Render) {
      if (m_executedTasks & Stop) {
         m_atEnd = false;
         if (m_seekPosition == -1) {
            m_dontCacheNextSeekResult = true;
            m_seekPosition = 0;
            m_position = 0;
            m_pendingTasks |= Seek;
         }
         m_executedTasks ^= Stop;
      }

      ::SetEvent(m_taskHandle);
   }

   updateStatus();
}

void DirectShowPlayerService::doPause(QMutexLocker *locker)
{
   if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
      locker->unlock();
      HRESULT hr = control->Pause();
      locker->relock();

      control->Release();

      if (SUCCEEDED(hr)) {
         if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
            LONGLONG position = 0;

            seeking->GetCurrentPosition(&position);
            seeking->Release();

            m_position = position / qt_directShowTimeScale;
         } else {
            m_position = 0;
         }

         m_executedTasks |= Pause;

         QCoreApplication::postEvent(this, new QEvent(QEvent::Type(StatusChange)));
      } else {
         m_error = QMediaPlayer::ResourceError;
         m_errorString = QString();
         qWarning("DirectShowPlayerService::doPause: Unresolved error code %x", uint(hr));

         QCoreApplication::postEvent(this, new QEvent(QEvent::Type(Error)));
      }
   }
}

void DirectShowPlayerService::stop()
{
   QMutexLocker locker(&m_mutex);

   m_pendingTasks &= ~(Play | Pause | Seek);

   if ((m_executingTask | m_executedTasks) & (Play | Pause | Seek)) {
      m_pendingTasks |= Stop;

      ::SetEvent(m_taskHandle);

      m_loop->wait(&m_mutex);
   }

   updateStatus();
}

void DirectShowPlayerService::doStop(QMutexLocker *locker)
{
   (void) locker;

   if (m_executedTasks & (Play | Pause)) {
      if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
         control->Stop();
         control->Release();
      }

      m_seekPosition = 0;
      m_position = 0;
      m_dontCacheNextSeekResult = true;
      m_pendingTasks |= Seek;

      m_executedTasks &= ~(Play | Pause);

      QCoreApplication::postEvent(this, new QEvent(QEvent::Type(StatusChange)));
   }

   m_executedTasks |= Stop;

   m_loop->wake();
}

void DirectShowPlayerService::setRate(qreal rate)
{
   QMutexLocker locker(&m_mutex);

   m_rate = rate;

   m_pendingTasks |= SetRate;

   if (m_executedTasks & FinalizeLoad) {
      ::SetEvent(m_taskHandle);
   }
}

void DirectShowPlayerService::doSetRate(QMutexLocker *locker)
{
   if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
      // Cache current values as we can't query IMediaSeeking during a seek due to the
      // possibility of a deadlock when flushing the VideoSurfaceFilter.
      LONGLONG currentPosition = 0;
      seeking->GetCurrentPosition(&currentPosition);
      m_position = currentPosition / qt_directShowTimeScale;

      LONGLONG minimum = 0;
      LONGLONG maximum = 0;

      m_playbackRange = SUCCEEDED(seeking->GetAvailable(&minimum, &maximum))
         ? QMediaTimeRange(minimum / qt_directShowTimeScale, maximum / qt_directShowTimeScale)
         : QMediaTimeRange();

      locker->unlock();
      HRESULT hr = seeking->SetRate(m_rate);
      locker->relock();

      if (!SUCCEEDED(hr)) {
         double rate = 0.0;
         m_rate = seeking->GetRate(&rate)
            ? rate
            : 1.0;
      }

      seeking->Release();
   } else if (m_rate != 1.0) {
      m_rate = 1.0;
   }
   QCoreApplication::postEvent(this, new QEvent(QEvent::Type(RateChange)));
}

qint64 DirectShowPlayerService::position() const
{
   QMutexLocker locker(const_cast<QMutex *>(&m_mutex));

   if (m_graphStatus == Loaded) {
      if (m_executingTask == Seek || m_executingTask == SetRate || (m_pendingTasks & Seek)) {
         return m_position;
      } else if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
         LONGLONG position = 0;

         seeking->GetCurrentPosition(&position);
         seeking->Release();

         const_cast<qint64 &>(m_position) = position / qt_directShowTimeScale;

         return m_position;
      }
   }
   return 0;
}

QMediaTimeRange DirectShowPlayerService::availablePlaybackRanges() const
{
   QMutexLocker locker(const_cast<QMutex *>(&m_mutex));

   if (m_graphStatus == Loaded) {
      if (m_executingTask == Seek || m_executingTask == SetRate || (m_pendingTasks & Seek)) {
         return m_playbackRange;
      } else if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
         LONGLONG minimum = 0;
         LONGLONG maximum = 0;

         HRESULT hr = seeking->GetAvailable(&minimum, &maximum);
         seeking->Release();

         if (SUCCEEDED(hr)) {
            return QMediaTimeRange(minimum, maximum);
         }
      }
   }
   return QMediaTimeRange();
}

void DirectShowPlayerService::seek(qint64 position)
{
   QMutexLocker locker(&m_mutex);

   m_seekPosition = position;

   m_pendingTasks |= Seek;

   if (m_executedTasks & FinalizeLoad) {
      ::SetEvent(m_taskHandle);
   }
}

void DirectShowPlayerService::doSeek(QMutexLocker *locker)
{
   if (m_seekPosition == -1) {
      return;
   }

   if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
      LONGLONG seekPosition = LONGLONG(m_seekPosition) * qt_directShowTimeScale;

      // Cache current values as we can't query IMediaSeeking during a seek due to the
      // possibility of a deadlock when flushing the VideoSurfaceFilter.
      LONGLONG currentPosition = 0;
      if (! m_dontCacheNextSeekResult) {
         seeking->GetCurrentPosition(&currentPosition);
         m_position = currentPosition / qt_directShowTimeScale;
      }

      LONGLONG minimum = 0;
      LONGLONG maximum = 0;
      m_playbackRange = SUCCEEDED(seeking->GetAvailable(&minimum, &maximum))
         ? QMediaTimeRange(minimum / qt_directShowTimeScale, maximum / qt_directShowTimeScale) : QMediaTimeRange();

      locker->unlock();
      seeking->SetPositions(&seekPosition, AM_SEEKING_AbsolutePositioning, nullptr, AM_SEEKING_NoPositioning);
      locker->relock();

      if (!m_dontCacheNextSeekResult) {
         seeking->GetCurrentPosition(&currentPosition);
         m_position = currentPosition / qt_directShowTimeScale;
      }

      seeking->Release();

      QCoreApplication::postEvent(this, new QEvent(QEvent::Type(PositionChange)));
   }

   m_seekPosition = -1;
   m_dontCacheNextSeekResult = false;
}

int DirectShowPlayerService::bufferStatus() const

{
#if defined(QT_USE_WMSDK)
   QMutexLocker locker(const_cast<QMutex *>(&m_mutex));

   if (IWMReaderAdvanced2 *reader = com_cast<IWMReaderAdvanced2>(
            m_source, IID_IWMReaderAdvanced2)) {
      DWORD percentage = 0;

      reader->GetBufferProgress(&percentage, 0);
      reader->Release();

      return percentage;
   } else {
      return 0;
   }
#else
   return 0;
#endif

}

void DirectShowPlayerService::setAudioOutput(IBaseFilter *filter)
{
   QMutexLocker locker(&m_mutex);

   if (m_graph) {
      if (m_audioOutput) {
         if (m_executedTasks & SetAudioOutput) {
            m_pendingTasks |= ReleaseAudioOutput;

            ::SetEvent(m_taskHandle);

            m_loop->wait(&m_mutex);
         }
         m_audioOutput->Release();
      }

      m_audioOutput = filter;

      if (m_audioOutput) {
         m_audioOutput->AddRef();

         m_pendingTasks |= SetAudioOutput;

         if (m_executedTasks & SetSource) {
            m_pendingTasks |= Render;

            ::SetEvent(m_taskHandle);
         }

      } else {
         m_pendingTasks &= ~ SetAudioOutput;
      }

   } else {
      if (m_audioOutput) {
         m_audioOutput->Release();
      }

      m_audioOutput = filter;

      if (m_audioOutput) {
         m_audioOutput->AddRef();
      }
   }

   m_playerControl->updateAudioOutput(m_audioOutput);
}

void DirectShowPlayerService::doReleaseAudioOutput(QMutexLocker *locker)
{
   (void) locker;

   m_pendingTasks |= m_executedTasks & (Play | Pause);

   if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
      control->Stop();
      control->Release();
   }

   IBaseFilter *decoder = getConnected(m_audioOutput, PINDIR_INPUT);
   if (! decoder) {
      decoder = m_audioOutput;
      decoder->AddRef();
   }

   // {DCFBDCF6-0DC2-45f5-9AB2-7C330EA09C29}
   static const GUID iid_IFilterChain = {
      0xDCFBDCF6, 0x0DC2, 0x45f5, {0x9A, 0xB2, 0x7C, 0x33, 0x0E, 0xA0, 0x9C, 0x29}
   };

   if (IFilterChain *chain = com_cast<IFilterChain>(m_graph, iid_IFilterChain)) {
      chain->RemoveChain(decoder, m_audioOutput);
      chain->Release();
   } else {
      m_graph->RemoveFilter(m_audioOutput);
   }

   decoder->Release();

   m_executedTasks &= ~SetAudioOutput;
   m_loop->wake();
}

void DirectShowPlayerService::setVideoOutput(IBaseFilter *filter)
{
   QMutexLocker locker(&m_mutex);

   if (m_graph) {
      if (m_videoOutput) {
         if (m_executedTasks & SetVideoOutput) {
            m_pendingTasks |= ReleaseVideoOutput;

            ::SetEvent(m_taskHandle);

            m_loop->wait(&m_mutex);
         }
         m_videoOutput->Release();
      }

      m_videoOutput = filter;

      if (m_videoOutput) {
         m_videoOutput->AddRef();

         m_pendingTasks |= SetVideoOutput;

         if (m_executedTasks & SetSource) {
            m_pendingTasks |= Render;

            ::SetEvent(m_taskHandle);
         }
      }
   } else {
      if (m_videoOutput) {
         m_videoOutput->Release();
      }

      m_videoOutput = filter;

      if (m_videoOutput) {
         m_videoOutput->AddRef();
      }
   }
}

void DirectShowPlayerService::doReleaseVideoOutput(QMutexLocker *locker)
{
   (void) locker;

   m_pendingTasks |= m_executedTasks & (Play | Pause);

   if (IMediaControl *control = com_cast<IMediaControl>(m_graph, IID_IMediaControl)) {
      control->Stop();
      control->Release();
   }

   IBaseFilter *intermediate = nullptr;
   if (!SUCCEEDED(m_graph->FindFilterByName(L"Color Space Converter", &intermediate))) {
      intermediate = m_videoOutput;
      intermediate->AddRef();
   }

   IBaseFilter *decoder = getConnected(intermediate, PINDIR_INPUT);
   if (!decoder) {
      decoder = intermediate;
      decoder->AddRef();
   }

   // {DCFBDCF6-0DC2-45f5-9AB2-7C330EA09C29}
   static const GUID iid_IFilterChain = {
      0xDCFBDCF6, 0x0DC2, 0x45f5, {0x9A, 0xB2, 0x7C, 0x33, 0x0E, 0xA0, 0x9C, 0x29}
   };

   if (IFilterChain *chain = com_cast<IFilterChain>(m_graph, iid_IFilterChain)) {
      chain->RemoveChain(decoder, m_videoOutput);
      chain->Release();
   } else {
      m_graph->RemoveFilter(m_videoOutput);
   }

   intermediate->Release();
   decoder->Release();

   m_executedTasks &= ~SetVideoOutput;

   m_loop->wake();
}

void DirectShowPlayerService::customEvent(QEvent *event)
{
   if (event->type() == QEvent::Type(FinalizedLoad)) {
      QMutexLocker locker(&m_mutex);

      m_playerControl->updateMediaInfo(m_duration, m_streamTypes, m_seekable);
      m_metaDataControl->updateMetadata(m_graph, m_source, m_url.toString());

      updateStatus();

   } else if (event->type() == QEvent::Type(Error)) {
      QMutexLocker locker(&m_mutex);

      if (m_error != QMediaPlayer::NoError) {
         m_playerControl->updateError(m_error, m_errorString);
         m_playerControl->updateMediaInfo(m_duration, m_streamTypes, m_seekable);
         m_playerControl->updateState(QMediaPlayer::StoppedState);
         updateStatus();
      }

   } else if (event->type() == QEvent::Type(RateChange)) {
      QMutexLocker locker(&m_mutex);

      m_playerControl->updatePlaybackRate(m_rate);

   } else if (event->type() == QEvent::Type(StatusChange)) {
      QMutexLocker locker(&m_mutex);

      updateStatus();
      m_playerControl->updatePosition(m_position);

   } else if (event->type() == QEvent::Type(DurationChange)) {
      QMutexLocker locker(&m_mutex);

      m_playerControl->updateMediaInfo(m_duration, m_streamTypes, m_seekable);

   } else if (event->type() == QEvent::Type(EndOfMedia)) {
      QMutexLocker locker(&m_mutex);

      if (m_atEnd) {
         m_playerControl->updateState(QMediaPlayer::StoppedState);
         m_playerControl->updateStatus(QMediaPlayer::EndOfMedia);
         m_playerControl->updatePosition(m_position);
      }

   } else if (event->type() == QEvent::Type(PositionChange)) {
      QMutexLocker locker(&m_mutex);

      if (m_playerControl->mediaStatus() == QMediaPlayer::EndOfMedia) {
         m_playerControl->updateStatus(QMediaPlayer::LoadedMedia);
      }
      m_playerControl->updatePosition(m_position);

   } else {
      QMediaService::customEvent(event);
   }
}

void DirectShowPlayerService::videoOutputChanged()
{
   setVideoOutput(m_videoRendererControl->filter());
}

void DirectShowPlayerService::graphEvent(QMutexLocker *locker)
{
   (void) locker;

   if (IMediaEvent *event = com_cast<IMediaEvent>(m_graph, IID_IMediaEvent)) {
      long eventCode;
      LONG_PTR param1;
      LONG_PTR param2;

      while (event->GetEvent(&eventCode, &param1, &param2, 0) == S_OK) {
         switch (eventCode) {
            case EC_BUFFERING_DATA:
               m_buffering = param1;

               QCoreApplication::postEvent(this, new QEvent(QEvent::Type(StatusChange)));
               break;

            case EC_COMPLETE:
               m_executedTasks &= ~(Play | Pause);
               m_executedTasks |= Stop;

               m_buffering = false;
               m_atEnd = true;

               if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
                  LONGLONG position = 0;

                  seeking->GetCurrentPosition(&position);
                  seeking->Release();

                  m_position = position / qt_directShowTimeScale;
               }

               QCoreApplication::postEvent(this, new QEvent(QEvent::Type(EndOfMedia)));
               break;

            case EC_LENGTH_CHANGED:
               if (IMediaSeeking *seeking = com_cast<IMediaSeeking>(m_graph, IID_IMediaSeeking)) {
                  LONGLONG duration = 0;
                  seeking->GetDuration(&duration);
                  m_duration = duration / qt_directShowTimeScale;

                  DWORD capabilities = 0;
                  seeking->GetCapabilities(&capabilities);
                  m_seekable = capabilities & AM_SEEKING_CanSeekAbsolute;

                  seeking->Release();

                  QCoreApplication::postEvent(this, new QEvent(QEvent::Type(DurationChange)));
               }
               break;

            default:
               break;
         }

         event->FreeEventParams(eventCode, param1, param2);
      }

      event->Release();
   }
}

void DirectShowPlayerService::updateStatus()
{
   switch (m_graphStatus) {
      case NoMedia:
         m_playerControl->updateStatus(QMediaPlayer::NoMedia);
         break;

      case Loading:
         m_playerControl->updateStatus(QMediaPlayer::LoadingMedia);
         break;

      case Loaded:
         if ((m_pendingTasks | m_executingTask | m_executedTasks) & (Play | Pause)) {
            if (m_buffering) {
               m_playerControl->updateStatus(QMediaPlayer::BufferingMedia);
            } else {
               m_playerControl->updateStatus(QMediaPlayer::BufferedMedia);
            }
         } else {
            m_playerControl->updateStatus(QMediaPlayer::LoadedMedia);
         }
         break;

      case InvalidMedia:
         m_playerControl->updateStatus(QMediaPlayer::InvalidMedia);
         break;

      default:
         m_playerControl->updateStatus(QMediaPlayer::UnknownMediaStatus);
   }
}

bool DirectShowPlayerService::isConnected(IBaseFilter *filter, PIN_DIRECTION direction) const
{
   bool connected = false;

   IEnumPins *pins = nullptr;

   if (SUCCEEDED(filter->EnumPins(&pins))) {
      for (IPin *pin = nullptr; pins->Next(1, &pin, nullptr) == S_OK; pin->Release()) {
         PIN_DIRECTION dir;

         if (SUCCEEDED(pin->QueryDirection(&dir)) && dir == direction) {
            IPin *peer = nullptr;

            if (SUCCEEDED(pin->ConnectedTo(&peer))) {
               connected = true;
               peer->Release();
            }
         }
      }
      pins->Release();
   }

   return connected;
}

IBaseFilter *DirectShowPlayerService::getConnected(IBaseFilter *filter, PIN_DIRECTION direction) const
{
   IBaseFilter *connected = nullptr;
   IEnumPins *pins = nullptr;

   if (SUCCEEDED(filter->EnumPins(&pins))) {
      for (IPin *pin = nullptr; pins->Next(1, &pin, nullptr) == S_OK; pin->Release()) {
         PIN_DIRECTION dir;

         if (SUCCEEDED(pin->QueryDirection(&dir)) && dir == direction) {
            IPin *peer = nullptr;

            if (SUCCEEDED(pin->ConnectedTo(&peer))) {
               PIN_INFO info;

               if (SUCCEEDED(peer->QueryPinInfo(&info))) {
                  if (connected) {
                     qWarning("DirectShowPlayerService::getConnected: Multiple connected filters");
                     connected->Release();
                  }
                  connected = info.pFilter;
               }
               peer->Release();
            }
         }
      }
      pins->Release();
   }

   return connected;
}

void DirectShowPlayerService::run()
{
   QMutexLocker locker(&m_mutex);

   for (;;) {
      ::ResetEvent(m_taskHandle);

      while (m_pendingTasks == 0) {
         DWORD result = 0;

         locker.unlock();
         if (m_eventHandle) {
            HANDLE handles[] = { m_taskHandle, m_eventHandle };

            result = ::WaitForMultipleObjects(2, handles, false, INFINITE);
         } else {
            result = ::WaitForSingleObject(m_taskHandle, INFINITE);
         }
         locker.relock();

         if (result == WAIT_OBJECT_0 + 1) {
            graphEvent(&locker);
         }
      }

      if (m_pendingTasks & ReleaseGraph) {
         m_pendingTasks ^= ReleaseGraph;
         m_executingTask = ReleaseGraph;

         doReleaseGraph(&locker);
         //if the graph is released, we should not process other operations later
         if (m_pendingTasks & Shutdown) {
            m_pendingTasks = 0;
            return;
         }
         m_pendingTasks = 0;

      } else if (m_pendingTasks & Shutdown) {
         return;

      } else if (m_pendingTasks & ReleaseAudioOutput) {
         m_pendingTasks ^= ReleaseAudioOutput;
         m_executingTask = ReleaseAudioOutput;

         doReleaseAudioOutput(&locker);

      } else if (m_pendingTasks & ReleaseVideoOutput) {
         m_pendingTasks ^= ReleaseVideoOutput;
         m_executingTask = ReleaseVideoOutput;

         doReleaseVideoOutput(&locker);

      } else if (m_pendingTasks & SetUrlSource) {
         m_pendingTasks ^= SetUrlSource;
         m_executingTask = SetUrlSource;

         doSetUrlSource(&locker);

      } else if (m_pendingTasks & SetStreamSource) {
         m_pendingTasks ^= SetStreamSource;
         m_executingTask = SetStreamSource;

         doSetStreamSource(&locker);

      } else if (m_pendingTasks & Render) {
         m_pendingTasks ^= Render;
         m_executingTask = Render;

         doRender(&locker);

      } else if (!(m_executedTasks & Render)) {
         m_pendingTasks &= ~(FinalizeLoad | SetRate | Stop | Pause | Seek | Play);

      } else if (m_pendingTasks & FinalizeLoad) {
         m_pendingTasks ^= FinalizeLoad;
         m_executingTask = FinalizeLoad;

         doFinalizeLoad(&locker);

      } else if (m_pendingTasks & Stop) {
         m_pendingTasks ^= Stop;
         m_executingTask = Stop;

         doStop(&locker);
      } else if (m_pendingTasks & SetRate) {
         m_pendingTasks ^= SetRate;
         m_executingTask = SetRate;

         doSetRate(&locker);
      } else if (m_pendingTasks & Pause) {
         m_pendingTasks ^= Pause;
         m_executingTask = Pause;

         doPause(&locker);

      } else if (m_pendingTasks & Seek) {
         m_pendingTasks ^= Seek;
         m_executingTask = Seek;

         doSeek(&locker);

      } else if (m_pendingTasks & Play) {
         m_pendingTasks ^= Play;
         m_executingTask = Play;

         doPlay(&locker);
      }
      m_executingTask = 0;
   }
}
