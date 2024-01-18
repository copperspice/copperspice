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

#ifndef DIRECTSHOWMEDIATYPELIST_H
#define DIRECTSHOWMEDIATYPELIST_H

#include <dshow.h>

#include <qvector.h>

class DirectShowMediaTypeList : public IUnknown
{
 public:
   DirectShowMediaTypeList();
   virtual ~DirectShowMediaTypeList();

   IEnumMediaTypes *createMediaTypeEnum();

   void setMediaTypes(const QVector<AM_MEDIA_TYPE> &types);

   virtual int currentMediaTypeToken();
   virtual HRESULT nextMediaType(
      int token, int *index, ULONG count, AM_MEDIA_TYPE **types, ULONG *fetchedCount);
   virtual HRESULT skipMediaType(int token, int *index, ULONG count);
   virtual HRESULT cloneMediaType(int token, int index, IEnumMediaTypes **enumeration);

 protected:
   QVector<AM_MEDIA_TYPE> m_mediaTypes;

 private:
   int m_mediaTypeToken;
};

#endif
