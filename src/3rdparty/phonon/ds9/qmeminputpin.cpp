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

#include "qmeminputpin.h"
#include "qbasefilter.h"
#include "compointer.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {

        QMemInputPin::QMemInputPin(QBaseFilter *parent, const QVector<AM_MEDIA_TYPE> &mt, bool transform, QPin *output) :
            QPin(parent, PINDIR_INPUT, mt), m_shouldDuplicateSamples(true), m_transform(transform), m_output(output)
        {
        }

        QMemInputPin::~QMemInputPin()
        {
        }

        STDMETHODIMP QMemInputPin::QueryInterface(REFIID iid, void **out)
        {
            if (!out) {
                return E_POINTER;
            }

            if (iid == IID_IMemInputPin) {
                *out = static_cast<IMemInputPin*>(this);
                AddRef();
                return S_OK;
            } else {
                return QPin::QueryInterface(iid, out);
            }
        }

        STDMETHODIMP_(ULONG) QMemInputPin::AddRef()
        {
            return QPin::AddRef();
        }

        STDMETHODIMP_(ULONG) QMemInputPin::Release()
        {
            return QPin::Release();
        }

        STDMETHODIMP QMemInputPin::EndOfStream()
        {
            //this allows to serialize with Receive calls
            QMutexLocker locker(&m_mutexReceive);
            IPin *conn = m_output ? m_output->connected() : 0;
            if (conn) {
                conn->EndOfStream();
            }
            return S_OK;
        }

        STDMETHODIMP QMemInputPin::BeginFlush()
        {
            //pass downstream
            IPin *conn = m_output ? m_output->connected() : 0;
            if (conn) {
                conn->BeginFlush();
            }
            QMutexLocker locker(&m_mutex);
            m_flushing = true;
            return S_OK;
        }

        STDMETHODIMP QMemInputPin::EndFlush()
        {
            //pass downstream
            IPin *conn = m_output ? m_output->connected() : 0;
            if (conn) {
                conn->EndFlush();
            }
            QMutexLocker locker(&m_mutex);
            m_flushing = false;
            return S_OK;
        }

        STDMETHODIMP QMemInputPin::NewSegment(REFERENCE_TIME start, REFERENCE_TIME stop, double rate)
        {
            if (m_output)
                m_output->NewSegment(start, stop, rate);
            return S_OK;
        }

        //reimplementation to set the type for the output pin
        //no need to make a deep copy here
        STDMETHODIMP QMemInputPin::ReceiveConnection(IPin *pin ,const AM_MEDIA_TYPE *mt)
        {
            HRESULT hr = QPin::ReceiveConnection(pin, mt);
            if (hr == S_OK &&
                mt->majortype != MEDIATYPE_NULL &&
                mt->subtype != MEDIASUBTYPE_NULL &&
                mt->formattype != GUID_NULL && m_output) {
                    //we tell the output pin that it should connect with this type
                    hr = m_output->setAcceptedMediaType(connectedType());
            }
            return hr;
        }

        STDMETHODIMP QMemInputPin::GetAllocator(IMemAllocator **alloc)
        {
            if (!alloc) {
                return E_POINTER;
            }

            *alloc = memoryAllocator(true);
            if (*alloc) {
                return S_OK;
            }

            return VFW_E_NO_ALLOCATOR;
        }

        STDMETHODIMP QMemInputPin::NotifyAllocator(IMemAllocator *alloc, BOOL readonly)
        {
            if (!alloc) {
                return E_POINTER;
            }

            {
                QMutexLocker locker(&m_mutex);
                m_shouldDuplicateSamples = m_transform && readonly;
            }

            setMemoryAllocator(alloc);

            if (m_output) {
                ComPointer<IMemInputPin> input(m_output, IID_IMemInputPin);
                input->NotifyAllocator(alloc, m_shouldDuplicateSamples);
            }

            return S_OK;
        }

        STDMETHODIMP QMemInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *prop)
        {
            if (!prop) {
                return E_POINTER;
            }

            //we have no particular requirements
            return E_NOTIMPL;
        }

        STDMETHODIMP QMemInputPin::Receive(IMediaSample *sample)
        {
            QMutexLocker locker(&m_mutexReceive);
            if (!sample) {
                return E_POINTER;
            }

            if (filterState() == State_Stopped) {
                return VFW_E_WRONG_STATE;
            }

            if (isFlushing()) {
                return S_FALSE; //we are still flushing
            }

            if (!m_shouldDuplicateSamples) {
                //we do it just once
                HRESULT hr = m_parent->processSample(sample);
                if (!SUCCEEDED(hr)) {
                    return hr;
                }
            }

            if (m_output) {
                IMediaSample *outSample = m_shouldDuplicateSamples ?
                    duplicateSampleForOutput(sample, m_output->memoryAllocator())
                    : sample;

                if (m_shouldDuplicateSamples) {
                    m_parent->processSample(outSample);
                }

                ComPointer<IMemInputPin> input(m_output->connected(), IID_IMemInputPin);
                if (input) {
                    input->Receive(outSample);
                }

                if (m_shouldDuplicateSamples) {
                    outSample->Release();
                }
            }
            return S_OK;
        }

        STDMETHODIMP QMemInputPin::ReceiveMultiple(IMediaSample **samples,long count,long *nbDone)
        {
            //no need to lock here: there is no access to member data
            if (!samples || !nbDone) {
                return E_POINTER;
            }

            *nbDone = 0; //initialization
            while( *nbDone != count) {
                HRESULT hr = Receive(samples[*nbDone]);
                if (FAILED(hr)) {
                    return hr;
                }
                (*nbDone)++;
            }

            return S_OK;
        }

        STDMETHODIMP QMemInputPin::ReceiveCanBlock()
        {
            //we test the output to see if it can block
            if (m_output) {
                ComPointer<IMemInputPin> meminput(m_output->connected(), IID_IMemInputPin);
                if (meminput && meminput->ReceiveCanBlock() != S_FALSE) {
                    return S_OK;
                }
            }
            return S_FALSE;
        }


        ALLOCATOR_PROPERTIES QMemInputPin::getDefaultAllocatorProperties() const
        {
            //those values reduce buffering a lot (good for the volume effect)
            ALLOCATOR_PROPERTIES prop = {4096, 1, 1, 0};
            return prop;
        }


        IMediaSample *QMemInputPin::duplicateSampleForOutput(IMediaSample *sample, IMemAllocator *alloc)
        {
            LONG length = sample->GetActualDataLength();

            HRESULT hr = alloc->Commit();
            if (hr == HRESULT(VFW_E_SIZENOTSET)) {
                ALLOCATOR_PROPERTIES prop = getDefaultAllocatorProperties();
                prop.cbBuffer = qMax(prop.cbBuffer, length);
                ALLOCATOR_PROPERTIES actual;
                //we just try to set the properties...
                alloc->SetProperties(&prop, &actual);
                hr = alloc->Commit();
            }

            Q_ASSERT(SUCCEEDED(hr));

            IMediaSample *out;
            hr = alloc->GetBuffer(&out, 0, 0, AM_GBF_NOTASYNCPOINT);
            Q_ASSERT(SUCCEEDED(hr));

            //let's copy the sample
            {
                REFERENCE_TIME start, end;
                sample->GetTime(&start, &end);
                out->SetTime(&start, &end);
            }

            hr = out->SetActualDataLength(length);
            Q_ASSERT(SUCCEEDED(hr));
            hr = out->SetDiscontinuity(sample->IsDiscontinuity());
            Q_ASSERT(SUCCEEDED(hr));

            {
                LONGLONG start, end;
                hr = sample->GetMediaTime(&start, &end);
                if (hr != HRESULT(VFW_E_MEDIA_TIME_NOT_SET)) {
                    hr = out->SetMediaTime(&start, &end);
                    Q_ASSERT(SUCCEEDED(hr));
                }
            }

            AM_MEDIA_TYPE *type = 0;
            hr = sample->GetMediaType(&type);
            Q_ASSERT(SUCCEEDED(hr));
            hr = out->SetMediaType(type);
            Q_ASSERT(SUCCEEDED(hr));

            hr = out->SetPreroll(sample->IsPreroll());
            Q_ASSERT(SUCCEEDED(hr));
            hr = out->SetSyncPoint(sample->IsSyncPoint());
            Q_ASSERT(SUCCEEDED(hr));

            BYTE *dest = 0, *src = 0;
            hr = out->GetPointer(&dest);
            Q_ASSERT(SUCCEEDED(hr));
            hr = sample->GetPointer(&src);
            Q_ASSERT(SUCCEEDED(hr));

            memcpy(dest, src, sample->GetActualDataLength());

            return out;
        }
    }
}

QT_END_NAMESPACE
