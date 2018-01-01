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

#include "qasyncreader.h"
#include "qbasefilter.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        QAsyncReader::QAsyncReader(QBaseFilter *parent, const QVector<AM_MEDIA_TYPE> &mediaTypes) : QPin(parent, PINDIR_OUTPUT, mediaTypes)
        {
        }

        QAsyncReader::~QAsyncReader()
        {
        }

        STDMETHODIMP QAsyncReader::QueryInterface(REFIID iid, void **out)
        {
            if (!out) {
                return E_POINTER;
            }

            if (iid == IID_IAsyncReader) {
                AddRef();
                *out = static_cast<IAsyncReader*>(this);
                return S_OK;
            }

            return QPin::QueryInterface(iid, out);
        }

        STDMETHODIMP_(ULONG) QAsyncReader::AddRef()
        {
            return QPin::AddRef();
        }

        STDMETHODIMP_(ULONG) QAsyncReader::Release()
        {
            return QPin::Release();
        }


        STDMETHODIMP QAsyncReader::RequestAllocator(IMemAllocator *preferred, ALLOCATOR_PROPERTIES *prop,IMemAllocator **actual)
        {
            ALLOCATOR_PROPERTIES prop2;

            if (prop->cbAlign == 0) {
                prop->cbAlign = 1; //align on 1 char
            }

            if (preferred && preferred->SetProperties(prop, &prop2) == S_OK) {
                preferred->AddRef();
                *actual = preferred;
                return S_OK;
            }

            //we should try to create one memory allocator ourselves here
            return E_FAIL;
        }

        STDMETHODIMP QAsyncReader::Request(IMediaSample *sample,DWORD_PTR user)
        {
            QMutexLocker locker(&m_mutex);
            if (m_flushing) {
                return VFW_E_WRONG_STATE;
            }

            m_requestQueue.enqueue(AsyncRequest(sample, user));
            m_requestWait.wakeOne();
            return S_OK;
        }

        STDMETHODIMP QAsyncReader::WaitForNext(DWORD timeout, IMediaSample **sample, DWORD_PTR *user)
        {
            QMutexLocker locker(&m_mutex);
            if (!sample ||!user) {
                return E_POINTER;
            }

            //msdn says to return immediately if we're flushing but that doesn't seem to be true
            //since it triggers a dead-lock somewhere inside directshow (see task 258830)

            *sample = 0;
            *user = 0;

            if (m_requestQueue.isEmpty()) {
                if (m_requestWait.wait(&m_mutex, timeout) == false) {
                    return VFW_E_TIMEOUT;
                }
                if (m_requestQueue.isEmpty()) {
                    return VFW_E_WRONG_STATE;
                }
            }

            AsyncRequest r = m_requestQueue.dequeue();

            //at this point we're sure to have a request to proceed
            if (r.sample == 0) {
                return E_FAIL;
            }

            *sample = r.sample;
            *user = r.user;
            return syncReadAlignedUnlocked(r.sample);
        }

        STDMETHODIMP QAsyncReader::BeginFlush()
        {
            QMutexLocker locker(&m_mutex);
            m_flushing = true;
            m_requestWait.wakeOne();
            return S_OK;
        }

        STDMETHODIMP QAsyncReader::EndFlush()
        {
            QMutexLocker locker(&m_mutex);
            m_flushing = false;
            return S_OK;
        }

        STDMETHODIMP QAsyncReader::SyncReadAligned(IMediaSample *sample)
        {
            QMutexLocker locker(&m_mutex);
            return syncReadAlignedUnlocked(sample);
        }

        STDMETHODIMP QAsyncReader::SyncRead(LONGLONG pos, LONG length, BYTE *buffer)
        {
            QMutexLocker locker(&m_mutex);
            return read(pos, length, buffer, 0);
        }


        STDMETHODIMP QAsyncReader::syncReadAlignedUnlocked(IMediaSample *sample)
        {
            Q_ASSERT(!m_mutex.tryLock());

            if (!sample) {
                return E_POINTER;
            }

            REFERENCE_TIME start = 0,
                stop = 0;
            HRESULT hr = sample->GetTime(&start, &stop);
            if(FAILED(hr)) {
                return hr;
            }

            LONGLONG startPos = start / 10000000;
            LONG length = static_cast<LONG>((stop - start) / 10000000);

            BYTE *buffer;
            hr = sample->GetPointer(&buffer);
            if(FAILED(hr)) {
                return hr;
            }

            LONG actual = 0;
            read(startPos, length, buffer, &actual);

            return sample->SetActualDataLength(actual);
        }

    }
}

QT_END_NAMESPACE
