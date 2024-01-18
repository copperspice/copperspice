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

#include "directshowpinenum.h"

DirectShowPinEnum::DirectShowPinEnum(const QList<IPin *> &pins)
   : m_ref(1), m_pins(pins), m_index(0)
{
   for (IPin *pin : m_pins) {
      pin->AddRef();
   }
}

DirectShowPinEnum::~DirectShowPinEnum()
{
   for (IPin *pin : m_pins) {
      pin->Release();
   }
}

HRESULT DirectShowPinEnum::QueryInterface(REFIID riid, void **ppvObject)
{
   if (riid == IID_IUnknown
      || riid == IID_IEnumPins) {
      AddRef();

      *ppvObject = static_cast<IEnumPins *>(this);

      return S_OK;

   } else {
      *ppvObject = nullptr;

      return E_NOINTERFACE;
   }
}

ULONG DirectShowPinEnum::AddRef()
{
   return InterlockedIncrement(&m_ref);
}

ULONG DirectShowPinEnum::Release()
{
   ULONG ref = InterlockedDecrement(&m_ref);

   if (ref == 0) {
      delete this;
   }

   return ref;
}

HRESULT DirectShowPinEnum::Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched)
{
   if (ppPins && (pcFetched || cPins == 1)) {
      ULONG count = qBound<ULONG>(0, cPins, m_pins.count() - m_index);

      for (ULONG i = 0; i < count; ++i, ++m_index) {
         ppPins[i] = m_pins.at(m_index);
         ppPins[i]->AddRef();
      }

      if (pcFetched) {
         *pcFetched = count;
      }

      return count == cPins ? S_OK : S_FALSE;
   } else {
      return E_POINTER;
   }
}

HRESULT DirectShowPinEnum::Skip(ULONG cPins)
{
   m_index = qMin(int(m_index + cPins), m_pins.count());

   return m_index < m_pins.count() ? S_OK : S_FALSE;
}

HRESULT DirectShowPinEnum::Reset()
{
   m_index = 0;

   return S_OK;
}

HRESULT DirectShowPinEnum::Clone(IEnumPins **ppEnum)
{
   if (ppEnum) {
      *ppEnum = new DirectShowPinEnum(m_pins);

      return S_OK;
   } else {
      return E_POINTER;
   }
}
