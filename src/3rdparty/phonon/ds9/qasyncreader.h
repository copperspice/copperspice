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

#ifndef DS9_QASYNCREADER_H
#define DS9_QASYNCREADER_H

#include <QtCore/QWaitCondition>
#include <QtCore/QQueue>
#include <QtCore/QMutex>

#include "qpin.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        //his class reads asynchronously from a QIODevice
        class QAsyncReader : public QPin, public IAsyncReader
        {
        public:
            QAsyncReader(QBaseFilter *, const QVector<AM_MEDIA_TYPE> &mediaTypes);
            ~QAsyncReader();

            //reimplementation from IUnknown
            STDMETHODIMP QueryInterface(REFIID iid, void** out) override;
            STDMETHODIMP_(ULONG) AddRef() override;
            STDMETHODIMP_(ULONG) Release() override;

            //reimplementation from IAsyncReader
            STDMETHODIMP RequestAllocator(IMemAllocator *,ALLOCATOR_PROPERTIES *,IMemAllocator **) override;
            STDMETHODIMP Request(IMediaSample *,DWORD_PTR) override;
            STDMETHODIMP WaitForNext(DWORD,IMediaSample **,DWORD_PTR *) override;
            STDMETHODIMP SyncReadAligned(IMediaSample *) override;
            STDMETHODIMP SyncRead(LONGLONG,LONG,BYTE *) override;
            STDMETHODIMP Length(LONGLONG *,LONGLONG *) override  = 0;
            STDMETHODIMP BeginFlush() override;
            STDMETHODIMP EndFlush() override;

        protected:
            STDMETHODIMP syncReadAlignedUnlocked(IMediaSample *);
            virtual HRESULT read(LONGLONG pos, LONG length, BYTE *buffer, LONG *actual) = 0;

        private:
            struct AsyncRequest
            {
                AsyncRequest(IMediaSample *s = 0, DWORD_PTR u = 0) : sample(s), user(u) {}
                IMediaSample *sample;
                DWORD_PTR user;
            };

            QQueue<AsyncRequest> m_requestQueue;
            QWaitCondition m_requestWait;
        };
    }
}

QT_END_NAMESPACE

#endif
