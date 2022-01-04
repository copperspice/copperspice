/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#ifndef DIRECTSHOWIOSOURCE_H
#define DIRECTSHOWIOSOURCE_H

#include <qfile.h>

#include <directshowioreader.h>
#include <directshowmediatype.h>
#include <directshowmediatypelist.h>
#include <dsplayer_global.h>

class DirectShowIOSource : public DirectShowMediaTypeList , public IBaseFilter, public IAMFilterMiscFlags, public IPin
{
 public:
   DirectShowIOSource(DirectShowEventLoop *loop);
   virtual ~DirectShowIOSource();

   void setDevice(QIODevice *device);
   void setAllocator(IMemAllocator *allocator);

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

 private:
   volatile LONG m_ref;
   FILTER_STATE m_state;
   DirectShowIOReader *m_reader;
   DirectShowEventLoop *m_loop;
   IFilterGraph *m_graph;
   IReferenceClock *m_clock;
   IMemAllocator *m_allocator;
   IPin *m_peerPin;
   DirectShowMediaType m_connectionMediaType;
   QString m_filterName;
   const QString m_pinId;
   bool m_queriedForAsyncReader;
   QMutex m_mutex;
};

#endif
