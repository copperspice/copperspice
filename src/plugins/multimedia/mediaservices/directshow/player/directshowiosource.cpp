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

#include <directshowiosource.h>

#include <qcoreapplication.h>
#include <qurl.h>

#include <directshowmediatype.h>
#include <directshowpinenum.h>
#include <dsplayer_global.h>

static const GUID directshow_subtypes[] = {
   MEDIASUBTYPE_NULL,
   MEDIASUBTYPE_Avi,
   MEDIASUBTYPE_Asf,
   MEDIASUBTYPE_MPEG1Video,
   MEDIASUBTYPE_QTMovie,
   MEDIASUBTYPE_WAVE,
   MEDIASUBTYPE_AIFF,
   MEDIASUBTYPE_AU,
   MEDIASUBTYPE_DssVideo,
   MEDIASUBTYPE_MPEG1Audio,
   MEDIASUBTYPE_MPEG1System,
   MEDIASUBTYPE_MPEG1VideoCD
};

DirectShowIOSource::DirectShowIOSource(DirectShowEventLoop *loop)
   : m_ref(1)
   , m_state(State_Stopped)
   , m_reader(nullptr)
   , m_loop(loop)
   , m_graph(nullptr)
   , m_clock(nullptr)
   , m_allocator(nullptr)
   , m_peerPin(nullptr)
   , m_pinId(QLatin1String("Data"))
   , m_queriedForAsyncReader(false)
{
   // This filter has only one possible output type, that is, a stream of data
   // with no particular subtype. The graph builder will try every demux/decode filters
   // to find one able to decode the stream.
   //
   // The filter works in pull mode, the downstream filter is responsible for requesting
   // samples from this one.
   //
   QVector<AM_MEDIA_TYPE> mediaTypes;
   AM_MEDIA_TYPE type = {
      MEDIATYPE_Stream,  // majortype
      MEDIASUBTYPE_NULL, // subtype
      TRUE,              // bFixedSizeSamples
      FALSE,             // bTemporalCompression
      1,                 // lSampleSize
      GUID_NULL,         // formattype
      nullptr,          // pUnk
      0,                 // cbFormat
      nullptr,          // pbFormat
   };

   static const int count = sizeof(directshow_subtypes) / sizeof(GUID);

   for (int i = 0; i < count; ++i) {
      type.subtype = directshow_subtypes[i];
      mediaTypes.append(type);
   }

   setMediaTypes(mediaTypes);
}

DirectShowIOSource::~DirectShowIOSource()
{
   Q_ASSERT(m_ref == 0);

   delete m_reader;
}

void DirectShowIOSource::setDevice(QIODevice *device)
{
   Q_ASSERT(!m_reader);

   m_reader = new DirectShowIOReader(device, this, m_loop);
}

void DirectShowIOSource::setAllocator(IMemAllocator *allocator)
{
   if (m_allocator == allocator) {
      return;
   }

   if (m_allocator) {
      m_allocator->Release();
   }

   m_allocator = allocator;

   if (m_allocator) {
      m_allocator->AddRef();
   }
}

// IUnknown
HRESULT DirectShowIOSource::QueryInterface(REFIID riid, void **ppvObject)
{
   // 2dd74950-a890-11d1-abe8-00a0c905f375
   static const GUID iid_IAmFilterMiscFlags = {
      0x2dd74950, 0xa890, 0x11d1, {0xab, 0xe8, 0x00, 0xa0, 0xc9, 0x05, 0xf3, 0x75}
   };

   if (!ppvObject) {
      return E_POINTER;

   } else if (riid == IID_IUnknown || riid == IID_IPersist || riid == IID_IMediaFilter || riid == IID_IBaseFilter) {
      *ppvObject = static_cast<IBaseFilter *>(this);

   } else if (riid == iid_IAmFilterMiscFlags) {
      *ppvObject = static_cast<IAMFilterMiscFlags *>(this);

   } else if (riid == IID_IPin) {
      *ppvObject = static_cast<IPin *>(this);

   } else if (riid == IID_IAsyncReader) {
      m_queriedForAsyncReader = true;
      *ppvObject = static_cast<IAsyncReader *>(m_reader);

   } else {
      *ppvObject = nullptr;

      return E_NOINTERFACE;
   }

   AddRef();

   return S_OK;
}

ULONG DirectShowIOSource::AddRef()
{
   return InterlockedIncrement(&m_ref);
}

ULONG DirectShowIOSource::Release()
{
   ULONG ref = InterlockedDecrement(&m_ref);

   if (ref == 0) {
      delete this;
   }

   return ref;
}

// IPersist
HRESULT DirectShowIOSource::GetClassID(CLSID *pClassID)
{
   *pClassID = CLSID_NULL;

   return S_OK;
}

// IMediaFilter
HRESULT DirectShowIOSource::Run(REFERENCE_TIME tStart)
{
   (void) tStart;

   QMutexLocker locker(&m_mutex);

   m_state = State_Running;

   return S_OK;
}

