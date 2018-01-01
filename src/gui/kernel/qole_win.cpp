/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qdnd_p.h>

#if !(defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

QT_BEGIN_NAMESPACE

QOleEnumFmtEtc::QOleEnumFmtEtc(const QVector<FORMATETC> &fmtetcs)
{
   m_isNull = false;
   m_dwRefs = 1;
   m_nIndex = 0;

   for (int idx = 0; idx < fmtetcs.count(); ++idx) {
      LPFORMATETC destetc = new FORMATETC();
      if (copyFormatEtc(destetc, (LPFORMATETC) & (fmtetcs.at(idx)))) {
         m_lpfmtetcs.append(destetc);
      } else {
         m_isNull = true;
         delete destetc;
         break;
      }
   }
}

QOleEnumFmtEtc::QOleEnumFmtEtc(const QVector<LPFORMATETC> &lpfmtetcs)
{
   m_isNull = false;
   m_dwRefs = 1;
   m_nIndex = 0;

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

QOleEnumFmtEtc::~QOleEnumFmtEtc()
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

bool QOleEnumFmtEtc::isNull() const
{
   return m_isNull;
}

// IUnknown methods
STDMETHODIMP
QOleEnumFmtEtc::QueryInterface(REFIID riid, void FAR *FAR *ppvObj)
{
   if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC) {
      *ppvObj = this;
      AddRef();
      return NOERROR;
   }
   *ppvObj = NULL;
   return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
QOleEnumFmtEtc::AddRef(void)
{
   return ++m_dwRefs;
}

STDMETHODIMP_(ULONG)
QOleEnumFmtEtc::Release(void)
{
   if (--m_dwRefs == 0) {
      delete this;
      return 0;
   }
   return m_dwRefs;
}

// IEnumFORMATETC methods
STDMETHODIMP
QOleEnumFmtEtc::Next(ULONG celt, LPFORMATETC rgelt, ULONG FAR *pceltFetched)
{
   ULONG i = 0;
   ULONG nOffset;

   if (rgelt == NULL) {
      return ResultFromScode(E_INVALIDARG);
   }

   while (i < celt) {
      nOffset = m_nIndex + i;

      if (nOffset < ULONG(m_lpfmtetcs.count())) {
         copyFormatEtc((LPFORMATETC) & (rgelt[i]), m_lpfmtetcs.at(nOffset));
         i++;
      } else {
         break;
      }
   }

   m_nIndex += (WORD)i;

   if (pceltFetched != NULL) {
      *pceltFetched = i;
   }

   if (i != celt) {
      return ResultFromScode(S_FALSE);
   }

   return NOERROR;
}

STDMETHODIMP
QOleEnumFmtEtc::Skip(ULONG celt)
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

   m_nIndex += (WORD)i;

   if (i != celt) {
      return ResultFromScode(S_FALSE);
   }

   return NOERROR;
}

STDMETHODIMP
QOleEnumFmtEtc::Reset()
{
   m_nIndex = 0;
   return NOERROR;
}

STDMETHODIMP
QOleEnumFmtEtc::Clone(LPENUMFORMATETC FAR *newEnum)
{
   if (newEnum == NULL) {
      return ResultFromScode(E_INVALIDARG);
   }

   QOleEnumFmtEtc *result = new QOleEnumFmtEtc(m_lpfmtetcs);
   result->m_nIndex = m_nIndex;

   if (result->isNull()) {
      delete result;
      return ResultFromScode(E_OUTOFMEMORY);
   } else {
      *newEnum = result;
   }

   return NOERROR;
}

bool QOleEnumFmtEtc::copyFormatEtc(LPFORMATETC dest, LPFORMATETC src) const
{
   if (dest == NULL || src == NULL) {
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

QT_END_NAMESPACE
#endif // QT_NO_DRAGANDDROP && QT_NO_CLIPBOARD
