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

#include <qwin_ole.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qmimedata.h>
#include <qmouseevent.h>
#include <qpainter.h>
#include <qwin_context.h>
#include <qwin_mime.h>
#include <qwindow.h>

#include <shlobj.h>

QWindowsOleDataObject::QWindowsOleDataObject(QMimeData *mimeData)
   : m_refs(1), data(mimeData),
     CF_PERFORMEDDROPEFFECT(RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT)),
     performedEffect(DROPEFFECT_NONE)
{
}

QWindowsOleDataObject::~QWindowsOleDataObject()
{
}

void QWindowsOleDataObject::releaseData()
{
   data = nullptr;
}

QMimeData *QWindowsOleDataObject::mimeData() const
{
   return data.data();
}

DWORD QWindowsOleDataObject::reportedPerformedEffect() const
{
   return performedEffect;
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP QWindowsOleDataObject::QueryInterface(REFIID iid, void FAR *FAR *ppv)
{
   if (iid == IID_IUnknown || iid == IID_IDataObject) {
      *ppv = this;
      AddRef();
      return NOERROR;
   }

   *ppv = nullptr;

   return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) QWindowsOleDataObject::AddRef(void)
{
   return ++m_refs;
}

STDMETHODIMP_(ULONG) QWindowsOleDataObject::Release(void)
{
   if (--m_refs == 0) {
      releaseData();
      delete this;

      return 0;
   }

   return m_refs;
}

STDMETHODIMP QWindowsOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
{
   HRESULT hr = ResultFromScode(DATA_E_FORMATETC);

   if (data) {
      const QWindowsMimeConverter &mc = QWindowsContext::instance()->mimeConverter();

      if (QWindowsMime *converter = mc.converterFromMime(*pformatetc, data)) {
         if (converter->convertFromMime(*pformatetc, data, pmedium)) {
            hr = ResultFromScode(S_OK);
         }
      }
   }

   return hr;
}

STDMETHODIMP QWindowsOleDataObject::GetDataHere(LPFORMATETC, LPSTGMEDIUM)
{
   return ResultFromScode(DATA_E_FORMATETC);
}

STDMETHODIMP QWindowsOleDataObject::QueryGetData(LPFORMATETC pformatetc)
{
   HRESULT hr = ResultFromScode(DATA_E_FORMATETC);

   if (data) {
      const QWindowsMimeConverter &mc = QWindowsContext::instance()->mimeConverter();
      hr = mc.converterFromMime(*pformatetc, data) ?
         ResultFromScode(S_OK) : ResultFromScode(S_FALSE);
   }

   return hr;
}

STDMETHODIMP QWindowsOleDataObject::GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC pformatetcOut)
{
   pformatetcOut->ptd = nullptr;
   return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP QWindowsOleDataObject::SetData(LPFORMATETC pFormatetc, STGMEDIUM *pMedium, BOOL fRelease)
{
   HRESULT hr = ResultFromScode(E_NOTIMPL);

   if (pFormatetc->cfFormat == CF_PERFORMEDDROPEFFECT && pMedium->tymed == TYMED_HGLOBAL) {
      DWORD *val = (DWORD *)GlobalLock(pMedium->hGlobal);
      performedEffect = *val;
      GlobalUnlock(pMedium->hGlobal);
      if (fRelease) {
         ReleaseStgMedium(pMedium);
      }
      hr = ResultFromScode(S_OK);
   }

   return hr;
}

STDMETHODIMP QWindowsOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR *ppenumFormatEtc)
{
   if (!data) {
      return ResultFromScode(DATA_E_FORMATETC);
   }

   SCODE sc = S_OK;

   QVector<FORMATETC> fmtetcs;
   if (dwDirection == DATADIR_GET) {
      QWindowsMimeConverter &mc = QWindowsContext::instance()->mimeConverter();
      fmtetcs = mc.allFormatsForMime(data);
   } else {
      FORMATETC formatetc;
      formatetc.cfFormat = CLIPFORMAT(CF_PERFORMEDDROPEFFECT);
      formatetc.dwAspect = DVASPECT_CONTENT;
      formatetc.lindex = -1;
      formatetc.ptd = nullptr;
      formatetc.tymed = TYMED_HGLOBAL;
      fmtetcs.append(formatetc);
   }

   QWindowsOleEnumFmtEtc *enumFmtEtc = new QWindowsOleEnumFmtEtc(fmtetcs);
   *ppenumFormatEtc = enumFmtEtc;
   if (enumFmtEtc->isNull()) {
      delete enumFmtEtc;
      *ppenumFormatEtc = nullptr;
      sc = E_OUTOFMEMORY;
   }

   return ResultFromScode(sc);
}

