/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef VIDEOSURFACEFILTER_H
#define VIDEOSURFACEFILTER_H

#include <dsplayer_global.h>
#include <directshowmediatypelist.h>
#include <directshowsamplescheduler.h>
#include <directshowmediatype.h>

#include <qbasictimer.h>
#include <qcoreevent.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qstring.h>
#include <qwaitcondition.h>

#include <dshow.h>

class QAbstractVideoSurface;
class DirectShowEventLoop;

class VideoSurfaceFilter
   : public QObject
   , public DirectShowMediaTypeList
   , public IBaseFilter
   , public IAMFilterMiscFlags
   , public IPin
{
   CS_OBJECT(VideoSurfaceFilter)

 public:
   VideoSurfaceFilter(QAbstractVideoSurface *surface, DirectShowEventLoop *loop, QObject *parent = nullptr);
   ~VideoSurfaceFilter();

   // IUnknown
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
   ULONG STDMETHODCALLTYPE AddRef();
   ULONG STDMETHODCALLTYPE Release();

   // IPersist
   HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClassID);

   // IMediaFilter
   HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart);
   HRESULT STDMETHODCALLTYPE Pause();
   HRESULT STDMETHODCALLTYPE Stop();

   HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *pState);

   HRESULT STDMETHODCALLTYPE SetSyncSource(IReferenceClock *pClock);
   HRESULT STDMETHODCALLTYPE GetSyncSource(IReferenceClock **ppClock);

   // IBaseFilter
   HRESULT STDMETHODCALLTYPE EnumPins(IEnumPins **ppEnum);
   HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR Id, IPin **ppPin);

   HRESULT STDMETHODCALLTYPE JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);

   HRESULT STDMETHODCALLTYPE QueryFilterInfo(FILTER_INFO *pInfo);
   HRESULT STDMETHODCALLTYPE QueryVendorInfo(LPWSTR *pVendorInfo);

   // IAMFilterMiscFlags
   ULONG STDMETHODCALLTYPE GetMiscFlags();

   // IPin
   HRESULT STDMETHODCALLTYPE Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
   HRESULT STDMETHODCALLTYPE ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
   HRESULT STDMETHODCALLTYPE Disconnect();
   HRESULT STDMETHODCALLTYPE ConnectedTo(IPin **ppPin);

   HRESULT STDMETHODCALLTYPE ConnectionMediaType(AM_MEDIA_TYPE *pmt);

   HRESULT STDMETHODCALLTYPE QueryPinInfo(PIN_INFO *pInfo);
   HRESULT STDMETHODCALLTYPE QueryId(LPWSTR *Id);

   HRESULT STDMETHODCALLTYPE QueryAccept(const AM_MEDIA_TYPE *pmt);

   HRESULT STDMETHODCALLTYPE EnumMediaTypes(IEnumMediaTypes **ppEnum);

   HRESULT STDMETHODCALLTYPE QueryInternalConnections(IPin **apPin, ULONG *nPin);

   HRESULT STDMETHODCALLTYPE EndOfStream();

   HRESULT STDMETHODCALLTYPE BeginFlush();
   HRESULT STDMETHODCALLTYPE EndFlush();

   HRESULT STDMETHODCALLTYPE NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

   HRESULT STDMETHODCALLTYPE QueryDirection(PIN_DIRECTION *pPinDir);

   int currentMediaTypeToken();
   HRESULT nextMediaType(
      int token, int *index, ULONG count, AM_MEDIA_TYPE **types, ULONG *fetchedCount);
   HRESULT skipMediaType(int token, int *index, ULONG count);
   HRESULT cloneMediaType(int token, int index, IEnumMediaTypes **enumeration);

 protected:
   void customEvent(QEvent *event);

 private:
   CS_SLOT_1(Private, void supportedFormatsChanged())
   CS_SLOT_2(supportedFormatsChanged)
   CS_SLOT_1(Private, void sampleReady())
   CS_SLOT_2(sampleReady)

   HRESULT start();
   void stop();
   void flush();

   enum {
      StartSurface = QEvent::User,
      StopSurface,
      FlushSurface
   };

   LONG m_ref;
   FILTER_STATE m_state;
   QAbstractVideoSurface *m_surface;
   DirectShowEventLoop *m_loop;
   IFilterGraph *m_graph;
   IPin *m_peerPin;
   int m_bytesPerLine;
   HRESULT m_startResult;
   QString m_name;
   QString m_pinId;
   DirectShowMediaType m_mediaType;
   QVideoSurfaceFormat m_surfaceFormat;
   QMutex m_mutex;
   QWaitCondition m_wait;
   DirectShowSampleScheduler m_sampleScheduler;
};

#endif
