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

#include "directshowmediatypelist.h"

#include "directshowmediatype.h"
#include "videosurfacefilter.h"

class DirectShowMediaTypeEnum : public IEnumMediaTypes
{
 public:
   DirectShowMediaTypeEnum(DirectShowMediaTypeList *list, int token, int index = 0);
   virtual ~DirectShowMediaTypeEnum();

   // IUnknown
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
   ULONG STDMETHODCALLTYPE AddRef() override;
   ULONG STDMETHODCALLTYPE Release() override;

   // IEnumMediaTypes
   HRESULT STDMETHODCALLTYPE Next(ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched) override;
   HRESULT STDMETHODCALLTYPE Skip(ULONG cMediaTypes) override;
   HRESULT STDMETHODCALLTYPE Reset() override;

   HRESULT STDMETHODCALLTYPE Clone(IEnumMediaTypes **ppEnum) override;

 private:
   LONG m_ref;
   DirectShowMediaTypeList *m_list;
   int m_mediaTypeToken;
   int m_index;
};

DirectShowMediaTypeEnum::DirectShowMediaTypeEnum(
   DirectShowMediaTypeList *list, int token, int index)
   : m_ref(1), m_list(list), m_mediaTypeToken(token), m_index(index)
{
   m_list->AddRef();
}

DirectShowMediaTypeEnum::~DirectShowMediaTypeEnum()
{
   m_list->Release();
}

HRESULT DirectShowMediaTypeEnum::QueryInterface(REFIID riid, void **ppvObject)
{
   if (! ppvObject) {
      return E_POINTER;

   } else if (riid == IID_IUnknown || riid == IID_IEnumMediaTypes) {
      *ppvObject = static_cast<IEnumMediaTypes *>(this);

   } else {
      *ppvObject = nullptr;

      return E_NOINTERFACE;
   }

   AddRef();

   return S_OK;
}

ULONG DirectShowMediaTypeEnum::AddRef()
{
   return InterlockedIncrement(&m_ref);
}

ULONG DirectShowMediaTypeEnum::Release()
{
   ULONG ref = InterlockedDecrement(&m_ref);

   if (ref == 0) {
      delete this;
   }

   return ref;
}

HRESULT DirectShowMediaTypeEnum::Next(
   ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched)
{
   return m_list->nextMediaType(m_mediaTypeToken, &m_index, cMediaTypes, ppMediaTypes, pcFetched);
}

HRESULT DirectShowMediaTypeEnum::Skip(ULONG cMediaTypes)
{
   return m_list->skipMediaType(m_mediaTypeToken, &m_index, cMediaTypes);
}

HRESULT DirectShowMediaTypeEnum::Reset()
{
   m_mediaTypeToken = m_list->currentMediaTypeToken();
   m_index = 0;

   return S_OK;
}

HRESULT DirectShowMediaTypeEnum::Clone(IEnumMediaTypes **ppEnum)
{
   return m_list->cloneMediaType(m_mediaTypeToken, m_index, ppEnum);
}

DirectShowMediaTypeList::DirectShowMediaTypeList()
   : m_mediaTypeToken(0)
{
}

DirectShowMediaTypeList::~DirectShowMediaTypeList()
{
}

IEnumMediaTypes *DirectShowMediaTypeList::createMediaTypeEnum()
{
   return new DirectShowMediaTypeEnum(this, m_mediaTypeToken, 0);
}

void DirectShowMediaTypeList::setMediaTypes(const QVector<AM_MEDIA_TYPE> &types)
{
   ++m_mediaTypeToken;

   m_mediaTypes = types;
}

int DirectShowMediaTypeList::currentMediaTypeToken()
{
   return m_mediaTypeToken;
}

HRESULT DirectShowMediaTypeList::nextMediaType(
   int token, int *index, ULONG count, AM_MEDIA_TYPE **types, ULONG *fetchedCount)
{
   if (!types || (count != 1 && !fetchedCount)) {
      return E_POINTER;
   } else if (m_mediaTypeToken != token) {
      return VFW_E_ENUM_OUT_OF_SYNC;
   } else {
      int boundedCount = qBound<int>(0, count, m_mediaTypes.count() - *index);

      for (int i = 0; i < boundedCount; ++i, ++(*index)) {
         types[i] = reinterpret_cast<AM_MEDIA_TYPE *>(CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE)));

         if (types[i]) {
            DirectShowMediaType::copy(types[i], m_mediaTypes.at(*index));
         } else {
            for (--i; i >= 0; --i) {
               CoTaskMemFree(types[i]);
            }

            if (fetchedCount) {
               *fetchedCount = 0;
            }

            return E_OUTOFMEMORY;
         }
      }
      if (fetchedCount) {
         *fetchedCount = boundedCount;
      }

      return boundedCount == int(count) ? S_OK : S_FALSE;
   }
}

HRESULT DirectShowMediaTypeList::skipMediaType(int token, int *index, ULONG count)
{
   if (m_mediaTypeToken != token) {
      return VFW_E_ENUM_OUT_OF_SYNC;
   } else {
      *index = qMin<int>(*index + count, m_mediaTypes.size());

      return *index < m_mediaTypes.size() ? S_OK : S_FALSE;
   }
}

HRESULT DirectShowMediaTypeList::cloneMediaType(int token, int index, IEnumMediaTypes **enumeration)
{
   if (m_mediaTypeToken != token) {
      return VFW_E_ENUM_OUT_OF_SYNC;
   } else {
      *enumeration = new DirectShowMediaTypeEnum(this, token, index);

      return S_OK;
   }
}

