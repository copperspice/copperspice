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

#ifndef QWINDOWSOLE_H
#define QWINDOWSOLE_H

#include <qwin_additional.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvector.h>

#include <objidl.h>

class QMimeData;
class QWindow;

class QWindowsOleDataObject : public IDataObject
{
 public:
   explicit QWindowsOleDataObject(QMimeData *mimeData);
   virtual ~QWindowsOleDataObject();

   void releaseData();

   QMimeData *mimeData() const;
   DWORD reportedPerformedEffect() const;

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void FAR *FAR *ppvObj);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

   // IDataObject methods
   STDMETHOD(GetData)(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
   STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
   STDMETHOD(QueryGetData)(LPFORMATETC pformatetc);
   STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
   STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR *pmedium, BOOL fRelease);
   STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR *ppenumFormatEtc);
   STDMETHOD(DAdvise)(FORMATETC FAR *pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR *pdwConnection);
   STDMETHOD(DUnadvise)(DWORD dwConnection);
   STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR *ppenumAdvise);

 private:
   ULONG m_refs;
   QPointer<QMimeData> data;
   int CF_PERFORMEDDROPEFFECT;
   DWORD performedEffect;
};

class QWindowsOleEnumFmtEtc : public IEnumFORMATETC
{
 public:
   explicit QWindowsOleEnumFmtEtc(const QVector<FORMATETC> &fmtetcs);
   explicit QWindowsOleEnumFmtEtc(const QVector<LPFORMATETC> &lpfmtetcs);

   virtual ~QWindowsOleEnumFmtEtc();

   bool isNull() const;

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void FAR *FAR *ppvObj);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

   // IEnumFORMATETC methods
   STDMETHOD(Next)(ULONG celt, LPFORMATETC rgelt, ULONG FAR *pceltFetched);
   STDMETHOD(Skip)(ULONG celt);
   STDMETHOD(Reset)(void);
   STDMETHOD(Clone)(LPENUMFORMATETC FAR *newEnum);

 private:
   bool copyFormatEtc(LPFORMATETC dest, const FORMATETC *src) const;

   ULONG m_dwRefs;
   ULONG m_nIndex;
   QVector<LPFORMATETC> m_lpfmtetcs;
   bool m_isNull;
};

#endif
