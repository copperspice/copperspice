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

#ifndef DSCAMERA_GLOBAL_H
#define DSCAMERA_GLOBAL_H

#include <qglobal.h>

#include <dshow.h>
#include <initguid.h>

DEFINE_GUID(MEDIASUBTYPE_I420,
            0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

extern const GUID MEDIASUBTYPE_RGB24;
extern const GUID MEDIASUBTYPE_RGB32;
extern const GUID MEDIASUBTYPE_YUY2;
extern const GUID MEDIASUBTYPE_MJPG;
extern const GUID MEDIASUBTYPE_RGB555;
extern const GUID MEDIASUBTYPE_YVU9;
extern const GUID MEDIASUBTYPE_UYVY;
extern const GUID PIN_CATEGORY_CAPTURE;
extern const GUID PIN_CATEGORY_PREVIEW;

extern const IID IID_IPropertyBag;
extern const IID IID_ISampleGrabber;
extern const IID IID_ICaptureGraphBuilder2;
extern const IID IID_IAMStreamConfig;


extern const CLSID CLSID_CVidCapClassManager;
extern const CLSID CLSID_VideoInputDeviceCategory;
extern const CLSID CLSID_SampleGrabber;
extern const CLSID CLSID_CaptureGraphBuilder2;

#define SAFE_RELEASE(x) { if(x) x->Release(); x = NULL; }

typedef struct IFileSinkFilter *LPFILESINKFILTER;
typedef struct IAMCopyCaptureFileProgress *LPAMCOPYCAPTUREFILEPROGRESS;

#ifndef __ICaptureGraphBuilder2_INTERFACE_DEFINED__
#define __ICaptureGraphBuilder2_INTERFACE_DEFINED__

struct ICaptureGraphBuilder2 : public IUnknown {
 public:
   virtual HRESULT STDMETHODCALLTYPE SetFiltergraph(
      /* [in] */ IGraphBuilder *pfg) = 0;

   virtual HRESULT STDMETHODCALLTYPE GetFiltergraph(
      /* [out] */ IGraphBuilder **ppfg) = 0;

   virtual HRESULT STDMETHODCALLTYPE SetOutputFileName(
      /* [in] */ const GUID *pType,
      /* [in] */ LPCOLESTR lpstrFile,
      /* [out] */ IBaseFilter **ppf,
      /* [out] */ IFileSinkFilter **ppSink) = 0;

   virtual /* [local] */ HRESULT STDMETHODCALLTYPE FindInterface(
      /* [in] */ const GUID *pCategory,
      /* [in] */ const GUID *pType,
      /* [in] */ IBaseFilter *pf,
      /* [in] */ REFIID riid,
      /* [out] */ void **ppint) = 0;

   virtual HRESULT STDMETHODCALLTYPE RenderStream(
      /* [in] */ const GUID *pCategory,
      /* [in] */ const GUID *pType,
      /* [in] */ IUnknown *pSource,
      /* [in] */ IBaseFilter *pfCompressor,
      /* [in] */ IBaseFilter *pfRenderer) = 0;

   virtual HRESULT STDMETHODCALLTYPE ControlStream(
      /* [in] */ const GUID *pCategory,
      /* [in] */ const GUID *pType,
      /* [in] */ IBaseFilter *pFilter,
      /* [in] */ REFERENCE_TIME *pstart,
      /* [in] */ REFERENCE_TIME *pstop,
      /* [in] */ WORD wStartCookie,
      /* [in] */ WORD wStopCookie) = 0;

   virtual HRESULT STDMETHODCALLTYPE AllocCapFile(
      /* [in] */ LPCOLESTR lpstr,
      /* [in] */ DWORDLONG dwlSize) = 0;

   virtual HRESULT STDMETHODCALLTYPE CopyCaptureFile(
      /* [in] */ LPOLESTR lpwstrOld,
      /* [in] */ LPOLESTR lpwstrNew,
      /* [in] */ int fAllowEscAbort,
      /* [in] */ IAMCopyCaptureFileProgress *pCallback) = 0;

   virtual HRESULT STDMETHODCALLTYPE FindPin(
      /* [in] */ IUnknown *pSource,
      /* [in] */ PIN_DIRECTION pindir,
      /* [in] */ const GUID *pCategory,
      /* [in] */ const GUID *pType,
      /* [in] */ BOOL fUnconnected,
      /* [in] */ int num,
      /* [out] */ IPin **ppPin) = 0;

};
#endif

#ifndef __IAMStreamConfig_INTERFACE_DEFINED__
#define __IAMStreamConfig_INTERFACE_DEFINED__
struct IAMStreamConfig : public IUnknown {
 public:
   virtual HRESULT STDMETHODCALLTYPE SetFormat(
      /* [in] */ AM_MEDIA_TYPE *pmt) = 0;

   virtual HRESULT STDMETHODCALLTYPE GetFormat(
      /* [out] */ AM_MEDIA_TYPE **ppmt) = 0;

   virtual HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(
      /* [out] */ int *piCount,
      /* [out] */ int *piSize) = 0;

   virtual HRESULT STDMETHODCALLTYPE GetStreamCaps(
      /* [in] */ int iIndex,
      /* [out] */ AM_MEDIA_TYPE **ppmt,
      /* [out] */ BYTE *pSCC) = 0;

};
#endif

#ifndef __IErrorLog_INTERFACE_DEFINED__
#define __IErrorLog_INTERFACE_DEFINED__
struct IErrorLog : public IUnknown {
 public:
   virtual HRESULT STDMETHODCALLTYPE AddError(
      /* [in] */ LPCOLESTR pszPropName,
      /* [in] */ EXCEPINFO *pExcepInfo) = 0;

};
#endif

#ifndef __IPropertyBag_INTERFACE_DEFINED__
#define __IPropertyBag_INTERFACE_DEFINED__
struct IPropertyBag : public IUnknown {
 public:
   virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read(
      /* [in] */ LPCOLESTR pszPropName,
      /* [out][in] */ VARIANT *pVar,
      /* [in] */ IErrorLog *pErrorLog) = 0;

   virtual HRESULT STDMETHODCALLTYPE Write(
      /* [in] */ LPCOLESTR pszPropName,
      /* [in] */ VARIANT *pVar) = 0;

};
#endif

typedef struct IMediaSample *LPMEDIASAMPLE;

EXTERN_C const IID IID_ISampleGrabberCB;

#ifndef __ISampleGrabberCB_INTERFACE_DEFINED__
#define __ISampleGrabberCB_INTERFACE_DEFINED__

#undef INTERFACE
#define INTERFACE ISampleGrabberCB
DECLARE_INTERFACE_(ISampleGrabberCB, IUnknown)
{
   //    STDMETHOD(QueryInterface) (THIS_ const GUID *, void **) PURE;
   STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **) override PURE;
   STDMETHOD_(ULONG, AddRef) (THIS) override PURE;
   STDMETHOD_(ULONG, Release) (THIS) override PURE;
   STDMETHOD_(HRESULT, SampleCB) (THIS_ double, LPMEDIASAMPLE) PURE;
   STDMETHOD_(HRESULT, BufferCB) (THIS_ double, BYTE *, long) PURE;
};
#undef INTERFACE

#endif


#ifndef __ISampleGrabber_INTERFACE_DEFINED__
#define __ISampleGrabber_INTERFACE_DEFINED__

#define INTERFACE ISampleGrabber
DECLARE_INTERFACE_(ISampleGrabber, IUnknown)
{
   STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *) override PURE;
   STDMETHOD_(ULONG, AddRef)(THIS) override PURE;
   STDMETHOD_(ULONG, Release)(THIS) override PURE;
   STDMETHOD(SetOneShot)(THIS_ BOOL) PURE;
   STDMETHOD(SetMediaType)(THIS_ const AM_MEDIA_TYPE *) PURE;
   STDMETHOD(GetConnectedMediaType)(THIS_ AM_MEDIA_TYPE *) PURE;
   STDMETHOD(SetBufferSamples)(THIS_ BOOL) PURE;
   STDMETHOD(GetCurrentBuffer)(THIS_ long *, long *) PURE;
   STDMETHOD(GetCurrentSample)(THIS_ IMediaSample **) PURE;
   STDMETHOD(SetCallback)(THIS_ ISampleGrabberCB *, long) PURE;
};
#undef INTERFACE
#endif


#endif
