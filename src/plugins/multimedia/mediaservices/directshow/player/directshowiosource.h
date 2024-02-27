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
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
   ULONG STDMETHODCALLTYPE AddRef() override;
   ULONG STDMETHODCALLTYPE Release() override;

   // IPersist
   HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClassID) override;

   // IMediaFilter
   HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart) override;
   HRESULT STDMETHODCALLTYPE Pause() override;
   HRESULT STDMETHODCALLTYPE Stop() override;

   HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *pState) override;

   HRESULT STDMETHODCALLTYPE SetSyncSource(IReferenceClock *pClock) override;
   HRESULT STDMETHODCALLTYPE GetSyncSource(IReferenceClock **ppClock) override;

   // IBaseFilter
   HRESULT STDMETHODCALLTYPE EnumPins(IEnumPins **ppEnum) override;
   HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR Id, IPin **ppPin) override;

   HRESULT STDMETHODCALLTYPE JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName) override;

   HRESULT STDMETHODCALLTYPE QueryFilterInfo(FILTER_INFO *pInfo) override;
   HRESULT STDMETHODCALLTYPE QueryVendorInfo(LPWSTR *pVendorInfo) override;

   // IAMFilterMiscFlags
   ULONG STDMETHODCALLTYPE GetMiscFlags() override;

   // IPin
   HRESULT STDMETHODCALLTYPE Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt) override;
   HRESULT STDMETHODCALLTYPE ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt) override;
   HRESULT STDMETHODCALLTYPE Disconnect() override;
   HRESULT STDMETHODCALLTYPE ConnectedTo(IPin **ppPin) override;

   HRESULT STDMETHODCALLTYPE ConnectionMediaType(AM_MEDIA_TYPE *pmt) override;

   HRESULT STDMETHODCALLTYPE QueryPinInfo(PIN_INFO *pInfo) override;
   HRESULT STDMETHODCALLTYPE QueryId(LPWSTR *Id) override;

   HRESULT STDMETHODCALLTYPE QueryAccept(const AM_MEDIA_TYPE *pmt) override;

   HRESULT STDMETHODCALLTYPE EnumMediaTypes(IEnumMediaTypes **ppEnum) override;

   HRESULT STDMETHODCALLTYPE QueryInternalConnections(IPin **apPin, ULONG *nPin) override;

   HRESULT STDMETHODCALLTYPE EndOfStream() override;

   HRESULT STDMETHODCALLTYPE BeginFlush() override;
   HRESULT STDMETHODCALLTYPE EndFlush() override;

   HRESULT STDMETHODCALLTYPE NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)  override;

   HRESULT STDMETHODCALLTYPE QueryDirection(PIN_DIRECTION *pPinDir)  override;

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