HRESULT DirectShowIOSource::Pause()
{
   QMutexLocker locker(&m_mutex);

   m_state = State_Paused;

   return S_OK;
}

HRESULT DirectShowIOSource::Stop()
{
   QMutexLocker locker(&m_mutex);

   m_state = State_Stopped;

   return S_OK;
}

HRESULT DirectShowIOSource::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *pState)
{
   (void) dwMilliSecsTimeout;

   if (!pState) {
      return E_POINTER;
   } else {
      QMutexLocker locker(&m_mutex);

      *pState = m_state;

      return S_OK;
   }
}

HRESULT DirectShowIOSource::SetSyncSource(IReferenceClock *pClock)
{
   QMutexLocker locker(&m_mutex);

   if (m_clock) {
      m_clock->Release();
   }

   m_clock = pClock;

   if (m_clock) {
      m_clock->AddRef();
   }

   return S_OK;
}

HRESULT DirectShowIOSource::GetSyncSource(IReferenceClock **ppClock)
{
   if (!ppClock) {
      return E_POINTER;

   } else {
      if (!m_clock) {
         *ppClock = nullptr;

         return S_FALSE;
      } else {
         m_clock->AddRef();

         *ppClock = m_clock;

         return S_OK;
      }
   }
}

// IBaseFilter
HRESULT DirectShowIOSource::EnumPins(IEnumPins **ppEnum)
{
   if (!ppEnum) {
      return E_POINTER;
   } else {
      *ppEnum = new DirectShowPinEnum(QList<IPin *>() << this);

      return S_OK;
   }
}

HRESULT DirectShowIOSource::FindPin(LPCWSTR Id, IPin **ppPin)
{
   if (! ppPin || !Id) {
      return E_POINTER;

   } else {
      QMutexLocker locker(&m_mutex);
      if (QString::fromStdWString(std::wstring(Id)) == m_pinId) {
         AddRef();

         *ppPin = this;

         return S_OK;

      } else {
         *ppPin = nullptr;

         return VFW_E_NOT_FOUND;
      }
   }
}

HRESULT DirectShowIOSource::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
   QMutexLocker locker(&m_mutex);

   m_graph = pGraph;
   m_filterName = QString::fromStdWString(std::wstring(pName));

   return S_OK;
}

HRESULT DirectShowIOSource::QueryFilterInfo(FILTER_INFO *pInfo)
{
   if (! pInfo) {
      return E_POINTER;

   } else {
      QString name = m_filterName;

      if (name.length() >= MAX_FILTER_NAME) {
         name.truncate(MAX_FILTER_NAME - 1);
      }

      std::wstring tmp = name.toStdWString();
      int length = tmp.size();

      memcpy(pInfo->achName, tmp.data(), tmp.size() * 2);
      pInfo->achName[length] = '\0';

      if (m_graph) {
         m_graph->AddRef();
      }

      pInfo->pGraph = m_graph;

      return S_OK;
   }
}

HRESULT DirectShowIOSource::QueryVendorInfo(LPWSTR *pVendorInfo)
{
   (void) pVendorInfo;

   return E_NOTIMPL;
}

// IAMFilterMiscFlags
ULONG DirectShowIOSource::GetMiscFlags()
{
   return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

// IPin
HRESULT DirectShowIOSource::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
   if (!pReceivePin) {
      return E_POINTER;
   }

   QMutexLocker locker(&m_mutex);

   if (m_state != State_Stopped) {
      return VFW_E_NOT_STOPPED;
   }

   if (m_peerPin) {
      return VFW_E_ALREADY_CONNECTED;
   }

   // If we get a type from the graph manager, check that we support that
   if (pmt && pmt->majortype != MEDIATYPE_Stream) {
      return VFW_E_TYPE_NOT_ACCEPTED;
   }

   // This filter only works in pull mode, the downstream filter must query for the
   // AsyncReader interface during ReceiveConnection().
   // If it doesn't, we can't connect to it.
   m_queriedForAsyncReader = false;
   HRESULT hr = 0;

   // Negotiation of media type
   // - Complete'ish type (Stream with subtype specified).

   if (pmt && pmt->subtype != MEDIASUBTYPE_NULL /* aka. GUID_NULL */) {
      hr = pReceivePin->ReceiveConnection(this, pmt);

      // Update the media type for the current connection.
      if (SUCCEEDED(hr)) {
         m_connectionMediaType = *pmt;
      }

   } else if (pmt && pmt->subtype == MEDIATYPE_NULL) { // - Partial type (Stream, but no subtype specified).
      m_connectionMediaType = *pmt;
      // Check if the receiving pin accepts any of the streaming subtypes.
      QVector<AM_MEDIA_TYPE>::const_iterator cit = m_mediaTypes.constBegin();

      while (cit != m_mediaTypes.constEnd()) {
         m_connectionMediaType.subtype = cit->subtype;
         hr = pReceivePin->ReceiveConnection(this, &m_connectionMediaType);
         if (SUCCEEDED(hr)) {
            break;
         }
         ++cit;
      }

   } else { // - No media type specified.
      // Check if the receiving pin accepts any of the streaming types.
      QVector<AM_MEDIA_TYPE>::const_iterator cit = m_mediaTypes.constBegin();

      while (cit != m_mediaTypes.constEnd()) {
         // &* will dereference the iterator, return the item, then take the address of the item
         hr = pReceivePin->ReceiveConnection(this, &*cit);

         if (SUCCEEDED(hr)) {
            m_connectionMediaType = *cit;
            break;
         }
         ++cit;
      }
   }

   if (SUCCEEDED(hr) && m_queriedForAsyncReader) {
      m_peerPin = pReceivePin;
      m_peerPin->AddRef();

   } else {
      pReceivePin->Disconnect();
      if (m_allocator) {
         m_allocator->Release();
         m_allocator = nullptr;
      }
      if (!m_queriedForAsyncReader) {
         hr = VFW_E_NO_TRANSPORT;
      }

      m_connectionMediaType.clear();
   }

   return hr;
}

