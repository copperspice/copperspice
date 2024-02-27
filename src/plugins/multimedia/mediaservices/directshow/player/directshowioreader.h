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

#ifndef DIRECTSHOWIOREADER_H
#define DIRECTSHOWIOREADER_H

#include <dshow.h>

#include <qmutex.h>
#include <qobject.h>
#include <qwaitcondition.h>

class QIODevice;

class DirectShowEventLoop;
class DirectShowIOSource;
class DirectShowSampleRequest;

class DirectShowIOReader : public QObject, public IAsyncReader
{
   CS_OBJECT(DirectShowIOReader)

 public:
   DirectShowIOReader(QIODevice *device, DirectShowIOSource *source, DirectShowEventLoop *loop);
   ~DirectShowIOReader();

   // IUnknown
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
   ULONG STDMETHODCALLTYPE AddRef() override;
   ULONG STDMETHODCALLTYPE Release() override;

   // IAsyncReader
   HRESULT STDMETHODCALLTYPE RequestAllocator(
      IMemAllocator *pPreferred, ALLOCATOR_PROPERTIES *pProps, IMemAllocator **ppActual) override;

   HRESULT STDMETHODCALLTYPE Request(IMediaSample *pSample, DWORD_PTR dwUser) override;

   HRESULT STDMETHODCALLTYPE WaitForNext(
      DWORD dwTimeout, IMediaSample **ppSample, DWORD_PTR *pdwUser) override;

   HRESULT STDMETHODCALLTYPE SyncReadAligned(IMediaSample *pSample) override;

   HRESULT STDMETHODCALLTYPE SyncRead(LONGLONG llPosition, LONG lLength, BYTE *pBuffer) override;

   HRESULT STDMETHODCALLTYPE Length(LONGLONG *pTotal, LONGLONG *pAvailable) override;

   HRESULT STDMETHODCALLTYPE BeginFlush() override;
   HRESULT STDMETHODCALLTYPE EndFlush() override;

 protected:
   void customEvent(QEvent *event) override;

 private:
   CS_SLOT_1(Private, void readyRead())
   CS_SLOT_2(readyRead)

   HRESULT blockingRead(LONGLONG position, LONG length, BYTE *buffer, qint64 *bytesRead);
   bool nonBlockingRead(
      LONGLONG position, LONG length, BYTE *buffer, qint64 *bytesRead, HRESULT *result);
   void flushRequests();

   DirectShowIOSource *m_source;
   QIODevice *m_device;
   DirectShowEventLoop *m_loop;
   DirectShowSampleRequest *m_pendingHead;
   DirectShowSampleRequest *m_pendingTail;
   DirectShowSampleRequest *m_readyHead;
   DirectShowSampleRequest *m_readyTail;
   LONGLONG m_synchronousPosition;
   LONG m_synchronousLength;
   qint64 m_synchronousBytesRead;
   BYTE *m_synchronousBuffer;
   HRESULT m_synchronousResult;
   LONGLONG m_totalLength;
   LONGLONG m_availableLength;
   bool m_flushing;
   QMutex m_mutex;
   QWaitCondition m_wait;
};

#endif