STDMETHODIMP QWindowsOleDataObject::DAdvise(FORMATETC FAR *, DWORD,
   LPADVISESINK, DWORD FAR *)
{
   return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


STDMETHODIMP QWindowsOleDataObject::DUnadvise(DWORD)
{
   return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP QWindowsOleDataObject::EnumDAdvise(LPENUMSTATDATA FAR *)
{
   return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

QWindowsOleEnumFmtEtc::QWindowsOleEnumFmtEtc(const QVector<FORMATETC> &fmtetcs) :
      m_dwRefs(1), m_nIndex(0), m_isNull(false)
{
   m_lpfmtetcs.reserve(fmtetcs.count());

   for (int idx = 0; idx < fmtetcs.count(); ++idx) {
      LPFORMATETC destetc = new FORMATETC();

      if (copyFormatEtc(destetc, &(fmtetcs.at(idx)))) {
         m_lpfmtetcs.append(destetc);
      } else {
         m_isNull = true;
         delete destetc;
         break;
      }
   }
}

QWindowsOleEnumFmtEtc::QWindowsOleEnumFmtEtc(const QVector<LPFORMATETC> &lpfmtetcs) :
   m_dwRefs(1), m_nIndex(0), m_isNull(false)
{
   m_lpfmtetcs.reserve(lpfmtetcs.count());

   for (int idx = 0; idx < lpfmtetcs.count(); ++idx) {
      LPFORMATETC srcetc = lpfmtetcs.at(idx);
      LPFORMATETC destetc = new FORMATETC();
      if (copyFormatEtc(destetc, srcetc)) {
         m_lpfmtetcs.append(destetc);
      } else {
         m_isNull = true;
         delete destetc;
         break;
      }
   }
}

QWindowsOleEnumFmtEtc::~QWindowsOleEnumFmtEtc()
{
   LPMALLOC pmalloc;

   if (CoGetMalloc(MEMCTX_TASK, &pmalloc) == NOERROR) {
      for (int idx = 0; idx < m_lpfmtetcs.count(); ++idx) {
         LPFORMATETC tmpetc = m_lpfmtetcs.at(idx);
         if (tmpetc->ptd) {
            pmalloc->Free(tmpetc->ptd);
         }
         delete tmpetc;
      }

      pmalloc->Release();
   }
   m_lpfmtetcs.clear();
}

bool QWindowsOleEnumFmtEtc::isNull() const
{
   return m_isNull;
}

// IUnknown methods
STDMETHODIMP QWindowsOleEnumFmtEtc::QueryInterface(REFIID riid, void FAR *FAR *ppvObj)
{
   if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC) {
      *ppvObj = this;
      AddRef();
      return NOERROR;
   }
   *ppvObj = nullptr;
   return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
QWindowsOleEnumFmtEtc::AddRef(void)
{
   return ++m_dwRefs;
}

STDMETHODIMP_(ULONG)
QWindowsOleEnumFmtEtc::Release(void)
{
   if (--m_dwRefs == 0) {
      delete this;
      return 0;
   }
   return m_dwRefs;
}

// IEnumFORMATETC methods
STDMETHODIMP QWindowsOleEnumFmtEtc::Next(ULONG celt, LPFORMATETC rgelt, ULONG FAR *pceltFetched)
{
   ULONG i = 0;
   ULONG nOffset;

   if (rgelt == nullptr) {
      return ResultFromScode(E_INVALIDARG);
   }

   while (i < celt) {
      nOffset = m_nIndex + i;

      if (nOffset < ULONG(m_lpfmtetcs.count())) {
         copyFormatEtc(rgelt + i, m_lpfmtetcs.at(int(nOffset)));
         i++;
      } else {
         break;
      }
   }

   m_nIndex += i;

   if (pceltFetched != nullptr) {
      *pceltFetched = i;
   }

   if (i != celt) {
      return ResultFromScode(S_FALSE);
   }

   return NOERROR;
}

STDMETHODIMP QWindowsOleEnumFmtEtc::Skip(ULONG celt)
{
   ULONG i = 0;
   ULONG nOffset;

   while (i < celt) {
      nOffset = m_nIndex + i;

      if (nOffset < ULONG(m_lpfmtetcs.count())) {
         i++;
      } else {
         break;
      }
   }

   m_nIndex += i;

   if (i != celt) {
      return ResultFromScode(S_FALSE);
   }

   return NOERROR;
}

STDMETHODIMP QWindowsOleEnumFmtEtc::Reset()
{
   m_nIndex = 0;
   return NOERROR;
}

STDMETHODIMP QWindowsOleEnumFmtEtc::Clone(LPENUMFORMATETC FAR *newEnum)
{
   if (newEnum == nullptr) {
      return ResultFromScode(E_INVALIDARG);
   }

   QWindowsOleEnumFmtEtc *result = new QWindowsOleEnumFmtEtc(m_lpfmtetcs);
   result->m_nIndex = m_nIndex;

   if (result->isNull()) {
      delete result;
      return ResultFromScode(E_OUTOFMEMORY);
   } else {
      *newEnum = result;
   }

   return NOERROR;
}

bool QWindowsOleEnumFmtEtc::copyFormatEtc(LPFORMATETC dest, const FORMATETC *src) const
{
   if (dest == nullptr || src == nullptr) {
      return false;
   }

   *dest = *src;

   if (src->ptd) {
      LPMALLOC pmalloc;

      if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
         return false;
      }

      pmalloc->Alloc(src->ptd->tdSize);
      memcpy(dest->ptd, src->ptd, size_t(src->ptd->tdSize));

      pmalloc->Release();
   }

   return true;
}