HRESULT DirectShowIOSource::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
   (void) pConnector;
   (void) pmt;

   // Output pin
   return E_NOTIMPL;
}

HRESULT DirectShowIOSource::Disconnect()
{
   QMutexLocker locker(&m_mutex);

   if (!m_peerPin) {
      return S_FALSE;
   } else if (m_state != State_Stopped) {
      return VFW_E_NOT_STOPPED;
   } else {
      HRESULT hr = m_peerPin->Disconnect();

      if (!SUCCEEDED(hr)) {
         return hr;
      }

      if (m_allocator) {
         m_allocator->Release();
         m_allocator = nullptr;
      }

      m_peerPin->Release();
      m_peerPin = nullptr;

      return S_OK;
   }
}

HRESULT DirectShowIOSource::ConnectedTo(IPin **ppPin)
{
   if (!ppPin) {
      return E_POINTER;
   } else {
      QMutexLocker locker(&m_mutex);

      if (!m_peerPin) {
         *ppPin = nullptr;

         return VFW_E_NOT_CONNECTED;
      } else {
         m_peerPin->AddRef();

         *ppPin = m_peerPin;

         return S_OK;
      }
   }
}

HRESULT DirectShowIOSource::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
   if (!pmt) {
      return E_POINTER;

   } else {
      QMutexLocker locker(&m_mutex);

      if (!m_peerPin) {
         pmt = nullptr;
         return VFW_E_NOT_CONNECTED;

      } else {
         DirectShowMediaType::copy(pmt, m_connectionMediaType);
         return S_OK;
      }
   }
}

HRESULT DirectShowIOSource::QueryPinInfo(PIN_INFO *pInfo)
{
   if (! pInfo) {
      return E_POINTER;

   } else {
      AddRef();

      pInfo->pFilter = this;
      pInfo->dir = PINDIR_OUTPUT;

      std::wstring tmp = m_pinId.toStdWString();

      const int bytes = qMin(static_cast<std::wstring::size_type>(MAX_FILTER_NAME), (tmp.size() + 1) * 2);
      memcpy(pInfo->achName, tmp.data(), bytes);

      return S_OK;
   }
}

HRESULT DirectShowIOSource::QueryId(LPWSTR *Id)
{
   if (! Id) {
      return E_POINTER;

   } else {
      std::wstring tmp = m_pinId.toStdWString();
      const int bytes = (tmp.size() + 1) * 2;

      *Id = static_cast<LPWSTR>(::CoTaskMemAlloc(bytes));

      memcpy(*Id, tmp.data(), bytes);

      return S_OK;
   }
}

HRESULT DirectShowIOSource::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
   if (! pmt) {
      return E_POINTER;

   } else if (pmt->majortype == MEDIATYPE_Stream) {
      return S_OK;

   } else {
      return S_FALSE;
   }
}

HRESULT DirectShowIOSource::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
   if (! ppEnum) {
      return E_POINTER;

   } else {
      *ppEnum = createMediaTypeEnum();

      return S_OK;
   }
}

HRESULT DirectShowIOSource::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
   (void) apPin;
   (void) nPin;

   return E_NOTIMPL;
}

HRESULT DirectShowIOSource::EndOfStream()
{
   return S_OK;
}

HRESULT DirectShowIOSource::BeginFlush()
{
   return m_reader->BeginFlush();
}

HRESULT DirectShowIOSource::EndFlush()
{
   return m_reader->EndFlush();
}

HRESULT DirectShowIOSource::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
   (void) tStart;
   (void) tStop;
   (void) dRate;

   return S_OK;
}

HRESULT DirectShowIOSource::QueryDirection(PIN_DIRECTION *pPinDir)
{
   if (!pPinDir) {
      return E_POINTER;

   } else {
      *pPinDir = PINDIR_OUTPUT;

      return S_OK;
   }
}
