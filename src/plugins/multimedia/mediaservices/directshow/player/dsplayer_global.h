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

#ifndef DSPLAYER_GLOBAL_H
#define DSPLAYER_GLOBAL_H

#include <qglobal.h>

#include <dshow.h>

template <typename T> T *com_cast(IUnknown *unknown, const IID &iid)
{
   T *iface = nullptr;
   return unknown && unknown->QueryInterface(iid, reinterpret_cast<void **>(&iface)) == S_OK
      ? iface : nullptr;
}

template <typename T> T *com_new(const IID &clsid)
{
   T *object = nullptr;
   return CoCreateInstance(
         clsid,
         nullptr,
         CLSCTX_INPROC_SERVER,
         IID_PPV_ARGS(&object)) == S_OK
      ? object : nullptr;
}

template <typename T> T *com_new(const IID &clsid, const IID &iid)
{
   T *object = nullptr;
   return CoCreateInstance(
         clsid,
         nullptr,
         CLSCTX_INPROC_SERVER,
         iid,
         reinterpret_cast<void **>(&object)) == S_OK
      ? object : nullptr;
}

#ifndef __IFilterGraph2_INTERFACE_DEFINED__
#define __IFilterGraph2_INTERFACE_DEFINED__
#define INTERFACE IFilterGraph2
DECLARE_INTERFACE_(IFilterGraph2, IGraphBuilder)
{
   STDMETHOD(AddSourceFilterForMoniker)(THIS_ IMoniker *, IBindCtx *, LPCWSTR, IBaseFilter **) PURE;
   STDMETHOD(ReconnectEx)(THIS_ IPin *, const AM_MEDIA_TYPE *) PURE;
   STDMETHOD(RenderEx)(IPin *, DWORD, DWORD *) PURE;
};
#undef INTERFACE
#endif

#ifndef __IAMFilterMiscFlags_INTERFACE_DEFINED__
#define __IAMFilterMiscFlags_INTERFACE_DEFINED__
#define INTERFACE IAMFilterMiscFlags
DECLARE_INTERFACE_(IAMFilterMiscFlags, IUnknown)
{
   STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;
   STDMETHOD_(ULONG, GetMiscFlags)(THIS) PURE;
};
#undef INTERFACE
#endif

#ifndef __IFileSourceFilter_INTERFACE_DEFINED__
#define __IFileSourceFilter_INTERFACE_DEFINED__
#define INTERFACE IFileSourceFilter
DECLARE_INTERFACE_(IFileSourceFilter, IUnknown)
{
   STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;
   STDMETHOD(Load)(THIS_ LPCOLESTR, const AM_MEDIA_TYPE *) PURE;
   STDMETHOD(GetCurFile)(THIS_ LPOLESTR * ppszFileName, AM_MEDIA_TYPE *) PURE;
};
#undef INTERFACE
#endif

#ifndef __IAMOpenProgress_INTERFACE_DEFINED__
#define __IAMOpenProgress_INTERFACE_DEFINED__
#undef INTERFACE
#define INTERFACE IAMOpenProgress
DECLARE_INTERFACE_(IAMOpenProgress, IUnknown)
{
   STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;
   STDMETHOD(QueryProgress)(THIS_ LONGLONG *, LONGLONG *) PURE;
   STDMETHOD(AbortOperation)(THIS) PURE;
};
#undef INTERFACE
#endif

#ifndef __IFilterChain_INTERFACE_DEFINED__
#define __IFilterChain_INTERFACE_DEFINED__
#define INTERFACE IFilterChain
DECLARE_INTERFACE_(IFilterChain, IUnknown)
{
   STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *) PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) PURE;
   STDMETHOD_(ULONG, Release)(THIS) PURE;
   STDMETHOD(StartChain)(IBaseFilter *, IBaseFilter *) PURE;
   STDMETHOD(PauseChain)(IBaseFilter *, IBaseFilter *) PURE;
   STDMETHOD(StopChain)(IBaseFilter *, IBaseFilter *) PURE;
   STDMETHOD(RemoveChain)(IBaseFilter *, IBaseFilter *) PURE;
};
#undef INTERFACE
#endif

#endif
