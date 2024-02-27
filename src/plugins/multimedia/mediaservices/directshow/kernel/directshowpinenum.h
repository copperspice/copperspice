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

#ifndef DIRECTSHOWPINENUM_H
#define DIRECTSHOWPINENUM_H

#include <dshow.h>

#include <qlist.h>

class DirectShowPinEnum : public IEnumPins
{
 public:
   DirectShowPinEnum(const QList<IPin *> &pins);
   virtual ~DirectShowPinEnum();

   // IUnknown
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
   ULONG STDMETHODCALLTYPE AddRef() override;
   ULONG STDMETHODCALLTYPE Release() override;

   // IEnumPins
   HRESULT STDMETHODCALLTYPE Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched) override;
   HRESULT STDMETHODCALLTYPE Skip(ULONG cPins) override;
   HRESULT STDMETHODCALLTYPE Reset() override;
   HRESULT STDMETHODCALLTYPE Clone(IEnumPins **ppEnum) override;

 private:
   LONG m_ref;
   QList<IPin *> m_pins;
   int m_index;
};

#endif
